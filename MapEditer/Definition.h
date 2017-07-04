#pragma once
#include <vector>
#include <DirectXMath.h>

namespace DXMapEditer 
{
	enum class OPT_WINDOW_FUNCTIONS : int
	{
		NONE,

		CAMERA_MOVE_SPEED_CHANGE,
		CHECK_WIREFRAME,
		GO_TO_ORIGIN_CLICKED,

		PICKING_MOVE_SELECTED,
		PICKING_RISE_SELECTED,
		PICKING_DOWN_SELECTED,
		PICKING_STND_SELECTED,
		PICKING_RANGE_CHANGE,
		GRID_INITIALIZE_CLICKED,

		DATA_SAVE,
		DATA_LOAD,
		TEXTURE_SELECT_CLICKED,

		FUNC_NUM
	};

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

	struct Vertex
	{
		Vertex() {};
		Vertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {};

		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {};

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};

	struct MeshData
	{
		std::vector<MyVertex> Vertices;
		std::vector<uint32_t> Indices32;

		std::vector<uint16_t>& GetIndices16()
		{
			if (m_Indices16.empty())
			{
				m_Indices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); ++i)
				{
					m_Indices16[i] = static_cast<uint16_t>(Indices32[i]);
				}
			}
		}

		UINT GetWidthNum() const { return widthNum; }
		UINT GetHeightNum() const { return heightNum; }
		void SetWidthNum(UINT num) { widthNum = num; }
		void SetHeightNum(UINT num) { heightNum = num; }
		void Clear()
		{
			Vertices.clear();
			Indices32.clear();
			m_Indices16.clear();
			widthNum = 0;
			heightNum = 0;
		}

	private :
		std::vector<uint16_t> m_Indices16;
		UINT widthNum = 0;
		UINT heightNum = 0;
	};
}