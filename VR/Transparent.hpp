#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Scene.hpp"

class DoubleFrameBuffer;

class Transparent
{
public:
	// Vertex
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct GSMeta
	{
		glm::mat4 modelMatrix;
		glm::mat4 mvpMatrix;
	} mGSMeta;

	struct PSMeta
	{
		glm::vec3 cameraPostion;
		unsigned int skyboxMipLevels;
		glm::mat4 projMatrix;
		unsigned int screenWidth;
		unsigned int screenHeight;
		float pad[2];
	} mPSMeta;

	// Constructor.
	// pDevice Pointer to D3D11 device.
	// pDeviceContext Pointer to D3D11 device context.
	Transparent(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

	// Destructor.
	~Transparent();

	// Intialise.
	void Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const char* VSPath, const char* GSPath, const char* PSPath);

	// Render scene.
	// scene Scene to render.
	// cameraPosition Position of the camera in world space.
	// viewMatrix Camera view matrix.
	// projectionMatrix Camera projection matrix.
	// screenWidth Width of screen.
	// screenHeight Height of screen.
	// fb Double frame buffer to render.
	void Render(Scene& scene, const glm::vec3& cameraPosition, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, unsigned int screenWidth, unsigned int screenHeight, DoubleFrameBuffer* fb);

	// D3D11 input layout.
	ID3D11InputLayout* mInputLayout;
	// Vertex shader.
	ID3D11VertexShader* mVS;
	// Geometry shader.
	ID3D11GeometryShader* mGS;
	// Pixel shader.
	ID3D11PixelShader* mPS;
	// GS Meta buffer.
	ID3D11ShaderResourceView* mGSMetaBuff;
	// PS Meta buffer.
	ID3D11ShaderResourceView* mPSMetaBuff;

private:
	ID3D11Device* mpDevice;
	ID3D11DeviceContext* mpDeviceContext;
};
