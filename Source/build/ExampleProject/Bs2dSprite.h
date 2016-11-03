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

#include "BsRenderAPI.h"
#include "BsCoreRenderer.h"
#include "BsRendererManager.h"

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

			RenderAPICore& renderApi = RenderAPICore::instance();
			SPtr<CameraCore> mCamera;
			SPtr<CoreRenderer> activeRenderer;
		public:
			Sprite2d(const SPtr<CameraCore>&);
			~Sprite2d();

			void addTexture();
			HTexture getTexture();
			
			void addtarget(SPtr<RenderTargetCore>);
			void setUp();
	};
}
