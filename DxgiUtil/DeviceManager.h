#pragma once
#include <d3d11.h>
#include <d2d1_1.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <memory>
#include <string>
#include <vector>

namespace dxgiutil {

class Swapchain;
class Texture;
class Shader;
class DeviceManager
{
    Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;

	DeviceManager(const DeviceManager &);
	DeviceManager& operator=(const DeviceManager &);
public:
    DeviceManager(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice);
    static std::shared_ptr<DeviceManager> Create();
	static std::shared_ptr<DeviceManager> Create(int adapterIndex);
	static std::shared_ptr<DeviceManager> Create(
            const Microsoft::WRL::ComPtr<IDXGIAdapter> &adapter);

	Microsoft::WRL::ComPtr<IDXGIFactory1> GetDxgiFactory();
	Microsoft::WRL::ComPtr<IDXGIDevice1> GetDxgiDevice();
	Microsoft::WRL::ComPtr<IDXGIAdapter1> GetDxgiAdapter();

	Microsoft::WRL::ComPtr<ID3D11Device> GetD3D11Device()const{ return m_pDevice; }
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> GetD2D1DeviceContext();

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSampler();

#ifndef UWP
	std::shared_ptr<Swapchain> CreateSwapchain(HWND hWnd);
	std::shared_ptr<Swapchain> CreateSwapchain(
            const Microsoft::WRL::ComPtr<IDXGIOutput> &output, HWND hWnd);
#endif

    std::shared_ptr<Texture> CreateTexture(int w, int h, int pixelBytes);

    std::shared_ptr<Shader> CompilePipeline(
            const std::wstring &shaderPath
            , const std::string &vs, const std::string &gs, const std::string &ps);
};

struct Output
{
	Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> pOutput;

	Output(const Microsoft::WRL::ComPtr<IDXGIAdapter1> &adapter, const Microsoft::WRL::ComPtr<IDXGIOutput> &output)
		: pAdapter(adapter), pOutput(output)
	{}
};

inline std::ostream& operator<<(std::ostream &os, const Output &output)
{
    DXGI_ADAPTER_DESC adesc;
    output.pAdapter->GetDesc(&adesc);
    auto adapterName = std::wstring(adesc.Description);

    DXGI_OUTPUT_DESC odesc;
    output.pOutput->GetDesc(&odesc);
    auto name = std::wstring(odesc.DeviceName);

    os
        << std::string(adapterName.begin(), adapterName.end())
        << " " << std::string(name.begin(), name.end())
        ;

    return os;
}

inline std::vector<Output> GetOutputs()
{
	std::vector<Output> outputs;

	Microsoft::WRL::ComPtr<IDXGIFactory1> pFactory;
	CreateDXGIFactory1(_uuidof(IDXGIFactory1), (void**)&pFactory);
	for (unsigned int index = 0;; index++){
		Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
		HRESULT	ret = pFactory->EnumAdapters1(index, &pAdapter);
		if (ret == DXGI_ERROR_NOT_FOUND){
			break;
		}
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		// output
		for (int j = 0;; ++j)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput> pOutput;
			if (FAILED(pAdapter->EnumOutputs(j, &pOutput))){
				break;
			}
			DXGI_OUTPUT_DESC desc;
			pOutput->GetDesc(&desc);

			outputs.push_back(Output(pAdapter, pOutput));
		}
	}

	return outputs;
}

}
