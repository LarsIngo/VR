#include "Material.hpp"
#include "RenderSystem.hpp"
#include "Skybox.hpp"

RenderSystem::RenderSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mStandardMaterial = new Material(mpDevice, mpDeviceContext);
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        mStandardMaterial->Init(inputDesc, "resources/shaders/Standard_VS.hlsl", "resources/shaders/Standard_GS.hlsl", "resources/shaders/Standard_PS.hlsl");
    }
}

RenderSystem::~RenderSystem()
{
    delete mStandardMaterial;
}

void RenderSystem::Render(Scene& scene, Camera& camera)
{
    // Skybox.
    scene.mpSkybox->Render(camera.mOrientationMatrix, camera.mProjectionMatrix, camera.mpFrameBuffer);

    // Scene
    mStandardMaterial->Render(scene, camera.mPosition, camera.mViewMatrix, camera.mProjectionMatrix, camera.mpFrameBuffer);
}

void RenderSystem::Render(Scene& scene, VRDevice& hmd)
{
    {   // Render left eye.
        // Skybox.
        scene.mpSkybox->Render(hmd.mOrientationMatrix, hmd.mLeftProjection, hmd.mpLeftFrameBuffer);

        // Scene.
        //Material* material = scene.mStandardMaterial;
        //material->mGSMeta.mvpMatrix = hmd.mMVPLeft;
        //RenderScene(scene, material, hmd.mLeftEyeFB);
		// TODO fix for each eye hmd.mPosition
        mStandardMaterial->Render(scene, hmd.mPosition, hmd.mLeftView, hmd.mLeftProjection, hmd.mpLeftFrameBuffer);
    }
    {   // Render right eye.
        // Skybox.
        scene.mpSkybox->Render(hmd.mRightView, hmd.mRightProjection, hmd.mpRightFrameBuffer);

        // Scene.
        mStandardMaterial->Render(scene, hmd.mPosition, hmd.mOrientationMatrix, hmd.mRightProjection, hmd.mpRightFrameBuffer);
    }
}
