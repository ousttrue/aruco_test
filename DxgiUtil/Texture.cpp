#include "Texture.h"
#include <assert.h>


namespace dxgiutil {

Texture::Texture(const Microsoft::WRL::ComPtr<ID3D11Device> &device
        , int width, int height, int pixelBytes)
{
    m_tdesc.Width = width;
    m_tdesc.Height = height;
    m_tdesc.MipLevels = 1;
    m_tdesc.ArraySize = 1;
    switch(pixelBytes)
    {
        case 2:
            m_tdesc.Format = DXGI_FORMAT_R8G8_UNORM;
            break;

		case 3:
			m_tdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;

        case 4:
            m_tdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            break;

        default:
            assert(false);
            break;
    }

	m_tdesc.SampleDesc.Count = 1;
    m_tdesc.SampleDesc.Quality = 0;
    m_tdesc.Usage = D3D11_USAGE_DEFAULT;
	m_tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE
		| D3D11_BIND_RENDER_TARGET
		;
    m_tdesc.CPUAccessFlags = 0;
    m_tdesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&m_tdesc, NULL, &m_texture);
    if (FAILED(hr)) { return; }
    
    m_texture->GetDesc(&m_tdesc);
}

Texture::Texture(const Microsoft::WRL::ComPtr<ID3D11Texture2D> &texture)
: m_texture(texture)
{
    m_texture->GetDesc(&m_tdesc);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture::GetSRV()
{
    if(!m_srv){
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		m_texture->GetDevice(&device);
		device->CreateShaderResourceView(m_texture.Get(), NULL, &m_srv);
    }
    return m_srv;
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Texture::GetRTV()
{
    if(!m_rtv){
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		m_texture->GetDevice(&device);
		device->CreateRenderTargetView(m_texture.Get(), NULL, &m_rtv);
    }
    return m_rtv;
}

Microsoft::WRL::ComPtr<ID2D1Bitmap1> Texture::GetD2D1Bitmap(const Microsoft::WRL::ComPtr<ID2D1DeviceContext> &d2d)
{
	if (!m_bitmap){
		Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
		if (FAILED(m_texture.As(&dxgiSurface))) {
			return nullptr;
		}

		const auto bp = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
		if (FAILED(d2d->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bp, &m_bitmap))) {
			return nullptr;
		}
	}
	return m_bitmap;
}

}

#if 0
bool Texture::Initialize(const Microsoft::WRL::ComPtr<ID3D11Device> &device
	, const std::shared_ptr<imageutil::Image> &image)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = image->Width();
	desc.Height = image->Height();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = image->Pointer();
	initData.SysMemPitch = image->Stride();
	initData.SysMemSlicePitch = image->Size();

	auto hr = device->CreateTexture2D(&desc, &initData, &m_texture);
	if (FAILED(hr)){
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_texture.Get(), &SRVDesc, &m_srv);
	if (FAILED(hr))
	{
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	hr = device->CreateSamplerState(&samplerDesc, &m_sampler);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}


#endif