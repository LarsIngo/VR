#include "Material.hpp"
#include "DxHelp.hpp"
#include "FrameBuffer.hpp"
#include "Mesh.hpp"
#include "Skybox.hpp"
#include "StorageBuffer.hpp"
#include "Texture2D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Material::Material(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mVS = nullptr;
    mGS = nullptr;
    mPS = nullptr;
    mGSMetaBuff = nullptr;
    mPSMetaBuff = nullptr;
}

Material::~Material()
{
    if (mVS != nullptr) mVS->Release();
    if (mGS != nullptr) mGS->Release();
    if (mPS != nullptr) mPS->Release();
    if (mGSMetaBuff != nullptr) mGSMetaBuff->Release();
    if (mPSMetaBuff != nullptr) mPSMetaBuff->Release();
}

void Material::Init(const char* VSPath, const char* GSPath, const char* PSPath)
{
    DxHelp::CreateVS(mpDevice, VSPath, "main", &mVS);
    DxHelp::CreateGS(mpDevice, GSPath, "main", &mGS);
    DxHelp::CreatePS(mpDevice, PSPath, "main", &mPS);
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<GSMeta>(mpDevice, 1, &mGSMetaBuff);
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<PSMeta>(mpDevice, 1, &mPSMetaBuff);
}

void Material::Render(Scene& scene, const glm::vec3& cameraPosition, const glm::mat4& vpMatrix, FrameBuffer* targetFb)
{
    ID3D11RenderTargetView* rtvList[4] = { targetFb->mColRTV, targetFb->mWorldRTV, targetFb->mNormRTV, targetFb->mDepthRTV };
    mpDeviceContext->OMSetRenderTargets(4, rtvList, targetFb->mDepthStencilDSV);

    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mpDeviceContext->GSSetShaderResources(0, 1, &mGSMetaBuff);
    mpDeviceContext->VSSetShader(mVS, nullptr, 0);
    mpDeviceContext->GSSetShader(mGS, nullptr, 0);
    mpDeviceContext->PSSetShader(mPS, nullptr, 0);
    D3D11_VIEWPORT vp;
    vp.Width = (float)targetFb->mWidth;
    vp.Height = (float)targetFb->mHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    mpDeviceContext->RSSetViewports(1, &vp);

	// Skybox.
	assert(scene.mpSkybox != nullptr);
	mpDeviceContext->PSSetShaderResources(8, 1, &scene.mpSkybox->mCubeMapSRV);

    // PS meta.
	mPSMeta.cameraPostion = cameraPosition;
	mPSMeta.skyboxMipLevels = scene.mpSkybox->mMipLevels;
    DxHelp::WriteStructuredBuffer<PSMeta>(mpDeviceContext, &mPSMeta, 1, mPSMetaBuff);
    mpDeviceContext->PSSetShaderResources(7, 1, &mPSMetaBuff);

    glm::mat4 modelMatrix;
    for (std::size_t i = 0; i < scene.mEntityList.size(); ++i)
    {
        Entity& entity = scene.mEntityList[i];
		if (!entity.mTransparent)
		{
            modelMatrix = glm::translate(glm::mat4(), entity.mPosition);
			mGSMeta.modelMatrix = glm::transpose(modelMatrix);
			mGSMeta.mvpMatrix = glm::transpose(vpMatrix * modelMatrix);
			DxHelp::WriteStructuredBuffer<Material::GSMeta>(mpDeviceContext, &mGSMeta, 1, mGSMetaBuff);

			mpDeviceContext->PSSetShaderResources(0, 1, &entity.mpAlbedoTex->mSRV);
			mpDeviceContext->PSSetShaderResources(1, 1, &entity.mpNormalTex->mSRV);
			mpDeviceContext->PSSetShaderResources(2, 1, &entity.mpGlossTex->mSRV);
			mpDeviceContext->PSSetShaderResources(3, 1, &entity.mpMetalTex->mSRV);

			Mesh* mesh = entity.mpMesh;
            mpDeviceContext->VSSetShaderResources(0, 1, &mesh->mIndexBuffer->mSRV);
            mpDeviceContext->VSSetShaderResources(1, 1, &mesh->mPositionBuffer->mSRV);
            mpDeviceContext->VSSetShaderResources(2, 1, &mesh->mUVBuffer->mSRV);
            mpDeviceContext->VSSetShaderResources(3, 1, &mesh->mNormalBuffer->mSRV);
            mpDeviceContext->VSSetShaderResources(4, 1, &mesh->mTangentBuffer->mSRV);

			mpDeviceContext->Draw(mesh->mNumIndices, 0);
		}
    }

    void* p[4] = { NULL, NULL, NULL, NULL };
    mpDeviceContext->OMSetRenderTargets(4, (ID3D11RenderTargetView**)p, *(ID3D11DepthStencilView**)p);

    mpDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->PSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(2, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(3, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(7, 1, (ID3D11ShaderResourceView**)p);
	mpDeviceContext->PSSetShaderResources(8, 1, (ID3D11ShaderResourceView**)p);

    mpDeviceContext->VSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(2, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(3, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(4, 1, (ID3D11ShaderResourceView**)p);

    mpDeviceContext->VSSetShader(NULL, nullptr, 0);
    mpDeviceContext->GSSetShader(NULL, nullptr, 0);
    mpDeviceContext->PSSetShader(NULL, nullptr, 0);
}
