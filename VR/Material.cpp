#include "Material.hpp"
#include "DxHelp.hpp"
#include "FrameBuffer.hpp"
#include "Mesh.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Material::Material(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mInputLayout = nullptr;
    mVS = nullptr;
    mGS = nullptr;
    mPS = nullptr;
    mGSMetaBuff = nullptr;
}

Material::~Material()
{
    if (mInputLayout != nullptr) mInputLayout->Release();
    if (mVS != nullptr) mVS->Release();
    if (mGS != nullptr) mGS->Release();
    if (mPS != nullptr) mPS->Release();
    if (mGSMetaBuff != nullptr) mGSMetaBuff->Release();
}

void Material::Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const char* VSPath, const char* GSPath, const char* PSPath)
{
    DxHelp::CreateVS(mpDevice, VSPath, &mVS, &inputDesc, &mInputLayout);
    DxHelp::CreateGS(mpDevice, GSPath, &mGS);
    DxHelp::CreatePS(mpDevice, PSPath, &mPS);
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<GSMeta>(mpDevice, 1, &mGSMetaBuff);
}

void Material::Render(Scene& scene, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, FrameBuffer* targetFb)
{
    glm::mat4 vpMatix = projectionMatrix * viewMatrix;
    mpDeviceContext->OMSetRenderTargets(1, &targetFb->mColRTV, targetFb->mDepthDSV);
    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mpDeviceContext->GSSetShaderResources(0, 1, &mGSMetaBuff);
    mpDeviceContext->IASetInputLayout(mInputLayout);
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

    unsigned int stride;
    unsigned int offset;
    glm::mat4 modelMatix;
    for (std::size_t i = 0; i < scene.mEntityList.size(); ++i)
    {
        Entity& entity = scene.mEntityList[i];
        modelMatix = glm::translate(glm::mat4(), entity.mPosition);
        mGSMeta.modelMatrix = glm::transpose(modelMatix);
        mGSMeta.mvpMatrix = glm::transpose(vpMatix * modelMatix);
        DxHelp::WriteStructuredBuffer<Material::GSMeta>(mpDeviceContext, &mGSMeta, 1, mGSMetaBuff);

        mpDeviceContext->PSSetShaderResources(0, 1, &entity.mpDiffuseTex->mSRV);
        mpDeviceContext->PSSetShaderResources(1, 1, &entity.mpNormalTex->mSRV);
        Mesh* mesh = entity.mpMesh;
        stride = sizeof(Material::Vertex);
        offset = 0;
        mpDeviceContext->IASetVertexBuffers(0, 1, &mesh->mVertexBuffer, &stride, &offset);
        mpDeviceContext->IASetIndexBuffer(mesh->mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        mpDeviceContext->DrawIndexed(mesh->mNumIndices, 0, 0);
    }

    void* p[1] = { NULL };
    mpDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, nullptr);
    mpDeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)p, &stride, &offset);
    mpDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->PSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShader(NULL, nullptr, 0);
    mpDeviceContext->GSSetShader(NULL, nullptr, 0);
    mpDeviceContext->PSSetShader(NULL, nullptr, 0);
}
