#include "Transparent.hpp"
#include "DoubleFrameBuffer.hpp"
#include "DxHelp.hpp"
#include "FrameBuffer.hpp"
#include "Mesh.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Transparent::Transparent(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	mpDevice = pDevice;
	mpDeviceContext = pDeviceContext;
	mInputLayout = nullptr;
	mVS = nullptr;
	mGS = nullptr;
	mPS = nullptr;
	mGSMetaBuff = nullptr;
	mPSMetaBuff = nullptr;
}

Transparent::~Transparent()
{
	if (mInputLayout != nullptr) mInputLayout->Release();
	if (mVS != nullptr) mVS->Release();
	if (mGS != nullptr) mGS->Release();
	if (mPS != nullptr) mPS->Release();
	if (mGSMetaBuff != nullptr) mGSMetaBuff->Release();
	if (mPSMetaBuff != nullptr) mPSMetaBuff->Release();
}

void Transparent::Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const char* VSPath, const char* GSPath, const char* PSPath)
{
	DxHelp::CreateVS(mpDevice, VSPath, "main", &mVS, &inputDesc, &mInputLayout);
	DxHelp::CreateGS(mpDevice, GSPath, "main", &mGS);
	DxHelp::CreatePS(mpDevice, PSPath, "main", &mPS);
	DxHelp::CreateCPUwriteGPUreadStructuredBuffer<GSMeta>(mpDevice, 1, &mGSMetaBuff);
	DxHelp::CreateCPUwriteGPUreadStructuredBuffer<PSMeta>(mpDevice, 1, &mPSMetaBuff);
}

void Transparent::Render(Scene& scene, const glm::vec3& cameraPosition, const glm::mat4& orientationMatix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, unsigned int screenWidth, unsigned int screenHeight, DoubleFrameBuffer* fb)
{
    glm::mat4 vpMatrix = projectionMatrix * viewMatrix;
    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mpDeviceContext->GSSetShaderResources(0, 1, &mGSMetaBuff);
	mpDeviceContext->IASetInputLayout(mInputLayout);
	mpDeviceContext->VSSetShader(mVS, nullptr, 0);
	mpDeviceContext->GSSetShader(mGS, nullptr, 0);
	mpDeviceContext->PSSetShader(mPS, nullptr, 0);
	D3D11_VIEWPORT vp;
	vp.Width = (float)screenWidth;
	vp.Height = (float)screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mpDeviceContext->RSSetViewports(1, &vp);
    FLOAT blendFactor[] = { 1.f, 1.f, 1.f, 1.f };
    //mpDeviceContext->OMSetBlendState(mBlendState, blendFactor, 0xffffffff);

	// Skybox.
	assert(scene.mpSkybox != nullptr);
	mpDeviceContext->PSSetShaderResources(8, 1, &scene.mpSkybox->mCubeMapSRV);

	// PS meta.
	mPSMeta.cameraPostion = cameraPosition;
	mPSMeta.skyboxMipLevels = scene.mpSkybox->mMipLevels;
    mPSMeta.vpMatrix = glm::inverse(vpMatrix); // TODO NOT WORKING 
	mPSMeta.screenWidth = screenWidth;
	mPSMeta.screenHeight = screenHeight;
	DxHelp::WriteStructuredBuffer<PSMeta>(mpDeviceContext, &mPSMeta, 1, mPSMetaBuff);
	mpDeviceContext->PSSetShaderResources(7, 1, &mPSMetaBuff);

	unsigned int stride;
	unsigned int offset;
	glm::mat4 modelMatrix;

    FrameBuffer* sourceFb = fb->GetFrameBuffer();
    fb->Swap();
    FrameBuffer* targetFb = fb->GetFrameBuffer();
    targetFb->Copy(sourceFb);

	for (std::size_t i = 0; i < scene.mEntityList.size(); ++i)
	{
		Entity& entity = scene.mEntityList[i];
		if (entity.mTransparent)
		{
            ID3D11RenderTargetView* rtvList[2] = { targetFb->mColRTV, targetFb->mDepthColRTV };
            mpDeviceContext->OMSetRenderTargets(2, rtvList, targetFb->mDepthStencilDSV);

            modelMatrix = glm::translate(glm::mat4(), entity.mPosition);
			mGSMeta.modelMatrix = glm::transpose(modelMatrix);
			mGSMeta.mvpMatrix = glm::transpose(vpMatrix * modelMatrix);
			DxHelp::WriteStructuredBuffer<Transparent::GSMeta>(mpDeviceContext, &mGSMeta, 1, mGSMetaBuff);

			mpDeviceContext->PSSetShaderResources(0, 1, &entity.mpAlbedoTex->mSRV);
			mpDeviceContext->PSSetShaderResources(1, 1, &entity.mpNormalTex->mSRV);
			mpDeviceContext->PSSetShaderResources(2, 1, &entity.mpGlossTex->mSRV);
			mpDeviceContext->PSSetShaderResources(3, 1, &sourceFb->mColSRV);
            mpDeviceContext->PSSetShaderResources(4, 1, &sourceFb->mDepthColSRV);
			Mesh* mesh = entity.mpMesh;
			stride = sizeof(Transparent::Vertex);
			offset = 0;
			mpDeviceContext->IASetVertexBuffers(0, 1, &mesh->mVertexBuffer, &stride, &offset);
			mpDeviceContext->IASetIndexBuffer(mesh->mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			mpDeviceContext->DrawIndexed(mesh->mNumIndices, 0, 0);

            void* p[2] = { NULL, NULL };
            mpDeviceContext->OMSetRenderTargets(2, (ID3D11RenderTargetView**)p, *(ID3D11DepthStencilView**)p);
            mpDeviceContext->PSSetShaderResources(3, 1, (ID3D11ShaderResourceView**)p);
            mpDeviceContext->PSSetShaderResources(4, 1, (ID3D11ShaderResourceView**)p);

            sourceFb->Copy(targetFb);
		}
	}

	void* p[2] = { NULL, NULL };
	mpDeviceContext->OMSetRenderTargets(2, (ID3D11RenderTargetView**)p, *(ID3D11DepthStencilView**)p);
	mpDeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)p, &stride, &offset);
	mpDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(2, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(3, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->PSSetShaderResources(4, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(7, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(8, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->VSSetShader(NULL, nullptr, 0);
	mpDeviceContext->GSSetShader(NULL, nullptr, 0);
	mpDeviceContext->PSSetShader(NULL, nullptr, 0);
}
