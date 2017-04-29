#include "DoubleFrameBuffer.hpp"
#include "Material.hpp"
#include "RenderSystem.hpp"
#include "Skybox.hpp"
#include "Transparent.hpp"

RenderSystem::RenderSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mStandardMaterial = new Material(mpDevice, mpDeviceContext);
	mTransparentMaterial = new Transparent(mpDevice, mpDeviceContext);
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        mStandardMaterial->Init(inputDesc, "resources/shaders/Standard_VS.hlsl", "resources/shaders/Standard_GS.hlsl", "resources/shaders/Standard_PS.hlsl");
		mTransparentMaterial->Init(inputDesc, "resources/shaders/Standard_VS.hlsl", "resources/shaders/Standard_GS.hlsl", "resources/shaders/Transparent_PS.hlsl");
	}
}

RenderSystem::~RenderSystem()
{
    delete mStandardMaterial;
	delete mTransparentMaterial;
}

void RenderSystem::Render(Scene& scene, Camera& camera)
{
    // Skybox.
    scene.mpSkybox->Render(camera.mOrientationMatrix, camera.mProjectionMatrix, camera.mpFrameBuffer->GetFrameBuffer());

    // Standard.
    mStandardMaterial->Render(scene, camera.mPosition, camera.mProjectionMatrix * camera.mViewMatrix, camera.mpFrameBuffer->GetFrameBuffer());

	// Transparent.
	mTransparentMaterial->Render(scene, camera.mPosition, camera.mOrientationMatrix, camera.mViewMatrix, camera.mProjectionMatrix, camera.mScreenWidth, camera.mScreenHeight, camera.mpFrameBuffer);
}

void RenderSystem::Render(Scene& scene, VRDevice& hmd)
{
    {   // Left eye.
        // Skybox.
        scene.mpSkybox->Render(hmd.mOrientationMatrix, hmd.mLeftProjection, hmd.mpLeftFrameBuffer->GetFrameBuffer());

        // Standard.
        mStandardMaterial->Render(scene, hmd.mPosition, hmd.mLeftProjection * hmd.mLeftView, hmd.mpLeftFrameBuffer->GetFrameBuffer());

    //    // Transparent.
    //    mTransparentMaterial->Render(scene, hmd.mPosition, hmd.mLeftView, hmd.mLeftProjection, hmd.GetWidth(), hmd.GetHeight(), hmd.mpLeftFrameBuffer);
    }
    {   // Right eye.
        // Skybox.
        scene.mpSkybox->Render(hmd.mOrientationMatrix, hmd.mRightProjection, hmd.mpRightFrameBuffer->GetFrameBuffer());

        // Standard.
        mStandardMaterial->Render(scene, hmd.mPosition, hmd.mRightProjection * hmd.mRightView, hmd.mpRightFrameBuffer->GetFrameBuffer());

    //    // Transparent.
    //    mTransparentMaterial->Render(scene, hmd.mPosition, hmd.mRightView, hmd.mRightProjection, hmd.GetWidth(), hmd.GetHeight(), hmd.mpRightFrameBuffer);
    }
}
