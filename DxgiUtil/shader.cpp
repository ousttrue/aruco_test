#include "shader.h"
#include "imageutil.h"
#include "CompileShaderFromFile.h"
#include "constantbuffer.h"
#include "Texture.h"
#include "debugprint.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <sstream>
#include <iomanip>
#include <DirectXMath.h>

namespace dxgiutil {

    InputAssemblerSource::InputAssemblerSource()
        : m_count(0)
    {}

    bool InputAssemblerSource::CreateRect(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice, float size)
    {
        auto halfSize = size * 0.5f;
        Vertex vertices[] =
        {
            { DirectX::XMFLOAT4(-halfSize, -halfSize, 0, 1.0f), DirectX::XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(-halfSize, halfSize, 0, 1.0f), DirectX::XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 0) },
            { DirectX::XMFLOAT4(halfSize, -halfSize, 0, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(halfSize, halfSize, 0, 1.0f), DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), DirectX::XMFLOAT2(1, 1) },
        };
        unsigned int vsize = sizeof(vertices);

        D3D11_BUFFER_DESC vdesc;
        ZeroMemory(&vdesc, sizeof(vdesc));
        vdesc.ByteWidth = vsize;
        vdesc.Usage = D3D11_USAGE_DEFAULT;
        vdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vdesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vertexData;
        ZeroMemory(&vertexData, sizeof(vertexData));
        vertexData.pSysMem = vertices;

        HRESULT hr = pDevice->CreateBuffer(&vdesc, &vertexData, m_pVertexBuf.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        m_count = 4;

        return true;
    }

    bool InputAssemblerSource::CreateCube(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice, float size)
    {
        if (!createVB(pDevice, size)) {
            return false;
        }
        if (!createIB(pDevice)) {
            return false;
        }

        m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        return true;
    }

    bool InputAssemblerSource::CreateRectForGS(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice)
    {
        D3D11_BUFFER_DESC vdesc;
        ZeroMemory(&vdesc, sizeof(vdesc));
        vdesc.ByteWidth = 2;
        vdesc.Usage = D3D11_USAGE_DEFAULT;
        vdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vdesc.CPUAccessFlags = 0;
        auto hr = pDevice->CreateBuffer(&vdesc, nullptr, &m_pVertexBuf);
        if (FAILED(hr)) {
            return false;
        }

        m_topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        m_count = 2;

        return true;
    }

    void InputAssemblerSource::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pDeviceContext)
    {
        ID3D11Buffer* pBufferTbl[] = { m_pVertexBuf.Get() };
        UINT SizeTbl[] = { sizeof(Vertex) };
        UINT OffsetTbl[] = { 0 };
        pDeviceContext->IASetVertexBuffers(0, 1, pBufferTbl, SizeTbl, OffsetTbl);

        pDeviceContext->IASetPrimitiveTopology(m_topology);

        if (m_pIndexBuf) {
            // with index buffer
            pDeviceContext->IASetIndexBuffer(m_pIndexBuf.Get(), DXGI_FORMAT_R32_UINT, 0);
            pDeviceContext->DrawIndexed(m_count, 0, 0);
        }
        else {
            // without index buffer
            pDeviceContext->Draw(m_count, 0);
        }
    }


    bool InputAssemblerSource::createVB(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice, float _size)
    {
        // Create VB
        auto size = _size * 0.5f;
        Vertex pVertices[] =
        {
            // x
            { DirectX::XMFLOAT4(-size, -size, -size, 1.0f), DirectX::XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(-size, -size, size, 1.0f), DirectX::XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 0) },
            { DirectX::XMFLOAT4(-size, size, size, 1.0f), DirectX::XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(-size, size, -size, 1.0f), DirectX::XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 1) },

            { DirectX::XMFLOAT4(size, -size, -size, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(size, size, -size, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 1) },
            { DirectX::XMFLOAT4(size, size, size, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(size, -size, size, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 0) },
            // y
            { DirectX::XMFLOAT4(-size, size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(-size, size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 0) },
            { DirectX::XMFLOAT4(size, size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(size, size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 1) },

            { DirectX::XMFLOAT4(-size, -size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(size, -size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 1) },
            { DirectX::XMFLOAT4(size, -size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(-size, -size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), DirectX::XMFLOAT2(0, 0) },
            // z
            { DirectX::XMFLOAT4(-size, -size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(-size, size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT2(0, 0) },
            { DirectX::XMFLOAT4(size, size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(size, -size, -size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT2(1, 1) },

            { DirectX::XMFLOAT4(-size, -size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(0, 1) },
            { DirectX::XMFLOAT4(size, -size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(1, 1) },
            { DirectX::XMFLOAT4(size, size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(1, 0) },
            { DirectX::XMFLOAT4(-size, size, size, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(0, 0) },
        };
        unsigned int vsize = sizeof(pVertices);

        D3D11_BUFFER_DESC vdesc;
        ZeroMemory(&vdesc, sizeof(vdesc));
        vdesc.ByteWidth = vsize;
        vdesc.Usage = D3D11_USAGE_DEFAULT;
        vdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vdesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vertexData;
        ZeroMemory(&vertexData, sizeof(vertexData));
        vertexData.pSysMem = pVertices;

        HRESULT hr = pDevice->CreateBuffer(&vdesc, &vertexData, m_pVertexBuf.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool InputAssemblerSource::createIB(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice)
    {
        unsigned int pIndices[] =
        {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20,
        };
        unsigned int isize = sizeof(pIndices);
        m_count = isize / sizeof(pIndices[0]);

        D3D11_BUFFER_DESC idesc;
        ZeroMemory(&idesc, sizeof(idesc));
        idesc.ByteWidth = isize;
        idesc.Usage = D3D11_USAGE_DEFAULT;
        idesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        idesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA indexData;
        ZeroMemory(&indexData, sizeof(indexData));
        indexData.pSysMem = pIndices;

        HRESULT hr = pDevice->CreateBuffer(&idesc, &indexData, m_pIndexBuf.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }


    Shader::Shader()
        : m_constant(new ConstantBuffer)
    {}

    bool Shader::Initialize(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice
        , const std::wstring &shaderFile
        , const std::string &vsFunc, const std::string &gsFunc, const std::string &psFunc
    )
    {
        if (!createShaders(pDevice, shaderFile, vsFunc, gsFunc, psFunc)) {
            return false;
        }

        /*
        auto wicFactory = std::make_shared<imageutil::Factory>();
        auto image=wicFactory->Load(textureFile);
        if(image){
        if (!m_texture->Initialize(pDevice, image)){
        return false;
        }
        }
        */

        // BLEND
        D3D11_RENDER_TARGET_BLEND_DESC blend;
        blend.BlendEnable = TRUE;
        blend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend.BlendOp = D3D11_BLEND_OP_ADD;
        blend.SrcBlendAlpha = D3D11_BLEND_ONE;
        blend.DestBlendAlpha = D3D11_BLEND_ZERO;
        blend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        D3D11_BLEND_DESC blendDesc = { 0 };
        blendDesc.RenderTarget[0] = blend;

        auto hr = pDevice->CreateBlendState(&blendDesc, &m_blendState);
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    void Shader::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pDeviceContext)
    {
        // set shaders
        pDeviceContext->VSSetShader(m_pVsh.Get(), NULL, 0);
        pDeviceContext->GSSetShader(m_pGsh.Get(), NULL, 0);
        pDeviceContext->PSSetShader(m_pPsh.Get(), NULL, 0);

        // 定数バッファ
        for (auto kv : m_cbMatrixMap)
        {
            auto v = m_constant->GetCBVariable(kv.first);
            m_constant->SetCBValue(v, kv.second);
        }
        m_constant->UpdateCB(pDeviceContext);
        m_constant->SetCB(pDeviceContext);

        // Texture
        for (auto kv : m_textures)
        {
            auto v = m_constant->GetSRV(kv.first);
            m_constant->SetSRV(pDeviceContext, v, kv.second->GetSRV());
        }
        // Sampler
        for (auto kv : m_samplers)
        {
            auto v = m_constant->GetSampler(kv.first);
            m_constant->SetSampler(pDeviceContext, v, kv.second);
        }
        // BlendState
        pDeviceContext->OMSetBlendState(m_blendState.Get(), 0, 0xffffffff);

        // IA InputLayout
        pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
        m_IASource->Draw(pDeviceContext);
    }

    std::wstring Shader::Animation()
    {
        std::wstringstream ss;
        {
            static float angleRadiansX = 0;
            {
                const auto DELTA = DirectX::XMConvertToRadians(0.01f);
                angleRadiansX += DELTA;
            }
            auto mx = DirectX::XMMatrixRotationZ(angleRadiansX);

            static float angleRadiansY = 0;
            {
                const auto DELTA = DirectX::XMConvertToRadians(0.02f);
                angleRadiansY += DELTA;
            }
            auto my = DirectX::XMMatrixRotationZ(angleRadiansY);

            static float angleRadiansZ = 0;
            {
                const auto DELTA = DirectX::XMConvertToRadians(0.03f);
                angleRadiansZ += DELTA;
            }
            auto mz = DirectX::XMMatrixRotationZ(angleRadiansZ);

            //auto m = DirectX::XMMatrixIdentity();
            static int s_frame = 0;
            auto m = DirectX::XMMatrixRotationRollPitchYaw(angleRadiansX, angleRadiansY, angleRadiansZ);
            ss
                << "Frame: " << (++s_frame) << std::endl
                << "X: " << std::fixed << std::setprecision(2) << angleRadiansX << std::endl
                << "Y: " << std::fixed << std::setprecision(2) << angleRadiansY << std::endl
                << "Z: " << std::fixed << std::setprecision(2) << angleRadiansZ << std::endl
                ;

            DirectX::XMFLOAT4X4 matrix;
            DirectX::XMStoreFloat4x4(&matrix, m);
            auto v = m_constant->GetCBVariable("ModelMatrix");
            m_constant->SetCBValue(v, matrix);
        }

        {
            auto hEye = DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
            auto hAt = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            auto hUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            auto m = DirectX::XMMatrixLookAtLH(hEye, hAt, hUp);

            DirectX::XMFLOAT4X4 matrix;
            DirectX::XMStoreFloat4x4(&matrix, m);
            auto v = m_constant->GetCBVariable("ViewMatrix");
            m_constant->SetCBValue(v, matrix);
        }

        {
            auto m = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), 1.0f, 1.0f, 1000.0f);
            DirectX::XMFLOAT4X4 matrix;
            DirectX::XMStoreFloat4x4(&matrix, m);
            auto v = m_constant->GetCBVariable("ProjectionMatrix");
            m_constant->SetCBValue(v, matrix);
        }

        return ss.str();
    }

    static DXGI_FORMAT GetDxgiFormat(D3D_REGISTER_COMPONENT_TYPE type, BYTE mask)
    {
        if ((mask & 0x0F) == 0x0F)
        {
            // xyzw
            switch (type)
            {
            case D3D10_REGISTER_COMPONENT_FLOAT32:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }

        if ((mask & 0x07) == 0x07)
        {
            // xyz
            switch (type)
            {
            case D3D10_REGISTER_COMPONENT_FLOAT32:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            }
        }

        if ((mask & 0x03) == 0x03)
        {
            // xy
            switch (type)
            {
            case D3D10_REGISTER_COMPONENT_FLOAT32:
                return DXGI_FORMAT_R32G32_FLOAT;
            }
        }

        if ((mask & 0x1) == 0x1)
        {
            // x
            switch (type)
            {
            case D3D10_REGISTER_COMPONENT_FLOAT32:
                return DXGI_FORMAT_R32_FLOAT;
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    bool Shader::createShaders(const Microsoft::WRL::ComPtr<ID3D11Device> &pDevice
        , const std::wstring &shaderFile
        , const std::string &vsFunc, const std::string &gsFunc, const std::string &psFunc)
    {
        // vertex shader
        {
            Microsoft::WRL::ComPtr<ID3DBlob> vblob;
            HRESULT hr = CompileShaderFromFile(shaderFile.c_str(), vsFunc.c_str(), "vs_4_0", &vblob);
            if (FAILED(hr))
                return false;
            hr = pDevice->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, &m_pVsh);
            if (FAILED(hr))
                return false;

            // vertex shader reflection
            Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
            hr = D3DReflect(vblob->GetBufferPointer(), vblob->GetBufferSize(), IID_ID3D11ShaderReflection, &pReflector);
            if (FAILED(hr))
                return false;

            OutputDebugPrintfA("#### VertexShader ####\n");
            if (!m_constant->Initialize(pDevice, SHADERSTAGE_VERTEX, pReflector)) {
                return false;
            }

            // Create InputLayout
            D3D11_SHADER_DESC shaderdesc;
            pReflector->GetDesc(&shaderdesc);
            std::vector<D3D11_INPUT_ELEMENT_DESC> vbElement;
            for (size_t i = 0; i < shaderdesc.InputParameters; ++i) {
                D3D11_SIGNATURE_PARAMETER_DESC sigdesc;
                pReflector->GetInputParameterDesc(i, &sigdesc);

                auto format = GetDxgiFormat(sigdesc.ComponentType, sigdesc.Mask);

                D3D11_INPUT_ELEMENT_DESC eledesc = {
                    sigdesc.SemanticName
                    , sigdesc.SemanticIndex
                    , format
                    , 0 // 決め打ち
                    , D3D11_APPEND_ALIGNED_ELEMENT // 決め打ち
                    , D3D11_INPUT_PER_VERTEX_DATA // 決め打ち
                    , 0 // 決め打ち
                };
                vbElement.push_back(eledesc);
            }
            if (!vbElement.empty()) {
                hr = pDevice->CreateInputLayout(&vbElement[0], vbElement.size(),
                    vblob->GetBufferPointer(), vblob->GetBufferSize(), &m_pInputLayout);
                if (FAILED(hr))
                    return false;
            }
        }

        // geometry shader
        if (!gsFunc.empty()) {
            Microsoft::WRL::ComPtr<ID3DBlob> blob;
            auto hr = CompileShaderFromFile(shaderFile.c_str(), gsFunc.c_str(), "gs_4_0", &blob);
            if (FAILED(hr)) {
                return false;
            }
            hr = pDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &m_pGsh);
            if (FAILED(hr)) {
                return false;
            }

            // shader reflection
            Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
            hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection,
                &pReflector);
            if (FAILED(hr)) {
                return false;
            }

            // Create InputLayout
            D3D11_SHADER_DESC shaderdesc;
            pReflector->GetDesc(&shaderdesc);

            OutputDebugPrintfA("#### GeometryShader ####\n");
            if (!m_constant->Initialize(pDevice, SHADERSTAGE_GEOMETRY, pReflector)) {
                return false;
            }
        }

        // pixel shader
        {
            Microsoft::WRL::ComPtr<ID3DBlob> pblob;
            auto hr = CompileShaderFromFile(shaderFile.c_str(), psFunc.c_str(), "ps_4_0", &pblob);
            if (FAILED(hr))
                return false;
            hr = pDevice->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, &m_pPsh);
            if (FAILED(hr))
                return false;

            // pixel shader reflection
            Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
            hr = D3DReflect(pblob->GetBufferPointer(), pblob->GetBufferSize(), IID_ID3D11ShaderReflection,
                &pReflector);
            if (FAILED(hr))
                return false;

            // Create InputLayout
            D3D11_SHADER_DESC shaderdesc;
            pReflector->GetDesc(&shaderdesc);

            OutputDebugPrintfA("#### PixelShader ####\n");
            if (!m_constant->Initialize(pDevice, SHADERSTAGE_PIXEL, pReflector)) {
                return false;
            }
        }

        return true;
    }

}