#pragma once

#include "BsMesh.h"
#include "BsMeshData.h"
#include "BsResources.h"
#include "BsVertexData.h"
#include "BsVertexDataDesc.h"
#include "BsTexture.h"
#include "BsCoreObject.h"
#include "BsCoreObjectCore.h"
#include "BsImporter.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT Sprite2d : public CoreObject
	{
		private:
			HMesh mesh;
			HTexture texture;

			SPtr<MeshData> meshData;
			SPtr<VertexData> vertexData;
			SPtr<VertexDataDesc> vertexDDesc;

			Vector3 spriteVertex[4];
		public:
			Sprite2d();
			~Sprite2d();

			void addTexture();
			HTexture getTexture();
			
	};
}
