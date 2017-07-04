#include "pch.h"
#include "CaptureRenderer.h"
#include "Detector.h"
#include <libdxgiutil.h>
#include <algorithm>
#include <functional>
#include <collection.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <wrl/client.h>
#include "..\Common\DirectXHelper.h"


static concurrency::task<Windows::Devices::Enumeration::DeviceInformation^> GetVideoDeviceAsync()
{
    return
        concurrency::create_task(Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
            Windows::Devices::Enumeration::DeviceClass::VideoCapture))
        .then([](Windows::Devices::Enumeration::DeviceInformationCollection^ devices)
            ->Windows::Devices::Enumeration::DeviceInformation^ {

        for (size_t i = 0; i < devices->Size; ++i)
        {
            auto device = devices->GetAt(i);
            std::wstring name(device->Name->Data());
            if (name == L"HTC Vive") {
                continue;
            }
            return device;
        }

        return nullptr;
    });
}


static concurrency::task<Platform::Agile<Windows::Media::Capture::MediaCapture^>> CreateMediaCaptureAsync(Windows::Devices::Enumeration::DeviceInformation^ device)
{
    Platform::Agile<Windows::Media::Capture::MediaCapture^> mediaCapture(ref new Windows::Media::Capture::MediaCapture());

    auto settings = ref new Windows::Media::Capture::MediaCaptureInitializationSettings();

    // Select the source we will be reading from.
    settings->VideoDeviceId = device->Id;

    // This media capture can share streaming with other apps.
    settings->SharingMode = Windows::Media::Capture::MediaCaptureSharingMode::SharedReadOnly;

    // Only stream video and don't initialize audio capture devices.
    settings->StreamingCaptureMode = Windows::Media::Capture::StreamingCaptureMode::Video;

    // Set to CPU to ensure frames always contain CPU SoftwareBitmap images,
    // instead of preferring GPU D3DSurface images.
    settings->MemoryPreference = Windows::Media::Capture::MediaCaptureMemoryPreference::Cpu;

    return
        concurrency::create_task(mediaCapture->InitializeAsync(settings))
        .then([mediaCapture](concurrency::task<void> initializeMediaCaptureTask) {

        try {
            // Get the result of the initialization. This call will throw if initialization failed
            // This pattern is docuemnted at https://msdn.microsoft.com/en-us/library/dd997692.aspx
            initializeMediaCaptureTask.get();
            return mediaCapture;
        }
        catch (Platform::Exception^ exception)
        {
            return Platform::Agile<Windows::Media::Capture::MediaCapture^>();
        }
    });
}


static Windows::Media::Capture::Frames::MediaFrameSourceInfo^ GetFirstSourceInfoOfKind(
    Windows::Media::Capture::Frames::MediaFrameSourceGroup^ group
    , Windows::Media::Capture::Frames::MediaFrameSourceKind kind)
{
    auto matchingInfo = std::find_if(begin(group->SourceInfos), end(group->SourceInfos),
        [kind](Windows::Media::Capture::Frames::MediaFrameSourceInfo^ sourceInfo)
    {
        return sourceInfo->SourceKind == kind;
    });
    return (matchingInfo != end(group->SourceInfos)) ? *matchingInfo : nullptr;
}


MIDL_INTERFACE("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")
IMemoryBufferByteAccess : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetBuffer(
        BYTE   **value,
        UINT32 *capacity
    );
};


static cv::Mat ToCV(Windows::Graphics::Imaging::SoftwareBitmap^ bitmap)
{
    auto buffer = bitmap->LockBuffer(Windows::Graphics::Imaging::BitmapBufferAccessMode::Read);
    auto reference = buffer->CreateReference();


    Microsoft::WRL::ComPtr<IMemoryBufferByteAccess> bufferByteAccess;

    HRESULT result = reinterpret_cast<IInspectable*>(reference)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

    if (result == S_OK)
    {
        //WriteLine("Get interface successfully");

        BYTE* data = nullptr;

        UINT32 capacity = 0;

        result = bufferByteAccess->GetBuffer(&data, &capacity);

        if (result == S_OK)
        {
            //WriteLine("get data access successfully, capacity: " + capacity);

            auto mat=cv::Mat(bitmap->PixelHeight, bitmap->PixelWidth, CV_8UC4);
            memcpy(mat.data, data, bitmap->PixelWidth*bitmap->PixelHeight*4);

            return mat;
        }
    }

    return cv::Mat();
}


static void ResizeMat(const cv::Mat &src, int width, int height, cv::Mat &dst)
{
    auto TheGlWindowSize = cv::Size(width, height);
    //not all sizes are allowed. OpenCv images have padding at the end of each line in these that are not aligned to 4 bytes
    if (width * 3 % 4 != 0) {
        width += width * 3 % 4;//resize to avoid padding
        ResizeMat(src, width, TheGlWindowSize.height, dst);
    }
    else {
        //resize the image to the size of the GL window
        cv::resize(src, dst, TheGlWindowSize);
    }
}


ref class ReaderCallback sealed
{
    std::function<void(const cv::Mat &)> m_callback;

internal:
    ReaderCallback(std::function<void(const cv::Mat &bitmap)> callback)
        : m_callback(callback)
    {

    }

public:
    void FrameReader_FrameArrived(Windows::Media::Capture::Frames::MediaFrameReader^ sender
        , Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs^ args)
    {
        if (auto frame = sender->TryAcquireLatestFrame())
        {
            if (frame)
            {
                auto input = frame->VideoMediaFrame;
                if (input) {
                    auto inputBitmap = input->SoftwareBitmap;
                    auto format = inputBitmap->BitmapPixelFormat;

                    try
                    {
                        // Convert to Bgra8 Premultiplied SoftwareBitmap, so xaml can display in UI.
                        auto converted = Windows::Graphics::Imaging::SoftwareBitmap::Convert(inputBitmap
                            , Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8
                            , Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied
                        );

                        m_callback(ToCV(converted));

                    }
                    catch (Platform::InvalidArgumentException^ exception)
                    {
                        // Conversion of software bitmap format is not supported.  Drop this frame.
                        OutputDebugStringW(exception->Message->Data());
                        OutputDebugStringW(L"\r\n");
                    }
                }
            }
        }
    }
};


namespace aruco_uwp
{
    CaptureRenderer::CaptureRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources)
        : m_deviceResources(deviceResources)
    {
        for (int i = 0; i < 4; ++i) {
            auto marker = std::make_shared<dxgiutil::Shader>();
            m_markerShaders.push_back(marker);
        }

        CreateDeviceDependentResources();
        CreateWindowSizeDependentResources();

        // setup capture
        m_detector = std::make_shared<Detector>();

        GetVideoDeviceAsync()
            .then([this](Windows::Devices::Enumeration::DeviceInformation^ device) {

            return CreateMediaCaptureAsync(device);
        })
            .then([this](Platform::Agile<Windows::Media::Capture::MediaCapture^> capture) {

            m_capture = capture;
            return StartCaptureAsync();

        });
        ;
    }

    void CaptureRenderer::CreateWindowSizeDependentResources()
    {
        auto outputSize = m_deviceResources->GetOutputSize();
        m_width = outputSize.Width;
        m_height = outputSize.Height;


        // background capture image
        auto captureRectTask = DX::ReadDataAsync(L"rect.hlsl.txt")
            .then([this](const std::vector<unsigned char> &bytes) {

            auto rect = std::make_shared<dxgiutil::Shader>();
            if (!rect->Initialize(m_deviceManager->GetD3D11Device()
                , bytes.data(), bytes.size(), "rect", "VS", "GS", "PS")) {
                throw Platform::Exception::CreateException(E_FAIL);
            }
            auto ia = std::make_shared<dxgiutil::InputAssemblerSource>();
            if (!ia->CreateRectForGS(m_deviceManager->GetD3D11Device())) {
                throw Platform::Exception::CreateException(E_FAIL);
            }
            rect->SetIA(ia);
            m_deferredRenderer->AddPipeline(rect);
            m_captureTexture = m_deviceManager->CreateTexture(m_width, m_height, 4);
            rect->SetTexture("tex0", m_captureTexture);
            auto sampler = m_deviceManager->CreateSampler();
            rect->SetSampler("sample0", sampler);
        });

        auto renderRectTask = DX::ReadDataAsync(L"rect.hlsl.txt")
            .then([this](const std::vector<unsigned char> &bytes) {

            auto rect = std::make_shared<dxgiutil::Shader>();
            if (!rect->Initialize(m_deviceManager->GetD3D11Device()
                , bytes.data(), bytes.size(), "rect", "VS", "GS", "PS")) {
                throw Platform::Exception::CreateException(E_FAIL);
            }
            auto ia = std::make_shared<dxgiutil::InputAssemblerSource>();
            if (!ia->CreateRectForGS(m_deviceManager->GetD3D11Device())) {
                throw Platform::Exception::CreateException(E_FAIL);
            }
            rect->SetIA(ia);
            m_deferredRenderer->AddPipeline(rect);
            m_renderTexture = m_deviceManager->CreateTexture(m_width, m_height, 4);
            rect->SetTexture("tex0", m_renderTexture);
            auto sampler = m_deviceManager->CreateSampler();
            rect->SetSampler("sample0", sampler);
        });

        std::vector<concurrency::task<void>> tasks;
        for (int i = 0; i < m_markerShaders.size(); ++i)
        {
            auto markerTask = DX::ReadDataAsync(L"marker.hlsl.txt")
                .then([this, i](const std::vector<unsigned char> &bytes) {

                auto marker = m_markerShaders[i];
                if (!marker->Initialize(m_deviceManager->GetD3D11Device(),
                    bytes.data(), bytes.size(), "marker", "VS", "", "PS")) {
                    throw Platform::Exception::CreateException(E_FAIL);
                }
                auto ia = std::make_shared<dxgiutil::InputAssemblerSource>();
                if (!ia->CreateRect(m_deviceManager->GetD3D11Device(), m_detector->GetMarkerSize())) {
                    throw Platform::Exception::CreateException(E_FAIL);
                }
                marker->SetIA(ia);
            });
            tasks.push_back(markerTask);
        }

        (captureRectTask && renderRectTask
            && tasks[0]
            && tasks[1]
            && tasks[2]
            && tasks[3]
            ).then([this]() {

            m_loadingComplete = true;

        });
    }

    void CaptureRenderer::CreateDeviceDependentResources()
    {
        auto device = m_deviceResources->GetD3DDevice();
        m_deviceManager = std::make_shared<dxgiutil::DeviceManager>(device);

        m_deferredRenderer = std::make_shared<dxgiutil::DeferredDeviceContext>(
            device);

        m_deferredCapture = std::make_shared<dxgiutil::DeferredDeviceContext>(
            device);
        m_deferredRenderer->AddSubcontext(m_deferredCapture);

        m_deferredMarker = std::make_shared<dxgiutil::DeferredDeviceContext>(
            device);
        m_deferredRenderer->AddSubcontext(m_deferredMarker);
    }

    void CaptureRenderer::ReleaseDeviceDependentResources()
    {
        m_loadingComplete = false;
        for (auto marker : m_markerShaders) {

            //marker->Finalize();

        }
        m_captureTexture.reset();
        m_renderTexture.reset();
        m_deferredMarker.reset();
        m_deferredCapture.reset();
        m_deferredRenderer.reset();
        m_deviceManager.reset();
    }

    void CaptureRenderer::Update(DX::StepTimer const& timer)
    {

    }

    void CaptureRenderer::Render()
    {
        if (!m_loadingComplete)return;

        auto rtv = m_deviceResources->GetBackBufferRenderTargetView();
        m_deferredRenderer->RenderPipelines(rtv, m_width, m_height);
        {
            m_deferredRenderer->ExecuteCommandList(m_deviceResources->GetD3DDeviceContext());
        }
    }

    concurrency::task<void> CaptureRenderer::StartCaptureAsync()
    {
        if (!m_capture.Get()) {
            return concurrency::task_from_result();
        }

        return
            concurrency::create_task(Windows::Media::Capture::Frames::MediaFrameSourceGroup::FindAllAsync())
            .then([](Windows::Foundation::Collections::IVectorView<Windows::Media::Capture::Frames::MediaFrameSourceGroup^>^ allGroups)
                ->Windows::Media::Capture::Frames::MediaFrameSourceInfo^
        {
            for (auto group : allGroups)
            {
                auto sourceInfo = GetFirstSourceInfoOfKind(group, Windows::Media::Capture::Frames::MediaFrameSourceKind::Color);
                if (sourceInfo) {
                    return sourceInfo;
                }
            }
            return nullptr;
        })
            .then([this](Windows::Media::Capture::Frames::MediaFrameSourceInfo^ info)
        {
            if (info) {
                return
                concurrency::create_task(m_capture->CreateFrameReaderAsync(m_capture->FrameSources->Lookup(info->Id)))
                    .then([this](Windows::Media::Capture::Frames::MediaFrameReader^ frameReader) {

                    auto callback = ref new ReaderCallback([this](const cv::Mat &bitmap) {
                        OnFrame(bitmap);
                    });
                    m_callback = callback;

                    m_reader = frameReader;

                    frameReader->FrameArrived += ref new Windows::Foundation::TypedEventHandler<
                        Windows::Media::Capture::Frames::MediaFrameReader^
                        , Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs^>(callback, &ReaderCallback::FrameReader_FrameArrived);

                    return concurrency::create_task(frameReader->StartAsync());

                })
                    .then([](Windows::Media::Capture::Frames::MediaFrameReaderStartStatus status) {

                        // done
                    if (status != Windows::Media::Capture::Frames::MediaFrameReaderStartStatus::Success)
                    {
                        //m_logger->Log("Unable to start " + info->SourceKind.ToString() + "  reader. Error: " + status.ToString());
                        int a = 0;
                    }
                    else {
                        int b = 0;
                    }

                    });
            }
        })
            ;
    }

    void CaptureRenderer::OnFrame(const cv::Mat &TheInputImage)
    {
        if (!m_loadingComplete) {
            return;
        }

        {
            std::lock_guard<std::mutex> scoped(m_captureMutex);
            if (m_captureTaskWorking)return;
            m_captureTaskWorking = true;
        }

        cv::Mat TheResizedImage;
        cv::Mat The32bit;

        ResizeMat(TheInputImage, m_width, m_height, TheResizedImage);
        cv::cvtColor(TheResizedImage, The32bit, CV_BGR2RGBA, 4);

        ///////////////////////////////////
        // intrinsics
        float ratioX = m_width / (float)TheInputImage.size().width;
        float ratioY = m_height / (float)TheInputImage.size().height;
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
        float fovy = (40.0f/180.0f) * DirectX::XM_PI;
        auto p = DirectX::XMMatrixPerspectiveFovLH(fovy, aspect, zNear, zFar);
        DirectX::XMFLOAT4X4 projMatrix;
        DirectX::XMStoreFloat4x4(&projMatrix, p);

        // View
        auto sy = DirectX::XMMatrixScaling(1.0f, -1.0f, 1.0f);
        DirectX::XMFLOAT4X4 viewMatrix;
        DirectX::XMStoreFloat4x4(&viewMatrix, sy);

        cv::Mat grey;
        cv::cvtColor(TheInputImage, grey, CV_BGRA2BGR);

        try {
            // Model
            auto markers = m_detector->Detect(grey, intrinsics);
            if (markers > m_markerShaders.size()) {
                markers = m_markerShaders.size();
            }

            m_deferredMarker->ResizePipelines(markers);
            for (size_t i = 0; i < markers; ++i) {
                float modelview_matrix[16];
                m_detector->GetModelViewMatrix(i, modelview_matrix);
                auto marker = m_markerShaders[i];
                m_deferredMarker->SetPipeline(i, marker);

                marker->SetCB("ModelMatrix", *((const DirectX::XMFLOAT4X4*)modelview_matrix));
                marker->SetCB("ViewMatrix", viewMatrix);
                marker->SetCB("ProjectionMatrix", projMatrix);
            }
            auto rtv = m_renderTexture->GetRTV();
            if (rtv) {
                m_deferredMarker->RenderPipelines(rtv, m_width, m_height);
            }
        }
        catch (const cv::Exception &ex)
        {
            //std::cerr << ex.what() << std::endl;
        }

        // update capture texture
        m_deferredCapture->OnFrame(m_captureTexture->GetTexture()
            , The32bit.data, The32bit.step*The32bit.rows, The32bit.step);

        {
            std::lock_guard<std::mutex> scoped(m_captureMutex);
            m_captureTaskWorking = false;
        }
    }
}
