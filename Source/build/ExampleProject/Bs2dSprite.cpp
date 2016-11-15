#include "Bs2dSprite.h"

namespace BansheeEngine
{
	Sprite2d::Sprite2d(const SPtr<CameraCore>& camera)
	{
		if(camera != nullptr)
			mCamera = camera;
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
		/*std::vector<UINT8> vector3ToUint8{ std::vector<UINT8>{0,0,0,0} };
		for (int i = 0; i < spriteVertex.size(); i++)
		{
			vector3ToUint8.at(i) = reinterpret_cast<UINT8>(&spriteVertex.at(i));
		}*/

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

		//set rendering callback
		activeRenderer = RendererManager::instance().getActive();
		activeRenderer->registerRenderCallback(mCamera.get(), 40, std::bind(&Sprite2d::setUp, this), true);

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

	void Sprite2d::addtarget(SPtr<RenderTargetCore> target)
	{
		mTarget = target;
		renderApi.setRenderTarget(mTarget);
	}

	void Sprite2d::setUp()
	{
		float invHeight = 1.0 / mTarget->getProperties().getHeight();
		float invWidth = 1.0 / mTarget->getProperties().getWidth();

		material = Material::create();
		material->setShader(builtInRes.getBuiltinShader(builtInShader));
		material->setFloat("invViewportWidth", invWidth);
		material->setFloat("invViewportHeight", invHeight);
		material->setMat4("worldTransform", Matrix4::IDENTITY);
		material->setTexture("mainTexture", texture);
		material->setColor("tint", Color::White);

		materialCore = material->getCore();
		mParam = materialCore->createParamsSet();
		passCore = materialCore->getPass();

		renderApi.setGraphicsPipeline(passCore->getPipelineState());

		gRendererUtility().setPass(materialCore);
		gRendererUtility().setPassParams(mParam);
		gRendererUtility().draw(meshCore, meshCore->getProperties().getSubMesh(0));


	}

	//start of defining the sprite rendering
	SpriteRenderer::SpriteRenderer()
	{

	}

}
