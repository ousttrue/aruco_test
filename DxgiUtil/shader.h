#pragma once
#include <wrl/client.h>
#include <memory>
#include <string>
#include <map>
#include <D3D11.h>
#include <DirectXMath.h>


namespace imageutil{
	class Image;
}

namespace dxgiutil {

	// input-assembler
	struct Vertex
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 tex;
	};


	class InputAssemblerSource
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuf;
		D3D11_PRIMITIVE_TOPOLOGY m_topology;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuf;
		int m_count;

	public:
		InputAssemblerSource();
		bool CreateRect(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice, float size);
		bool CreateCube(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice, float size);
		bool CreateRectForGS(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice);
		void Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pDeviceContext);

	private:
		bool createVB(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice, float size);
		bool createIB(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice);
	};

	class Texture;
	class Shader
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVsh;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_pGsh;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPsh;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
		std::shared_ptr<InputAssemblerSource> m_IASource;
		std::shared_ptr<class ConstantBuffer> m_constant;

		std::map<std::string, std::shared_ptr<Texture>> m_textures;
		std::map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplers;
		std::map<std::string, DirectX::XMFLOAT4X4> m_cbMatrixMap;
	public:
		Shader();
		std::shared_ptr<class ConstantBuffer> GetConstantBuffer()const{ return m_constant; }
		void SetIA(const std::shared_ptr<InputAssemblerSource> &ia){ m_IASource = ia; }
		void SetTexture(const std::string &name, const std::shared_ptr<Texture> &texture){ m_textures[name] = texture; }
		void SetSampler(const std::string &name, Microsoft::WRL::ComPtr<ID3D11SamplerState> &sampler){ m_samplers[name] = sampler; }
		void SetCB(const std::string &name, const DirectX::XMFLOAT4X4 &m){ m_cbMatrixMap[name] = m; }
		bool Initialize(
			const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice
			, const std::wstring &shaderFile
			, const std::string &vsFunc, const std::string &gsFunc, const std::string &psFunc
			);
        bool Initialize(
            const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice
            , const void *data, size_t size, const char *fileName
            , const std::string &vsFunc, const std::string &gsFunc, const std::string &psFunc
        );
        void Draw(
			const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pDeviceContext);
		std::wstring Animation();

	private:
		bool createShaders(
			const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice
			, const void *data, size_t size, const char *fileName
			, const std::string &vsFunc, const std::string &gsFunc, const std::string &psFunc
			);
	};
}
