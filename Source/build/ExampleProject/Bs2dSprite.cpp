#include "Bs2dSprite.h"

namespace BansheeEngine
{
	Sprite2d::Sprite2d()
	{
		//create the vertex description for the meshData
		vertexDDesc = VertexDataDesc::create();
		vertexDDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
		vertexDDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		// create the meshData with vertex description
		meshData = MeshData::create(4, 6, vertexDDesc);


	}

	Sprite2d::~Sprite2d()
	{

	}

	void Sprite2d::addTexture()
	{

	}
}
