#pragma once

#include "BsMesh.h"
#include "BsMeshData.h"
#include "BsResource.h"
#include "BsVertexData.h"
#include "BsVertexDataDesc.h"
#include "BsTexture.h"

namespace BansheeEngine
{
	class Sprite2d
	{
		private:
			HMesh mesh;
			HTexture texture;

			SPtr<MeshData> meshData;
			SPtr<VertexData> vertexData;
			SPtr<VertexDataDesc> vertexDDesc;

		public:
			Sprite2d();
			~Sprite2d();

			void addTexture();

	};
}
