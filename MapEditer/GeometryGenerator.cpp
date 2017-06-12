#include "stdafx.h"
#include "Definition.h"
#include "xnaCollision.h"
#include "GeometryGenerator.h"

namespace DirectXFramework
{
	MeshData GeometryGenerator::CreateGrid(
		float width,
		float depth,
		UINT m,
		UINT n,
		MeshData & meshData)
	{
		UINT vertexCount = m * n;
		UINT faceCount = (m - 1) *  (n - 1) * 2;

		float halfWidth = 0.5f * width;
		float halfDepth = 0.5f * depth;

		float dx = width / (n - 1);
		float dz = depth / (m - 1);

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		meshData.Vertices.resize(vertexCount);
		for (UINT i = 0; i < m; ++i)
		{
			float z = halfDepth - i * dz;
			for (UINT j = 0; j < n; ++j)
			{
				float x = -halfWidth + j * dx;

				meshData.Vertices[i * n + j].pos = DirectX::XMFLOAT3(x, 0.0f, z);

				meshData.Vertices[i * n + j].normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
				//meshData.Vertices[i * n + j].TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

				meshData.Vertices[i * n + j].tex.x = j * du;
				meshData.Vertices[i * n + j].tex.y = i * dv;
			}
		}

		meshData.Indices32.resize(faceCount * 3);

		UINT k = 0;
		for (UINT i = 0; i < m - 1; ++i)
		{
			for (UINT j = 0; j < n - 1; ++j)
			{
				meshData.Indices32[k]	  = i * n + j;
				meshData.Indices32[k + 1] = i * n + j + 1;
				meshData.Indices32[k + 2] = (i + 1) * n + j;
				meshData.Indices32[k + 3] = (i + 1) * n + j;
				meshData.Indices32[k + 4] = i * n + j + 1;
				meshData.Indices32[k + 5] = (i + 1) * n + j + 1;

				k += 6;
			}
		}

		meshData.SetWidthNum(m);
		meshData.SetHeightNum(n);

		return meshData;
	}
}