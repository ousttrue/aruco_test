#include "DeviceManager.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Shader.h"


namespace dxgiutil {


DeviceManager::DeviceManager(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice)
	: m_pDevice(pDevice)
{
}

std::shared_ptr<DeviceManager> DeviceManager::Create()
{
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
    UINT flags=D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	flags |= D3D11_CREATE_DEVICE_DEBUG
		;
#endif
	D3D_FEATURE_LEVEL featureLevelSupported;
	auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
		flags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&pDevice,
		&featureLevelSupported,
		nullptr);
	if (FAILED(hr)){
		return nullptr;
	}

	return std::shared_ptr<DeviceManager>(new DeviceManager(pDevice));
}

std::shared_ptr<DeviceManager> DeviceManager::Create(int adapterIndex)
{
	Microsoft::WRL::ComPtr<IDXGIFactory1> pFactory;
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr)){
		return nullptr;
	}
	for (int index = 0;; index++){
		Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
		HRESULT	ret = pFactory->EnumAdapters1(index, &pAdapter);
		if (ret == DXGI_ERROR_NOT_FOUND){
			break;
		}
		if (index == adapterIndex){
            return Create(pAdapter);
		}
	}
	return nullptr;
}

std::shared_ptr<DeviceManager> DeviceManager::Create(
        const Microsoft::WRL::ComPtr<IDXGIAdapter> &pAdapter)
{
    Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevelSupported;
    // if adapter is not null, then flag must be D3D_DRIVER_TYPE_UNKNOWN
    auto hr = D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
            flags,
            nullptr, 0,
            D3D11_SDK_VERSION,
            &pDevice,
            &featureLevelSupported,
            nullptr);
    if (FAILED(hr)){
        return nullptr;
    }

    return std::shared_ptr<DeviceManager>(new DeviceManager(pDevice));
}

Microsoft::WRL::ComPtr<IDXGIFactory1> DeviceManager::GetDxgiFactory()
{
	auto dxgiDevice = GetDxgiDevice();
	if (!dxgiDevice){
		return nullptr;
	}
	Microsoft::WRL::ComPtr<IDXGIFactory1> pDXGIFactory;
	auto hr=dxgiDevice->GetParent(IID_PPV_ARGS(&pDXGIFactory));
	if (FAILED(hr)){
		return nullptr;
	}
	return pDXGIFactory;
}

Microsoft::WRL::ComPtr<IDXGIDevice1> DeviceManager::GetDxgiDevice()
{
    Microsoft::WRL::ComPtr<IDXGIDevice1> pDXGIDevice;
	auto hr = m_pDevice.As(&pDXGIDevice);
	if (FAILED(hr)){
		return nullptr;
	}
    return pDXGIDevice;
}

Microsoft::WRL::ComPtr<IDXGIAdapter1> DeviceManager::GetDxgiAdapter()
{
	auto device = GetDxgiDevice();
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	if (FAILED(device->GetAdapter(&adapter))){
		return nullptr;
	}
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
	if (FAILED(adapter.As(&adapter1))){
		return nullptr;
	}
	return adapter1;
}

Microsoft::WRL::ComPtr<ID2D1DeviceContext> DeviceManager::GetD2D1DeviceContext()
{
	// ID2D1Factory1
	Microsoft::WRL::ComPtr<ID2D1Factory1> d2d1Factory;
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2d1Factory)))) {
		return nullptr;
	}

	// ID2D1Device
	Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
	if (FAILED(d2d1Factory->CreateDevice(GetDxgiDevice().Get(), &d2dDevice))) {
		return nullptr;
	}

	// ID2D1DeviceContext
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dDeviceContext;
	if (FAILED(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext))) {
		return nullptr;
	}

	return d2dDeviceContext;
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> DeviceManager::CreateSampler()
{
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC desc;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.BorderColor[0] = 0.0f;
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;
	desc.BorderColor[3] = 0.0f;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 2;
	desc.MinLOD = FLT_MAX * (-1);
	desc.MaxLOD = FLT_MAX;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	if (FAILED(GetD3D11Device()->CreateSamplerState(&desc, &sampler))){
		return nullptr;
	}
	return sampler;
}

#ifndef UWP
std::shared_ptr<Swapchain> DeviceManager::CreateSwapchain(HWND hWnd)
{
	auto adapter = GetDxgiAdapter();
	if (!adapter){
		return nullptr;
	}
	Microsoft::WRL::ComPtr<IDXGIOutput> output;
	if (FAILED(adapter->EnumOutputs(0, &output))){
		return nullptr;
	}
	return CreateSwapchain(output, hWnd);
}

std::shared_ptr<Swapchain> DeviceManager::CreateSwapchain(
        const Microsoft::WRL::ComPtr<IDXGIOutput> &output, HWND hWnd)
{
    auto swapchain=std::make_shared<Swapchain>(output);
    if(!swapchain->Create(GetDxgiDevice(), hWnd)){
        return nullptr;
    }

    return swapchain;
}
#endif

std::shared_ptr<Texture> DeviceManager::CreateTexture(int w, int h, int pixelBytes)
{
	return std::make_shared<Texture>(GetD3D11Device(), w, h, pixelBytes);
}

std::shared_ptr<Shader> DeviceManager::CompilePipeline(
        const std::wstring &shaderPath
        , const std::string &vs, const std::string &gs, const std::string &ps)
{
    auto pipeline=std::make_shared<Shader>();
    if(!pipeline->Initialize(GetD3D11Device(), shaderPath, vs, gs, ps)){
        return nullptr;
    }
    return pipeline;
}

} // namespace

