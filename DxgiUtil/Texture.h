#pragma once
#include <d3d11.h>
#include <d2d1_1.h>
#include <wrl/client.h>

namespace dxgiutil {


class Texture
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	D3D11_TEXTURE2D_DESC m_tdesc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_bitmap;
public:
    Texture(const Microsoft::WRL::ComPtr<ID3D11Device> &device
            , int w, int h, int pixelBytes);
    Texture(const Microsoft::WRL::ComPtr<ID3D11Texture2D> &texture);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture()const{ return m_texture; }
	D3D11_TEXTURE2D_DESC GetDesc()const{ return m_tdesc; }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV();
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRTV();
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> GetD2D1Bitmap(const Microsoft::WRL::ComPtr<ID2D1DeviceContext> &d2d);
};


} // dxgiutil
