#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <glm/glm.hpp>
#include <openvr.h>
#include <string>

class FrameBuffer;

class VRDevice
{
    public:
        // Constructor.
        VRDevice();

        // Destructor.
        ~VRDevice();

        // Init D3D resources.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        void InitD3D(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // IsActive.
        bool IsActive();

        // Shutdown.
        void Shutdown();

        // Update.
        void Update();

        // Get render size.
        std::uint32_t GetRenderWidth();
        std::uint32_t GetRenderHeight();

        FrameBuffer* mLeftEyeFB;
        FrameBuffer* mRightEyeFB;

        glm::vec3 mPosition;
		glm::vec3 mRightDir;
		glm::vec3 mUpDir;
		glm::vec3 mFrontDir;

        glm::mat4 mHMDTransform;

        glm::mat4 mEyePosLeft;
        glm::mat4 mEyePosRight;

        glm::mat4 mProjectionLeft;
        glm::mat4 mProjectionRight;

        glm::mat4 mMVPLeft;
        glm::mat4 mMVPRight;

    private:
        // Init HMD(VR).
        bool InitHMD();

        // Covert to glm matrix.
        glm::mat4 ConvertMatrix(const vr::HmdMatrix34_t& matPose);
        glm::mat4 ConvertMatrix(const vr::HmdMatrix44_t& matPose);

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

        std::uint32_t mRenderWidth;
        std::uint32_t mRenderHeight;
};
