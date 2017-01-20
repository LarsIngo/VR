#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include <glm/glm.hpp>
#include <openvr.h>
#include <string>

class VRDevice
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        VRDevice(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~VRDevice();

        // IsActive.
        bool IsActive();

        // Shutdown.
        void Shutdown();

        // Update.
        void Update();

        // Get render size.
        std::uint32_t GetRenderWidth();
        std::uint32_t GetRenderHeight();

        // VR left eye texture.
        ID3D11Texture2D* mHmdLeftTex;
        // VR right eye texture.
        ID3D11Texture2D* mHmdRightTex;

        // VR left eye shader resource.
        ID3D11ShaderResourceView* mHmdLeftSRV;
        // VR right eye shader resource.
        ID3D11ShaderResourceView* mHmdRightSRV;

        // VR left eye render target.
        ID3D11RenderTargetView* mHmdLeftRTV;
        // VR right eye render target.
        ID3D11RenderTargetView* mHmdRightRTV;

    private:
        // Init HMD(VR).
        bool InitHMD();

        // Init D3D resources.
        void InitD3D();

        // Covert to glm matrix.
        glm::mat4 ConvertMatrix(const vr::HmdMatrix34_t& matPose);

        // D3D
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        // OpenVR
        vr::IVRSystem* mpHMD;
        vr::IVRRenderModels* mpRenderModels;
        vr::TrackedDevicePose_t mTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
        int mValidPoseCount;
        std::string mStrPoseClasses;
        glm::mat4 mDeviceTransforms[vr::k_unMaxTrackedDeviceCount];
        std::string m_strPoseClasses;                            // what classes we saw poses for this frame
        char mDeviceClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
    
        glm::mat4 mHMDTransform;
        glm::mat4 mEyeTransformLeft;
        glm::mat4 mEyeTransformRight;

        glm::mat4 mProjectionLeft;
        glm::mat4 mProjectionRight;

        glm::mat4 mMVPLeft;
        glm::mat4 mMVPRight;

        std::uint32_t mRenderWidth;
        std::uint32_t mRenderHeight;
};
