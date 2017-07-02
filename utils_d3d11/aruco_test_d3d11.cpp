#include "capture.h"
#include "detector.h"
#include "win32window.h"
#include "swapchain.h"
#include "ThreadName.h"
#include <libdxgiutil.h>
#include <iostream>
#include <memory>
#include <thread>
#include <DirectXMath.h>
#include <dwrite.h>
#include <opencv2/imgproc/imgproc.hpp>


#define SHADERPATH L"D:/dev/_ar/aruco_sample"


int main(int argc,char **argv)
{
    // initialize capture
    auto capture=std::make_shared<Capture>();
    if(!capture->Open(1)){
        std::cerr<<"Could not open video"<<std::endl;
        return 2;
    }
    cv::Mat TheInputImage;
    if (!capture->GetFrame(TheInputImage)) {
        return 6;
    }

	// create window
	auto window = win32::RegisterAndCreateWindow(
		GetModuleHandle(NULL), TEXT("aruco_test_d3d11_class"), TEXT("aruco_test_d3d11"), 640, 480);
	if (!window){
		return 3;
	}
	int iWidth = 640;
	int iHeight = 480;
	auto callback = [&iWidth, &iHeight](int w, int h){
		iWidth = w;
		iHeight = h;
	};
	window->addResizeCallback(callback);
	ShowWindow(window->GetHandle(), SW_SHOW);

	{
		// Initialize DXGI Device
		auto deviceManager = dxgiutil::DeviceManager::Create();
		if (!deviceManager){
			return 4;
		}

		// Create Swapchain
		auto swapchain = deviceManager->CreateSwapchain(window->GetHandle());
		if (!swapchain){
			return 5;
		}
		std::weak_ptr<dxgiutil::Swapchain> weakSC(swapchain);
		window->addResizeCallback([weakSC](int w, int h){

			auto sc = weakSC.lock();
			if (sc){
				sc->SetSize(w, h);
			}

		});

		window->addKeyDownCallback([weakSC](HWND hWnd, int key){

			auto sc = weakSC.lock();
			if (sc){
				switch (key)
				{
				case VK_ESCAPE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;

				case VK_F11:
					sc->SetFullscreen(!sc->IsFullscreen());
					break;
				}
			}

		});

		// renderer
		auto deferredRenderer = std::make_shared<dxgiutil::DeferredDeviceContext>(
			deviceManager->GetD3D11Device());

		auto deferredCapture = std::make_shared<dxgiutil::DeferredDeviceContext>(
			deviceManager->GetD3D11Device());
		deferredRenderer->AddSubcontext(deferredCapture);

		auto deferredMarker = std::make_shared<dxgiutil::DeferredDeviceContext>(
			deviceManager->GetD3D11Device());
		deferredRenderer->AddSubcontext(deferredMarker);

		std::shared_ptr<dxgiutil::Texture> captureTexture;
		std::shared_ptr<dxgiutil::Texture> renderTexture;
		{

			captureTexture= deviceManager->CreateTexture(TheInputImage.size().width, TheInputImage.size().height
				, 4);
			renderTexture = deviceManager->CreateTexture(TheInputImage.size().width, TheInputImage.size().height
				, 4);
		}

		auto sampler = deviceManager->CreateSampler();

		// background capture image
		{
			auto rect = std::make_shared<dxgiutil::Shader>();
			if (!rect->Initialize(deviceManager->GetD3D11Device(), SHADERPATH L"/utils_d3d11/rect.hlsl", "VS", "GS", "PS")){
				return 6;
			}
			auto ia = std::make_shared<dxgiutil::InputAssemblerSource>();
			if (!ia->CreateRectForGS(deviceManager->GetD3D11Device())){
				return 7;
			}
			rect->SetIA(ia);
			deferredRenderer->AddPipeline(rect);
			rect->SetTexture("tex0", captureTexture);
			rect->SetSampler("sample0", sampler);
		}
		{
			auto rect = std::make_shared<dxgiutil::Shader>();
			if (!rect->Initialize(deviceManager->GetD3D11Device(), SHADERPATH L"/utils_d3d11/rect.hlsl", "VS", "GS", "PS")){
				return 6;
			}
			auto ia = std::make_shared<dxgiutil::InputAssemblerSource>();
			if (!ia->CreateRectForGS(deviceManager->GetD3D11Device())){
				return 7;
			}
			rect->SetIA(ia);
			deferredRenderer->AddPipeline(rect);
			rect->SetTexture("tex0", renderTexture);
			rect->SetSampler("sample0", sampler);
		}

		// capture thread
		bool stop = false;
		int captureFPS;
		auto captureLoop = [&stop, &iWidth, &iHeight, capture, captureTexture
			, deferredCapture, deviceManager, deferredMarker, renderTexture]()
		{
			// detector
			Detector detector;

			cv::Mat TheInputImage;
			cv::Mat TheResizedImage;
			cv::Mat The32bit;

			while (!stop){
				if (!capture->GetFrame(TheInputImage)){
					Sleep(33);
					continue;
				}

				capture->Resize(TheInputImage, iWidth, iHeight, TheResizedImage);
				cv::cvtColor(TheResizedImage, The32bit, CV_BGR2RGBA, 4);

				///////////////////////////////////
				// intrinsics
				float ratioX = iWidth / (float)TheInputImage.size().width;
				float ratioY = iHeight / (float)TheInputImage.size().height;
				float f = 628.0f;
				float fx = f;
				float cx = TheInputImage.cols*0.5f;
				float fy = f;
				float cy = TheInputImage.rows*0.5f;
				float tmp[] = {
					fx, 0, cx,
					0, fy, cy,
					0, 0, 1.0f,
				};
				cv::Mat intrinsics(3, 3, CV_32FC1, tmp);

				// Projection
				double zNear = 0.05;
				double zFar = 10;
				//float aspect = (float)iWidth / (float)iHeight;
				float aspect = (float)TheInputImage.size().width / (float)TheInputImage.size().height;
				float fovy = 2.0f * (float)atan(cy / fy);
				auto p = DirectX::XMMatrixPerspectiveFovLH(fovy, aspect, zNear, zFar);
				DirectX::XMFLOAT4X4 projMatrix;
				DirectX::XMStoreFloat4x4(&projMatrix, p);

				// View
				auto sy = DirectX::XMMatrixScaling(1.0f, -1.0f, 1.0f);
				DirectX::XMFLOAT4X4 viewMatrix;
				DirectX::XMStoreFloat4x4(&viewMatrix, sy);

				try{
					// Model
					auto markers = detector.Detect(TheInputImage, intrinsics);
					deferredMarker->ResizePipelines(markers);
					for (size_t i = 0; i < markers; ++i){
						float modelview_matrix[16];
						detector.GetModelViewMatrix(i, modelview_matrix);

						auto marker = deferredMarker->GetPipeline(i);
						if (!marker){
							auto device = deviceManager->GetD3D11Device();
							marker = std::make_shared<dxgiutil::Shader>();
							if (!marker->Initialize(device, SHADERPATH L"/utils_d3d11/marker.hlsl", "VS", "", "PS")){
								//return;
							}
							auto ia = std::make_shared<dxgiutil::InputAssemblerSource>();
							if (!ia->CreateRect(device, detector.GetMarkerSize())){
								//return;
							}
							marker->SetIA(ia);

							deferredMarker->SetPipeline(i, marker);
						}
						marker->SetCB("ModelMatrix", *((const DirectX::XMFLOAT4X4*)modelview_matrix));
						marker->SetCB("ViewMatrix", viewMatrix);
						marker->SetCB("ProjectionMatrix", projMatrix);
					}
					auto rtv = renderTexture->GetRTV();
					if (rtv){
						float clear[] = { 0, 0, 0, 0 };
						//deferredMarker->GetContext()->ClearRenderTargetView(rtv.Get(), clear);
						deferredMarker->RenderPipelines(rtv, iWidth, iHeight);
					}
				}
				catch (const cv::Exception &ex)
				{
					std::cerr << ex.what() << std::endl;
				}

				// update capture texture
				deferredCapture->OnFrame(captureTexture->GetTexture()
					, The32bit.data, The32bit.step*The32bit.rows, The32bit.step);

			}
		};
		std::thread captureTask(captureLoop);

		// rendering thread
		////////////////////////////////////////////////////////////
		// rendering thread
		////////////////////////////////////////////////////////////
		auto renderTask = [&stop, deviceManager, swapchain, deferredRenderer, &captureFPS
		, captureTexture]()
		{
			SetThreadName("RenderingThread");

			// d3d
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
			deviceManager->GetD3D11Device()->GetImmediateContext(&pContext);

			// d2d
			auto d2d = deviceManager->GetD2D1DeviceContext();
			// brush
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
			d2d->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);
			// dwrite
			Microsoft::WRL::ComPtr<IDWriteFactory> pDWriteFactory;
			if (FAILED(DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				&pDWriteFactory
				))){
				return;
			}
			Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
			if (FAILED(pDWriteFactory->CreateTextFormat(
				L"Gabriola",                // Font family name.
				NULL,                       // Font collection (NULL sets it to use the system font collection).
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				72.0f,
				L"en-us",
				&textFormat
				))){
				return;
			}
			if (FAILED(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING))){
				return;
			}
			if (FAILED(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER))){
				return;
			}

			// timer
			int frameCount = 0;
			int frameCountParSecond = 0;
			int renderFPS = 0;
			DWORD lastSecond = 0;

			while (!stop)
			{
				// update
				++frameCount;
				++frameCountParSecond;
				auto now = timeGetTime();
				auto nowSecond = now / 1000;
				if (nowSecond != lastSecond){
					renderFPS = frameCountParSecond;
					frameCountParSecond = 0;
					lastSecond = nowSecond;
				}

				// backbuffer
				auto backbuffer = swapchain->GetBackBuffer();
				if (!backbuffer){
					//std::cout << "no backbuffer. recreate" << std::endl;
					deferredRenderer->SetCommandList(nullptr);
					continue;
				}
				int width = backbuffer->GetDesc().Width;
				int height = backbuffer->GetDesc().Height;

				deferredRenderer->RenderPipelines(backbuffer->GetRTV(), width, height);
				{
					deferredRenderer->ExecuteCommandList(pContext);
				}

				auto bitmap = backbuffer->GetD2D1Bitmap(d2d);
				if (bitmap)
				{
					d2d->SetTarget(bitmap.Get());
					d2d->BeginDraw();

					D2D1_RECT_F layoutRect = D2D1::RectF(
						0,
						0,
						(float)width,
						(float)height
						);

					std::wstringstream ss;
					ss
						//<< "frame: " << frameCount << std::endl
						<< L"Rendering fps: " << renderFPS << std::endl
						<< L"Capture fps:" << captureFPS << std::endl

						;

					std::wstring text = ss.str();
					d2d->DrawText(
						text.c_str(),        // The string to render.
						text.size(),    // The string's length.
						textFormat.Get(),    // The text format.
						layoutRect,       // The region of the window where the text will be rendered.
						brush.Get()     // The brush used to draw the text.
						);

					d2d->EndDraw();
					d2d->SetTarget(nullptr);
				}

				swapchain->Present();
			}
		};
		std::thread renderThread(renderTask);

		// start message pump
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		stop = true;
		captureTask.join();
		renderThread.join();

		return msg.wParam;
	}
}
