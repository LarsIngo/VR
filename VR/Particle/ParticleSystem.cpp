#include "ParticleSystem.hpp"

#include "ParticleEmitter.hpp"
#include "../DxHelp.hpp"
#include "../DoubleFrameBuffer.hpp"
#include "../FrameBuffer.hpp"
#include "../Mesh.hpp"
#include "../StorageSwapBuffer.hpp"

ParticleSystem::ParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    // Create meta buffers.
    mEmittMetaDataBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(EmittMetaData) * 1, sizeof(EmittMetaData));
    mUpdateMetaDataBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(UpdateMetaData) * 1, sizeof(UpdateMetaData));
    mRenderMetaDataBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(RenderMetaData) * 1, sizeof(RenderMetaData));

    // Create render pipeline.
    {
        DxHelp::CreateVS(mpDevice, "../resources/shaders/Particles_Render_VS.hlsl", "main", &mVertexShader);
        DxHelp::CreateGS(mpDevice, "../resources/shaders/Particles_Render_GS.hlsl", "main", &mGeometryShader);
        DxHelp::CreatePS(mpDevice, "../resources/shaders/Particles_Render_PS.hlsl", "main", &mPixelShader);

        {   // Create blend state.
            D3D11_BLEND_DESC blendDesc;
            ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
            blendDesc.AlphaToCoverageEnable = false;
            blendDesc.IndependentBlendEnable = false;

            blendDesc.RenderTarget[0].BlendEnable = true;
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            DxAssert(mpDevice->CreateBlendState(&blendDesc, &mBlendState), S_OK);
        }

        {   // Create depth stencil state.
            D3D11_DEPTH_STENCIL_DESC dssDesc;
            ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
            dssDesc.DepthEnable = true;
            dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            dssDesc.DepthFunc = D3D11_COMPARISON_LESS;
            dssDesc.StencilEnable = true;
            dssDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
            dssDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            dssDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            dssDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            dssDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            dssDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            dssDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

            DxAssert(mpDevice->CreateDepthStencilState(&dssDesc, &mDepthStencilState), S_OK);
        }
    }

    // Create emitter pipeline.
    DxHelp::CreateCS(mpDevice, "../resources/shaders/Particles_Emitter_CS.hlsl", "main", &mEmittCS);

    // Create update pipeline.
    DxHelp::CreateCS(mpDevice, "../resources/shaders/Particles_Update_CS.hlsl", "main", &mUpdateCS);
}

ParticleSystem::~ParticleSystem()
{
    delete mEmittMetaDataBuffer;
    delete mUpdateMetaDataBuffer;
    delete mRenderMetaDataBuffer;

    mEmittCS->Release();
    mUpdateCS->Release();

    mVertexShader->Release();
    mGeometryShader->Release();
    mPixelShader->Release();
    mBlendState->Release();
    mDepthStencilState->Release();
}

void ParticleSystem::Update(Scene& scene, float dt)
{
    // Update emitters.
    mpDeviceContext->CSSetShader(mEmittCS, NULL, NULL);

    for (Entity& entity : scene.mEntityList)
    {
        ParticleEmitter* emitter = entity.mpParticleEmitter;
        if (emitter == nullptr) continue;

        emitter->mTime += dt;
        unsigned int emittCount = emitter->mTime * emitter->mEmittFrequency;

        if (!emittCount) continue;

        emitter->mTime -= (float)emittCount / emitter->mEmittFrequency;

        mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &emitter->mPositionBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(1, 1, &emitter->mScaleBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(2, 1, &emitter->mVelocityBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(3, 1, &emitter->mLifetimeBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(4, 1, &emitter->mColorBuffer->GetOutputBuffer()->mUAV, NULL);

        if (entity.mpMesh != nullptr)
            mpDeviceContext->CSSetShaderResources(1, 1, &entity.mpMesh->mPositionBuffer->mSRV);

        for (unsigned int i = 0; i < emittCount; ++i)
        {
            mEmittMetaData.position = entity.mPosition;
            mEmittMetaData.velocity = emitter->mVelocity;
            mEmittMetaData.scale = emitter->mScale;
            mEmittMetaData.lifetime = emitter->mLifetime;
            mEmittMetaData.color = emitter->mColor;
            mEmittMetaData.emittIndex = emitter->mEmittIndex;
            mEmittMetaData.emittPointIndex = -1;
            if (entity.mpMesh != nullptr)
            {
                std::uniform_int_distribution<int> distribution(0, entity.mpMesh->mNumVertices - 1);
                mEmittMetaData.emittPointIndex = distribution(mRandomEngine);
            }

            mEmittMetaDataBuffer->Write(&mEmittMetaData, sizeof(EmittMetaData), 0);

            mpDeviceContext->CSSetShaderResources(0, 1, &mEmittMetaDataBuffer->mSRV);

            mpDeviceContext->Dispatch(1, 1, 1);

            emitter->mEmittIndex = (emitter->mEmittIndex + 1) % emitter->mParticleCount;

            void* p[1] = { NULL };
            mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
        }

        void* p[1] = { NULL };
        if (entity.mpMesh != nullptr)
            mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);

        mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(1, 1, (ID3D11UnorderedAccessView**)p, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(2, 1, (ID3D11UnorderedAccessView**)p, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(3, 1, (ID3D11UnorderedAccessView**)p, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(4, 1, (ID3D11UnorderedAccessView**)p, NULL);

        emitter->mPositionBuffer->Swap();
        emitter->mScaleBuffer->Swap();
        emitter->mVelocityBuffer->Swap();
        emitter->mLifetimeBuffer->Swap();
        emitter->mColorBuffer->Swap();
    }

    mpDeviceContext->CSSetShader(NULL, NULL, NULL);

    // Update particles.
    mpDeviceContext->CSSetShader(mUpdateCS, NULL, NULL);

    for (Entity& entity : scene.mEntityList)
    {
        ParticleEmitter* emitter = entity.mpParticleEmitter;
        if (emitter == nullptr) continue;

        mUpdateMetaData.dt = dt;
        mUpdateMetaData.particleCount = emitter->mParticleCount;
        mUpdateMetaDataBuffer->Write(&mUpdateMetaData, sizeof(UpdateMetaData), 0);

        mpDeviceContext->CSSetShaderResources(0, 1, &mUpdateMetaDataBuffer->mSRV);
        mpDeviceContext->CSSetShaderResources(1, 1, &emitter->mPositionBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->CSSetShaderResources(2, 1, &emitter->mScaleBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->CSSetShaderResources(3, 1, &emitter->mVelocityBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->CSSetShaderResources(4, 1, &emitter->mLifetimeBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->CSSetShaderResources(5, 1, &emitter->mColorBuffer->GetInputBuffer()->mSRV);

        mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &emitter->mPositionBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(1, 1, &emitter->mScaleBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(2, 1, &emitter->mVelocityBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(3, 1, &emitter->mLifetimeBuffer->GetOutputBuffer()->mUAV, NULL);
        mpDeviceContext->CSSetUnorderedAccessViews(4, 1, &emitter->mColorBuffer->GetOutputBuffer()->mUAV, NULL);

        mpDeviceContext->Dispatch(static_cast<unsigned int>(ceil(emitter->mParticleCount / 128.f)), 1, 1);

        emitter->mPositionBuffer->Swap();
        emitter->mScaleBuffer->Swap();
        emitter->mVelocityBuffer->Swap();
        emitter->mLifetimeBuffer->Swap();
        emitter->mColorBuffer->Swap();
    }

    mpDeviceContext->CSSetShader(NULL, NULL, NULL);

    void* p[1] = { NULL };
    mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(2, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(3, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(4, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(5, 1, (ID3D11ShaderResourceView**)p);

    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
    mpDeviceContext->CSSetUnorderedAccessViews(1, 1, (ID3D11UnorderedAccessView**)p, NULL);
    mpDeviceContext->CSSetUnorderedAccessViews(2, 1, (ID3D11UnorderedAccessView**)p, NULL);
    mpDeviceContext->CSSetUnorderedAccessViews(3, 1, (ID3D11UnorderedAccessView**)p, NULL);
    mpDeviceContext->CSSetUnorderedAccessViews(4, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleSystem::Render(Scene& scene, Camera& camera)
{
    mpDeviceContext->VSSetShader(mVertexShader, NULL, NULL);
    mpDeviceContext->GSSetShader(mGeometryShader, NULL, NULL);
    mpDeviceContext->PSSetShader(mPixelShader, NULL, NULL);

    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    float blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
    UINT sampleMask = 0xffffffff;
    mpDeviceContext->OMSetBlendState(mBlendState, blendFactor, sampleMask);
    mpDeviceContext->OMSetRenderTargets(1, &camera.mpFrameBuffer->GetFrameBuffer()->mColRTV, camera.mpFrameBuffer->GetFrameBuffer()->mDepthStencilDSV);
    mpDeviceContext->OMSetDepthStencilState(mDepthStencilState, NULL);

    for (Entity& entity : scene.mEntityList)
    {
        ParticleEmitter* emitter = entity.mpParticleEmitter;
        if (emitter == nullptr) continue;

        mRenderMetaData.vpMatrix = glm::transpose(camera.mProjectionMatrix * camera.mViewMatrix);
        mRenderMetaData.lensPosition = camera.mPosition;
        mRenderMetaData.lensUpDirection = camera.mUpDirection;
        mRenderMetaDataBuffer->Write(&mRenderMetaData, sizeof(RenderMetaData), 0);

        mpDeviceContext->GSSetShaderResources(0, 1, &mRenderMetaDataBuffer->mSRV);

        mpDeviceContext->VSSetShaderResources(0, 1, &emitter->mPositionBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->VSSetShaderResources(1, 1, &emitter->mScaleBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->VSSetShaderResources(2, 1, &emitter->mVelocityBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->VSSetShaderResources(3, 1, &emitter->mLifetimeBuffer->GetInputBuffer()->mSRV);
        mpDeviceContext->VSSetShaderResources(4, 1, &emitter->mColorBuffer->GetInputBuffer()->mSRV);

        mpDeviceContext->Draw(emitter->mParticleCount, 0);
    }

    mpDeviceContext->VSSetShader(NULL, NULL, NULL);
    mpDeviceContext->GSSetShader(NULL, NULL, NULL);
    mpDeviceContext->PSSetShader(NULL, NULL, NULL);

    void* p[1] = { NULL };
    mpDeviceContext->OMSetBlendState(NULL, blendFactor, sampleMask);
    mpDeviceContext->OMSetDepthStencilState(NULL, NULL);
    mpDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, NULL);

    mpDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);

    mpDeviceContext->VSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(2, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(3, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShaderResources(4, 1, (ID3D11ShaderResourceView**)p);
}
