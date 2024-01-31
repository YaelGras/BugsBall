#pragma once

struct InputLayout
{

private:


	LPCSTR semantics_str[10] = {
		"BINORMAL",
		"BLENDINDICES",
		"BLENDWEIGHT",
		"NORMAL",
		"POSITION",
		"POSITIONT",
		"PSIZE",
		"TANGENT",
		"TEXCOORD",
		"COLOR"
	};

	UINT stride = 0;
	UINT instancedStride = 0;

	const std::vector<DXGI_FORMAT> formats{
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> elems;


public:


	enum class Semantic {
		Binormal,
		BlendIndices,
		BlendWeight,
		Normal,
		Position,
		PositionT,
		PSize,
		Tangent,
		Texcoord,
		Color
	};


	InputLayout() = default;
	InputLayout(const InputLayout& other)
	{
		elems = other.elems;
		stride = other.stride;
		instancedStride = other.instancedStride;
	}

	InputLayout& operator=(const InputLayout& other)
	{
		elems = other.elems;
		stride = other.stride;
		instancedStride = other.instancedStride;
		return *this;
	}

	D3D11_INPUT_ELEMENT_DESC* asInputDesc() {
		return elems.data();
	}
	std::vector < D3D11_INPUT_ELEMENT_DESC> getElements() { return elems; }

	// Works for now, as i'm not sure we will really use anything else than float32(s) formats
	template<size_t floatCount>
	void pushBack(Semantic elemType)
	{
		elems.push_back(
			D3D11_INPUT_ELEMENT_DESC{
				semantics_str[size_t(elemType)], 0,
				formats[floatCount - 1],
				0, stride,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		);
		stride += 16;
	}

	template<size_t floatCount>
	void pushBack(const char* semanticName)
	{
		elems.push_back(
			D3D11_INPUT_ELEMENT_DESC{
				semanticName, 0,
				formats[floatCount - 1],
				0, stride,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		);
		stride += 16;
	}


	template<size_t floatCount>
	void pushBackInstanced(const char* semanticName)
	{



		elems.push_back(
			D3D11_INPUT_ELEMENT_DESC{
				semanticName,						//     LPCSTR SemanticName;
				0,									//     UINT SemanticIndex;
				formats[floatCount - 1],			//     DXGI_FORMAT Format;
				1,									//     UINT InputSlot;
				instancedStride,					//     UINT AlignedByteOffset;
				D3D11_INPUT_PER_INSTANCE_DATA,		//     D3D11_INPUT_CLASSIFICATION InputSlotClass;
				1									//     UINT InstanceDataStepRate;
			}
		);
		instancedStride += 16;
	}

};