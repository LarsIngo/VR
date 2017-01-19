#pragma once

#include <glm/glm.hpp>
#include <openvr.h>
#include <string>


class VRDevice
{
    private:
        glm::mat4 ConvertMatrix(const vr::HmdMatrix34_t& matPose);

        vr::IVRSystem* mpHMD;
        vr::IVRRenderModels* mpRenderModels;
        vr::TrackedDevicePose_t* mTrackedDevicePose;
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

    public:
        // Constructor.
        VRDevice();

        // Destructor.
        ~VRDevice();

        // Init.
        bool Init();

        // IsActive.
        bool IsActive();

        // Shutdown.
        void Shutdown();

        // Update.
        void Update();

        // Get render size.
        std::uint32_t GetRenderWidth();
        std::uint32_t GetRenderHeight();
};
