#pragma once
#include <ranges>

#include "Mesh.h"
#include "../../Utils/Transform.h"
#include "abstraction/Camera.h"
#include "abstraction/Effect.h"
#include "abstraction/Texture.h"

// Quad mesh
const SpriteVertex vertices[6]
{
	{{-0.5f,-0.5f,0},{0,1}},
	{{+0.5f,+0.5f,0},{1,0}},
	{{+0.5f,-0.5f,0},{1,1}},
	{{-0.5f,-0.5f,0},{0,1}},
	{{-0.5f,+0.5f,0},{0,0}},
	{{+0.5f,+0.5f,0},{1,0}},
};

struct Billboard
{
	XMVECTOR pos;
	XMVECTOR color;
	XMVECTOR size;
	const Material* materials;
	XMVECTOR m_surfaceNormal;
};


// This is an instanced renderer for billboards
class BillboardRenderer
{
	struct BillboardInstance
	{
		XMVECTOR worldPosition;
		XMVECTOR surfaceNormal;
		XMVECTOR size;
		XMVECTOR color;
		XMVECTOR uv; // uv offset {start.xy, size.xy}
		float texId;
	};

	struct CameraConstantBufferParams
	{
		XMMATRIX viewMatrix;
		XMMATRIX projMatrix;
		XMVECTOR position;
	} m_cameraParams ;

	struct BillboardConstantBuffer
	{
		XMMATRIX modelMatrix;
	};

	std::vector<BillboardInstance> m_billboards;
	Effect m_effect;
	size_t m_instanceCount;
	InputLayout m_layout;




	mutable ID3D11Buffer* m_instancedBuffer = nullptr;
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11BlendState* m_blendState = nullptr;

	d3d11_graphics::RenderingContext m_renderContext;

	std::unordered_map<const Material*, int> m_materialMap;
	std::array<const Material*, 8> m_materials = { nullptr,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr };

public:

	void recompile() {
		m_effect.recompile();
	}
	void clear() 
	{
		m_billboards.clear();
		m_instanceCount = 0;
		DX_RELEASE(m_instancedBuffer);
	}
	void addBillboard(XMVECTOR worldPos, float size, const Material& mat, XMVECTOR normal, XMVECTOR color = {1,1,1,1})
	{
		addBillboard(worldPos, { size,size,size, size }, mat, normal, color);
	}
	void addBillboard(XMVECTOR worldPos, XMVECTOR size, const Material& mat, XMVECTOR normal, XMVECTOR color = {1,1,1,1})
	{

		const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		if (m_materialMap.size() >= 7)	m_materialMap.clear();
		if (!m_materialMap.contains(&mat))
		{
			m_materialMap[&mat] = static_cast<int>(m_materialMap.size());
			m_materials[m_materialMap.size() - 1] = &mat;
		}

		float quadTexId = static_cast<float>(m_materialMap[&mat]);

		BillboardInstance bbInstance;

		bbInstance.texId = quadTexId;
		bbInstance.color = color;
		bbInstance.size = size;
		bbInstance.uv = XMVECTOR{ 0,0,1,1 };
		bbInstance.worldPosition = worldPos;
		bbInstance.surfaceNormal = normal;

		m_billboards.push_back(bbInstance);
		m_instanceCount = m_billboards.size();
	}

	void addBillboard(const Billboard& billboard)
	{
		addBillboard(billboard.pos, billboard.size, *billboard.materials, billboard.m_surfaceNormal, billboard.color);
	}
	void addImpostor(const Billboard& billboard, const XMVECTOR& camPos)
	{
		addImpostor(camPos, billboard.size, billboard.pos, *billboard.materials, billboard.color);
	}

	void addImpostor(const XMVECTOR& camPos, float size, XMVECTOR worldPos, const Material& mat, XMVECTOR color = { 1,1,1,1 })
	{
		addImpostor(worldPos, { size,size,size, size }, worldPos,  mat, color);
	}
	void addImpostor(const XMVECTOR& camPos, XMVECTOR size, XMVECTOR worldPos, const Material& mat, XMVECTOR color = {1,1,1,1})
	{
		if (m_materialMap.size() >= 7)m_materialMap.clear();
		if (!m_materialMap.contains(&mat))
		{
			m_materialMap[&mat] = static_cast<int>(m_materialMap.size());
			m_materials[m_materialMap.size() - 1] = &mat;

		}

		float quadTexId = static_cast<float>(m_materialMap[&mat]);

		BillboardInstance bbInstance;

		bbInstance.texId = quadTexId;
		bbInstance.color = color;
		bbInstance.size = size;
		bbInstance.uv = XMVECTOR{ 0,0,1,1 };
		bbInstance.worldPosition = worldPos;
		bbInstance.surfaceNormal = XMVector3Normalize(worldPos-camPos);

		m_billboards.push_back(bbInstance);
		m_instanceCount = m_billboards.size();
			

	}

	void update(const Camera& cam)
	{
		// todo update effect camera cbuffer
		m_cameraParams.position = cam.getPosition();
		m_cameraParams.projMatrix = cam.getProjMatrix();
		m_cameraParams.viewMatrix = cam.getViewMatrix();
		m_effect.updateConstantBuffer(m_cameraParams, "CameraParams");
	}
	void render(const Camera& cam) const 
	{
		resizeBuffers();
		bind();
		bindTextures();
		m_renderContext.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_renderContext.context->IASetInputLayout(m_effect.getVertexLayout());
		m_renderContext.context->OMSetBlendState(m_blendState, nullptr, 0xffffffff);
		m_effect.apply();
		m_renderContext.context->DrawInstanced(6, static_cast<UINT>(m_instanceCount), 0, 0);
		m_effect.unbindResources();
	}

	Effect& getEffect() noexcept { return m_effect; }
	const std::unordered_map<const Material*, int>& getMaterials() const noexcept { return m_materialMap; }
private:
	void bind() const
	{
		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = sizeof(SpriteVertex);
		strides[1] = sizeof(BillboardInstance);

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = m_vertexBuffer;
		bufferPointers[1] = m_instancedBuffer;

		m_renderContext.context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	}

	void resizeBuffers() const 
	{
		DX_RELEASE(m_instancedBuffer);
		D3D11_BUFFER_DESC desc_instance{};
		D3D11_SUBRESOURCE_DATA initInstance{};
		// -- Vertex buffer

		desc_instance.Usage = D3D11_USAGE_DEFAULT;
		desc_instance.ByteWidth = static_cast<UINT>(m_billboards.size() ) * sizeof(BillboardInstance);
		desc_instance.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc_instance.CPUAccessFlags = 0;
		desc_instance.StructureByteStride = 0;
		desc_instance.MiscFlags = 0;

		initInstance.pSysMem = m_billboards.data();
		m_renderContext.device->CreateBuffer(&desc_instance, &initInstance, &m_instancedBuffer);
		
	}


	void bindTextures() const 
	{
		// Default behaviour : get the albedos
		std::vector<const Texture*> albedos(m_materials.max_size());
		std::vector<const Texture*> normals(m_materials.max_size());
		std::vector<const Texture*> aos(m_materials.max_size());
		std::ranges::transform(m_materials, albedos.begin(), [&](const Material* mat) {return mat->queryTexture<TextureType::ALBEDO>(); });
		std::ranges::transform(m_materials, normals.begin(), [&](const Material* mat) {return mat->queryTexture<TextureType::NORMAL>(); });
		std::ranges::transform(m_materials, aos.begin(), [&](const Material* mat) {return mat->queryTexture<TextureType::AO>(); });

		m_effect.bindTextureArray("tex_slots", albedos);
		m_effect.bindTextureArray("normal_slots", normals);
		m_effect.bindTextureArray("ao_slots", aos);
	}


public:
	BillboardRenderer(const std::filesystem::path& fx_path = "res/effects/billboards.fx")
	{
		m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();
		D3D11_BUFFER_DESC descriptor{};
		D3D11_SUBRESOURCE_DATA m_initData{};
		// -- Vertex buffer

		descriptor.Usage = D3D11_USAGE_IMMUTABLE;
		descriptor.ByteWidth = static_cast<UINT>(6) * sizeof(SpriteVertex);
		descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		descriptor.CPUAccessFlags = 0;
		m_initData.pSysMem = &vertices[0];
		m_renderContext.device->CreateBuffer(&descriptor, &m_initData, &m_vertexBuffer);

		/////


		m_layout.pushBack<3>(InputLayout::Semantic::Position);
		m_layout.pushBack<2>(InputLayout::Semantic::Texcoord);

		m_layout.pushBackInstanced<3>("INSTANCE_POS");	// instance pos
		m_layout.pushBackInstanced<3>("INSTANCE_NORMAL");	// instance pos
		m_layout.pushBackInstanced<2>("INSTANCE_SIZE");	// instance size
		m_layout.pushBackInstanced<4>("INSTANCE_COLOR");// instance col
		m_layout.pushBackInstanced<4>("INSTANCE_UV");// instance uv offset
		m_layout.pushBackInstanced<1>("INSTANCE_TEXID");	// instance texid


		m_effect.loadEffectFromFile(fx_path);
		m_effect.bindInputLayout(m_layout);

		m_effect.addNewCBuffer("CameraParams", sizeof(CameraConstantBufferParams));
		m_effect.addNewCBuffer("BillboardParams", sizeof(BillboardConstantBuffer));

		/////////////////////////////////////////////////////////////////
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		m_renderContext.device->CreateBlendState(&blendDesc, &m_blendState);
	}
	~BillboardRenderer()
	{
		DX_RELEASE(m_instancedBuffer);
		DX_RELEASE(m_vertexBuffer);
		DX_RELEASE(m_blendState);
	}

};