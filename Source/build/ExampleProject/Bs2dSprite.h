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
#include "BsRendererUtility.h"
#include "BsBuiltinResources.h"
#include "BsMaterial.h"
#include "BsPass.h"

namespace BansheeEngine
{
	class Sprite2d : public CoreObject
	{
		private:
			HMesh mesh;
			SPtr<MeshCore> meshCore;
			HTexture texture;

			SPtr<MeshData> meshData;
			SPtr<VertexData> vertexData;
			SPtr<VertexDataDesc> vertexDDesc;

			std::vector<Vector3> spriteVertex{ std::vector<Vector3>{Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0)} };

			RenderAPICore& renderApi = RenderAPICore::instance();
			SPtr<CameraCore> mCamera;
			SPtr<CoreRenderer> activeRenderer;
			SPtr<RenderTargetCore> mTarget;
			HMaterial material;
			SPtr<MaterialCore> materialCore;
			SPtr<PassCore> passCore;
			SPtr<GpuParamsSetCore> mParam;
			BuiltinResources builtInRes;
			BuiltinShader builtInShader;
			

		public:
			Sprite2d(const SPtr<CameraCore>&);
			~Sprite2d();

			void addTexture();
			HTexture getTexture();
			
			void addtarget(SPtr<RenderTargetCore>);
			void setUp();
	};
}
