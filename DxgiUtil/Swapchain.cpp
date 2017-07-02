#include "swapchain.h"
#include "texture.h"
#include <iostream>
#include <assert.h>


namespace dxgiutil {

Swapchain::Swapchain(const Microsoft::WRL::ComPtr<IDXGIOutput> &output)
	: m_output(output), m_requestWidth(0), m_requestHeight(0)
	, m_fullscreenRequest(FR_NONE), m_isFullscreen(FALSE)
	, m_fullscreenState(FS_UNKNOWN)
{}

Swapchain::~Swapchain()
{
	auto swapchain = m_swapchain;
	if (!m_swapchain){
		return;
	}

	BOOL isFullscreen = FALSE;
	swapchain->GetFullscreenState(&isFullscreen, NULL);
	if (!isFullscreen)
	{
		return;
	}

	// clean up
	m_swapchain->SetFullscreenState(FALSE, nullptr);
	m_swapchain.Reset();
}

bool Swapchain::Create(const Microsoft::WRL::ComPtr<IDXGIDevice1> &dxgiDevice, HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	m_requestWidth = rect.right - rect.left;
	m_requestHeight = rect.bottom - rect.top;

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	if (FAILED(dxgiDevice->GetAdapter(&dxgiAdapter))) {
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	if (FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)))) {
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 desc = { 0 };
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	// desc.Scaling = DXGI_SCALING_NONE; for windows8
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	auto hr = dxgiFactory->CreateSwapChainForHwnd(dxgiDevice.Get(),
		hWnd,
		&desc,
		nullptr,
		nullptr,
		&m_swapchain);
	if (FAILED(hr)) {
		return false;
	}
	m_swapchain->GetDesc(&m_sdesc);

	if (FAILED(dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER))){
		return false;
	}
	if (FAILED(dxgiDevice->SetMaximumFrameLatency(1))) {
		return false;
	}

	m_fullscreenState = FS_WINDOWED;
	return true;
}

std::shared_ptr<Texture> Swapchain::GetBackBuffer()
{
	if (!m_swapchain){
		return nullptr;
	}

	switch (m_fullscreenState)
	{
	case FS_WINDOWED:
		return GetBackBuffer_Windowed();

	case FS_FULLSCREEN_ENTERING:
		return GetBackBuffer_FullscreenEntering();

	case FS_FULLSCREEN:
		return GetBackBuffer_Fullscreen();

	case FS_FULLSCREEN_LEAVING:
		return GetBackBuffer_FullscreenLeaving();

	default:
		assert(false);
		return nullptr;
	}
}

std::shared_ptr<Texture> Swapchain::GetBackBuffer_Windowed()
{
	m_isFullscreen = false;

	// check fullscreenRequest
	switch (m_fullscreenRequest)
	{
	case FR_ENABLE:
	{
		if (m_texture){
			m_texture.reset();
			return nullptr;
		}

		// set fullscreen
		m_swapchain->SetFullscreenState(true, m_output.Get());
		m_fullscreenState = FS_FULLSCREEN_ENTERING;
		return nullptr;
	}
	}
	m_fullscreenRequest = FR_NONE;

	if (m_texture){
		// size check
		if (m_requestWidth != m_sdesc.BufferDesc.Width
			||m_requestHeight != m_sdesc.BufferDesc.Height)
		{
			m_texture.reset();
			return nullptr;
		}
	}

	if (!m_texture){
		// resize
		auto hr = m_swapchain->ResizeBuffers(m_sdesc.BufferCount
			, m_requestWidth, m_requestHeight
			, m_sdesc.BufferDesc.Format, m_sdesc.Flags);
		if (FAILED(hr)){
			return nullptr;
		}
		m_swapchain->GetDesc(&m_sdesc);

		// get
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		if (FAILED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&texture)))){
			return nullptr;
		}
		m_texture = std::make_shared<Texture>(texture);

		std::cout
			<< "WINDOWED "
			<< " " << m_texture->GetDesc().Width << "X" << m_texture->GetDesc().Height
			<< std::endl
			;
	}

	return m_texture;
}

std::shared_ptr<Texture> Swapchain::GetBackBuffer_FullscreenEntering()
{
	BOOL isFullscreen;
	m_swapchain->GetFullscreenState(&isFullscreen, NULL);
	if (!isFullscreen){
		// wait for fullscreen...
		return nullptr;
	}

	m_fullscreenState = FS_FULLSCREEN;
	return nullptr;
}

std::shared_ptr<Texture> Swapchain::GetBackBuffer_Fullscreen()
{
	BOOL isFullscreen;
	m_swapchain->GetFullscreenState(&isFullscreen, NULL);
	if (!isFullscreen){
		m_fullscreenState = FS_WINDOWED;
		return nullptr;
	}
	m_isFullscreen = true;

	// check fullscreenRequest
	switch (m_fullscreenRequest)
	{
	case FR_DISABLE:
	{
		m_texture.reset();
		m_fullscreenState = FS_FULLSCREEN_LEAVING;
		m_fullscreenRequest = FR_NONE;

		// set fullscreen
		m_swapchain->SetFullscreenState(false, nullptr);

		return nullptr;
	}
	}
	m_fullscreenRequest = FR_NONE;

	if (!m_texture){
		// resize
		auto hr = m_swapchain->ResizeBuffers(m_sdesc.BufferCount
			//, m_sdesc.BufferDesc.Width, m_sdesc.BufferDesc.Height
			, m_requestWidth, m_requestHeight
			, m_sdesc.BufferDesc.Format, m_sdesc.Flags);
		if (FAILED(hr)){
			return nullptr;
		}
		m_swapchain->GetDesc(&m_sdesc);

		// get
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		if (FAILED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&texture)))){
			return nullptr;
		}
		m_texture = std::make_shared<Texture>(texture);

		std::cout
			<< "FULLSCREEN "
			<< " " << m_texture->GetDesc().Width << "X" << m_texture->GetDesc().Height
			<< std::endl
			;
	}

	return m_texture;
}

std::shared_ptr<Texture> Swapchain::GetBackBuffer_FullscreenLeaving()
{
	BOOL isFullscreen;
	m_swapchain->GetFullscreenState(&isFullscreen, NULL);
	if (isFullscreen){
		// wait for windowed...
		return nullptr;
	}

	m_fullscreenState = FS_WINDOWED;
	return nullptr;
}

void Swapchain::Present()
{
	m_swapchain->Present(1, 0);
}

} // namespace
