#pragma once

#ifndef SHADER_CREATE_INFO_H
#define SHADER_CREATE_INFO_H

#include "d3d11.h"

template <typename T>
struct ShaderCreateInfo;

template<>
struct ShaderCreateInfo<ID3D11VertexShader>
{
    static constexpr const char* Target = "vs_5_0";
    static constexpr const char* Suffix = " vertex shader";

    static HRESULT Create(ID3D11Device* Device, ID3D10Blob* Blob, ID3D11VertexShader** OutShader)
    {
        return Device->CreateVertexShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, OutShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11HullShader>
{
    static constexpr const char* Target = "hs_5_0";
    static constexpr const char* Suffix = " hull shader";

    static HRESULT Create(ID3D11Device* Device, ID3D10Blob* Blob, ID3D11HullShader** OutShader)
    {
        return Device->CreateHullShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, OutShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11DomainShader>
{
    static constexpr const char* Target = "ds_5_0";
    static constexpr const char* Suffix = " domain shader";

    static HRESULT Create(ID3D11Device* Device, ID3D10Blob* Blob, ID3D11DomainShader** OutShader)
    {
        return Device->CreateDomainShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, OutShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11GeometryShader>
{
    static constexpr const char* Target = "gs_5_0";
    static constexpr const char* Suffix = " geometry shader";

    static HRESULT Create(ID3D11Device* Device, ID3D10Blob* Blob, ID3D11GeometryShader** OutShader)
    {
        return Device->CreateGeometryShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, OutShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11PixelShader>
{
    static constexpr const char* Target = "ps_5_0";
    static constexpr const char* Suffix = " pixel shader";

    static HRESULT Create(ID3D11Device* Device, ID3D10Blob* Blob, ID3D11PixelShader** OutShader)
    {
        return Device->CreatePixelShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, OutShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11ComputeShader>
{
    static constexpr const char* Target = "cs_5_0";
    static constexpr const char* Suffix = " compute shader";

    static HRESULT Create(ID3D11Device* Device, ID3D10Blob* Blob, ID3D11ComputeShader** OutShader)
    {
        return Device->CreateComputeShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, OutShader);
    }
};

#endif
