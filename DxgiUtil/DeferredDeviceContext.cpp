#include "DeferredDeviceContext.h"
#include "Shader.h"


namespace dxgiutil {

DeferredDeviceContext::DeferredDeviceContext(
        const Microsoft::WRL::ComPtr<ID3D11Device> &device)
{
    auto hr = device->CreateDeferredContext(0, &m_defferedContext);
}

void DeferredDeviceContext::AddPipeline(const std::shared_ptr<Shader> &pipeline)
{
	m_pipelines.push_back(pipeline);
}

void DeferredDeviceContext::AddSubcontext(const std::shared_ptr<DeferredDeviceContext> &context)
{
	m_childContexts.push_back(context);
}

void DeferredDeviceContext::RenderPipelines(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &rtv, const D3D11_VIEWPORT *vp)
{
	if (!rtv){
		return;
	}

	/*
	// subcontexts
	for (auto child : m_childContexts)
	{
		auto commandList = child->GetCommandList();
		if (commandList){
			m_defferedContext->ExecuteCommandList(commandList.Get(), false);
		}
	}
	*/

	// draw pipelines
	float clear[] = { 0, 0, 0, 0 };
	m_defferedContext->ClearRenderTargetView(rtv.Get(), clear);
	m_defferedContext->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);

	if (vp){
		m_defferedContext->RSSetViewports(1, vp);
	}

	for (auto pipeline : m_pipelines)
	{
		pipeline->Draw(m_defferedContext);
	}

	m_defferedContext->ClearState();

	Microsoft::WRL::ComPtr<ID3D11CommandList> commandList;
	if (FAILED(m_defferedContext->FinishCommandList(TRUE, &commandList))){
		return;
	}

	SetCommandList(commandList);
}

void DeferredDeviceContext::OnFrame(const Microsoft::WRL::ComPtr<ID3D11Texture2D> &texture
        , const BYTE *p, int size, int stride)
{
    D3D11_BOX box = { 0 };
    m_defferedContext->UpdateSubresource(texture.Get(), 0, NULL, p, stride, size);

	Microsoft::WRL::ComPtr<ID3D11CommandList> commandList;
	if (FAILED(m_defferedContext->FinishCommandList(TRUE, &commandList))){
		return;
	}

    SetCommandList(commandList);
}

Microsoft::WRL::ComPtr<ID3D11CommandList> DeferredDeviceContext::GetCommandList()
{
	std::lock_guard<std::mutex> lock(m_commandLock);
	return m_commandList;
}

void DeferredDeviceContext::SetCommandList(
        const Microsoft::WRL::ComPtr<ID3D11CommandList> &commandList)
{
	std::lock_guard<std::mutex> lock(m_commandLock);
	m_commandList = commandList;
}

void DeferredDeviceContext::ExecuteCommandList(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &immediate)
{
	auto command = GetCommandList();
	if (command){
		immediate->ExecuteCommandList(command.Get(), FALSE);
	}

	for (auto child : m_childContexts)
	{
		child->ExecuteCommandList(immediate);
	}
}


} // namespace
