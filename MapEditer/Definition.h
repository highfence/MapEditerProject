#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct  MyVertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;

	XMFLOAT3 normal;
	XMFLOAT2 tex;
};

struct ConstantBuffer
{
	XMMATRIX wvp;
	XMMATRIX world;

	XMFLOAT4 lightDir;
	XMFLOAT4 lightColor;
};
