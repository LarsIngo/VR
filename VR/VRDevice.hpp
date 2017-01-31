#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <glm/glm.hpp>
#include <openvr.h>
#include <string>

class DoubleFrameBuffer;

class VRDevice
{
    public:
        // Constructor.
        VRDevice();

        // Destructor.
        ~VRDevice();

        // Init HMD(VR).
        bool Start();

        // Init D3D resources.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // leftFrameBuffer Frame buffer to render.
        // rightFrameBuffer Frame buffer to render.
        void InitD3D(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, DoubleFrameBuffer* leftFrameBuffer, DoubleFrameBuffer* rightFrameBuffer);

        // IsActive.
        bool IsActive();

        // Shutdown.
        void Shutdown();

        // Sync HMD. Get poses from devices.
        void Sync();

        // Submit frame buffers to screen.
        void Submit();

        // Clear D3D frame buffers.
        void ClearFrameBuffers();

        // Get render size.
        std::uint32_t GetWidth();
        std::uint32_t GetHeight();

        DoubleFrameBuffer* mpLeftFrameBuffer;
        DoubleFrameBuffer* mpRightFrameBuffer;

        glm::vec3 mPosition;
		glm::vec3 mRightDirection;
		glm::vec3 mUpDirection;
		glm::vec3 mFrontDirection;
        glm::mat4 mOrientationMatrix;

        glm::mat4 mHMDTransform;

        glm::mat4 mLeftTransform;
        glm::mat4 mRightTransform;

        glm::mat4 mLeftView;
        glm::mat4 mRightView;

        glm::mat4 mLeftProjection;
        glm::mat4 mRightProjection;

    private:
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

        std::uint32_t mWidth;
        std::uint32_t mHeight;
};
