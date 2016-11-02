#include "Bs2dSprite.h"

namespace BansheeEngine
{
	Sprite2d::Sprite2d()
	{
		//create the vertex data for the mesh
		spriteVertex[0] = Vector3(0, 0, 0);
		spriteVertex[1] = Vector3(1, 0, 0);
		spriteVertex[2] = Vector3(0, 1, 0);
		spriteVertex[3] = Vector3(1, 1, 0);

		//create the vertex description for the meshData
		vertexDDesc = VertexDataDesc::create();
		vertexDDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
		vertexDDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		// create the meshData with vertex description
		meshData = MeshData::create(4, 6, vertexDDesc);

		//give vertex to the created meshData
		UINT8*(spriteVertex);
		meshData->setVertexData(VES_POSITION, spriteVertex, sizeof(spriteVertex));

		//set the indices
		UINT32* indices = meshData->getIndices32();

		//first triangle
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		//second triangle
		indices[3] = 2;
		indices[4] = 3;
		indices[5] = 1;

		//set texture (this will be moved to addTexture method)
		texture = gImporter().import<Texture>("photo.jpg");

	}

	Sprite2d::~Sprite2d()
	{

	}

	void Sprite2d::addTexture()
	{

	}

	HTexture Sprite2d::getTexture()
	{
		return texture;
	}

}
