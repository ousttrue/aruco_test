#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <utility>
#include <memory>
#include <wrl/client.h>

namespace dxgiutil {

class Texture;
class Swapchain
{
    Microsoft::WRL::ComPtr<IDXGIOutput> m_output;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapchain;
	DXGI_SWAP_CHAIN_DESC m_sdesc;
	BOOL m_isFullscreen;
    std::shared_ptr<Texture> m_texture;

	enum FullscreenRequest
	{
		FR_NONE,
		FR_ENABLE,
		FR_DISABLE,
	};
	FullscreenRequest m_fullscreenRequest;
	int m_requestWidth;
	int m_requestHeight;

	enum FullScreenState
	{
		FS_UNKNOWN,
		FS_WINDOWED, 
		FS_FULLSCREEN_ENTERING, // size changed. wait front buffer resizing
		FS_FULLSCREEN, 
		FS_FULLSCREEN_LEAVING, // state changed. wait resize window
	};
	FullScreenState m_fullscreenState;

public:
    Swapchain(const Microsoft::WRL::ComPtr<IDXGIOutput> &output);
    ~Swapchain();
	bool Create(const Microsoft::WRL::ComPtr<IDXGIDevice1> &device, HWND hWnd);

	void SetSize(int width, int height)
	{
		m_requestWidth = width;
		m_requestHeight = height;
	}
	BOOL IsFullscreen()const
	{
		return m_isFullscreen;
	}
	void SetFullscreen(BOOL isFullscreen)
	{
		m_fullscreenRequest = isFullscreen ? FR_ENABLE : FR_DISABLE;
	}

    void Present();
	std::shared_ptr<Texture> GetBackBuffer();
private:
	std::shared_ptr<Texture> GetBackBuffer_Windowed();
	std::shared_ptr<Texture> GetBackBuffer_FullscreenEntering();
	std::shared_ptr<Texture> GetBackBuffer_Fullscreen();
	std::shared_ptr<Texture> GetBackBuffer_FullscreenLeaving();
};

} // namespace
