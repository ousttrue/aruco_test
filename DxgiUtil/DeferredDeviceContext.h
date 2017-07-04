#pragma once
#include <mutex>
#include <wrl/client.h>
#include <d3d11.h>
#include <vector>
#include <memory>

namespace dxgiutil {

class Shader;
class DeferredDeviceContext
{
	std::mutex m_commandLock;
    Microsoft::WRL::ComPtr<ID3D11CommandList> m_commandList;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_defferedContext;

	std::vector<std::shared_ptr<Shader>> m_pipelines;
	std::vector<std::shared_ptr<DeferredDeviceContext>> m_childContexts;

public:
    DeferredDeviceContext(
            const Microsoft::WRL::ComPtr<ID3D11Device> &device);
    Microsoft::WRL::ComPtr<ID3D11Device> GetDevice()const { return m_device; }
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetContext()const{ return m_defferedContext; }
	void AddPipeline(const std::shared_ptr<Shader> &pipeline);
	void SetPipeline(size_t index, const std::shared_ptr<Shader> &pipeline){ m_pipelines[index] = pipeline; }
	std::shared_ptr<Shader> GetPipeline(size_t index)const{
		return index < m_pipelines.size() ? m_pipelines[index] : nullptr;
	}

	void ResizePipelines(size_t n){ m_pipelines.resize(n); }
	void AddSubcontext(const std::shared_ptr<DeferredDeviceContext> &context);

	// Update Commandlist
	void OnFrame(const Microsoft::WRL::ComPtr<ID3D11Texture2D> &texture
		, const BYTE *p, int size, int stride);

	void RenderPipelines(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &rtv, const D3D11_VIEWPORT *vp);
	void RenderPipelines(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &rtv, int width, int height)
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;
		RenderPipelines(rtv, &viewport);
	}
	Microsoft::WRL::ComPtr<ID3D11CommandList> GetCommandList();
    void SetCommandList(
            const Microsoft::WRL::ComPtr<ID3D11CommandList> &commandList);

	void ExecuteCommandList(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &immediate);
};


}

