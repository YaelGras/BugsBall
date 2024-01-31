#pragma once
#include <string>

#include "../Platform/WindowsEngine.h"


struct ID3D11ComputeShader;
struct ID3D11UnorderedAccessView;

class ComputeShader
{
private:

	d3d11_graphics::RenderingContext m_context;
	ID3D11ComputeShader* m_computeShader = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;

public:

	ComputeShader() = default;
	ComputeShader(const std::string& path);
	~ComputeShader();

	void bind(const std::vector<ID3D11UnorderedAccessView*>& uav) const;
	void dispatch(int x, int y, int z) const;
	void unbind() const;



};
