#include "Bs2dSprite.h"

namespace BansheeEngine
{
	Sprite2d::Sprite2d(const SPtr<CameraCore>& camera, SPtr<RenderTargetCore> target)
	{
		
		builtInRes = BuiltinResources::instance();
		builtInShader = BuiltinShader::ImageAlpha;

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

		//UINT8*(spriteVertex);
		meshData->setVertexData(VES_POSITION, (UINT8*)spriteVertex.data(), 3 * sizeof(spriteVertex));

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

		//create the mesh
		mesh = Mesh::create(meshData);

		//create the mesh core
		meshCore = mesh->getCore();

		//set texture (this will be moved to addTexture method)
		texture = gImporter().import<Texture>("photo.jpg");

		float invHeight = 1.0 / target->getProperties().getHeight();
		float invWidth = 1.0 / target->getProperties().getWidth();

		material = Material::create();
		material->setShader(builtInRes.getBuiltinShader(builtInShader));
		material->setFloat("invViewportWidth", invWidth);
		material->setFloat("invViewportHeight", invHeight);
		material->setMat4("worldTransform", Matrix4::IDENTITY);
		material->setTexture("mainTexture", texture);
		material->setColor("tint", Color::White);


		mSpriteCore.store(bs_new<SpriteRenderer>(), std::memory_order_release);
		gCoreAccessor().queueCommand(std::bind(&Sprite2d::startSpriteRenderer, this, material->getCore(), camera, target, meshCore));
	}

	Sprite2d::~Sprite2d()
	{
		gCoreAccessor().queueCommand(std::bind(&Sprite2d::deleteSpriteRenderer, this, mSpriteCore.load(std::memory_order_relaxed)));
	}

	void Sprite2d::addTexture()
	{

	}

	HTexture Sprite2d::getTexture()
	{
		return texture;
	}

	void Sprite2d::startSpriteRenderer(const SPtr<MaterialCore>& initMaterial, const SPtr<CameraCore>& camera, SPtr<RenderTargetCore> target, SPtr<MeshCore> meshcore)
	{
		mSpriteCore.load(std::memory_order_acquire)->setUp(initMaterial, camera, target, meshcore);
	}

	void Sprite2d::deleteSpriteRenderer(SpriteRenderer* core)
	{
		bs_delete(core);
	}

	//Sprite Renderer class

	SpriteRenderer::SpriteRenderer()
	{

	}

	SpriteRenderer::~SpriteRenderer()
	{
		if (mCamera != nullptr)
		{
			activeRenderer->unregisterRenderCallback(mCamera.get(), 50);
		}
		
	}

	void SpriteRenderer::setUp(const SPtr<MaterialCore>& material, const SPtr<CameraCore>& camera, SPtr<RenderTargetCore> target, SPtr<MeshCore> meshCore)
	{
		mTarget = target;
		mCamera = camera;
		mMeshCore = meshCore;
		materialCore = material;
		mParam = materialCore->createParamsSet();
		passCore = materialCore->getPass();

		renderApi.setRenderTarget(mTarget);

		//set rendering callback
		activeRenderer = RendererManager::instance().getActive();
		if(mCamera != nullptr)
			activeRenderer->registerRenderCallback(mCamera.get(), 50, std::bind(&SpriteRenderer::setUp, this), true);
	}

	void SpriteRenderer::render()
	{


		renderApi.setGraphicsPipeline(passCore->getPipelineState());

		gRendererUtility().setPass(materialCore);
		gRendererUtility().setPassParams(mParam);
		gRendererUtility().draw(mMeshCore, mMeshCore->getProperties().getSubMesh(0));
	}

}
