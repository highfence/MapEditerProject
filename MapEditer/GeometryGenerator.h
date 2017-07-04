#pragma once

namespace DXMapEditer
{
	class GeometryGenerator
	{
	public :
		GeometryGenerator() = default;
		~GeometryGenerator() = default;

		void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);

	private :

	};
}