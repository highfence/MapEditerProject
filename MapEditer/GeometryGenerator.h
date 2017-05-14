#pragma once

namespace DirectXFramework
{
	class GeometryGenerator
	{
	public :
		GeometryGenerator() = default;
		~GeometryGenerator() = default;

		MeshData CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
	private :
		
	};
}