#include "VRDevice.hpp"
#include "DxAssert.hpp"
#include "DoubleFrameBuffer.hpp"
#include "FrameBuffer.hpp"

#include <assert.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

VRDevice::VRDevice()
{
    mpHMD = nullptr;
    mpRenderModels = nullptr;
    mpLeftFrameBuffer = nullptr;
    mpRightFrameBuffer = nullptr;
}

VRDevice::~VRDevice()
{
    if (IsActive()) Shutdown();
}

bool VRDevice::Start()
{
    if (mpHMD != nullptr)
    {
        std::cout << "HMD Already running." << std::endl;
        return true;
    }

    // Loading the SteamVR Runtime.
    vr::EVRInitError eError = vr::VRInitError_None;
    mpHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None)
    {
        mpHMD = nullptr;
        std::cout << "Unable to init VR runtime: " << vr::VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;
        return false;
    }

    mpRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
    if (!mpRenderModels)
    {
        mpHMD = nullptr;
        vr::VR_Shutdown();
        std::cout << "Unable to get render model interface: " << vr::VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;
        return false;
    }

    // SetupStereoRenderTargets.
    mpHMD->GetRecommendedRenderTargetSize(&mWidth, &mHeight);

    // Get focus.
    vr::VRCompositor()->WaitGetPoses(NULL, 0, NULL, 0);

    return true;
}

void VRDevice::InitD3D(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, DoubleFrameBuffer* leftFrameBuffer, DoubleFrameBuffer* rightFrameBuffer)
{
    assert(mpHMD != nullptr);
    assert(mWidth != 0 && mHeight != 0);

    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mpLeftFrameBuffer = leftFrameBuffer;
    mpRightFrameBuffer = rightFrameBuffer;
}

bool VRDevice::IsActive()
{
    return mpHMD != nullptr;
}

void VRDevice::Shutdown()
{
    vr::VR_Shutdown();
    mpHMD = nullptr;
    mpRenderModels = nullptr;
}

void VRDevice::Sync()
{
    assert(mpHMD != nullptr);

    // UpdateHMDMatrixPose.
    {
        vr::VRCompositor()->WaitGetPoses(mTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

        mValidPoseCount = 0;
        mStrPoseClasses = "";
        for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
        {
            if (mTrackedDevicePose[nDevice].bPoseIsValid)
            {
                mValidPoseCount++;
                mDeviceTransforms[nDevice] = ConvertMatrix(mTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
                if (mDeviceClassChar[nDevice] == 0)
                {
                    switch (mpHMD->GetTrackedDeviceClass(nDevice))
                    {
                    case vr::TrackedDeviceClass_Controller:        mDeviceClassChar[nDevice] = 'C'; break;
                    case vr::TrackedDeviceClass_HMD:               mDeviceClassChar[nDevice] = 'H'; break;
                    case vr::TrackedDeviceClass_Invalid:           mDeviceClassChar[nDevice] = 'I'; break;
                    case vr::TrackedDeviceClass_GenericTracker:    mDeviceClassChar[nDevice] = 'G'; break;
                    case vr::TrackedDeviceClass_TrackingReference: mDeviceClassChar[nDevice] = 'T'; break;
                    default:                                       mDeviceClassChar[nDevice] = '?'; break;
                    }
                }
                mStrPoseClasses += mDeviceClassChar[nDevice];
            }
        }

        if (mTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
        {
            mHMDTransform = mDeviceTransforms[vr::k_unTrackedDeviceIndex_Hmd];
            mHMDTransform = glm::inverse(mHMDTransform);
        }
    }
   
    // GetHMDMatrixPoseEye.
    {
        mLeftTransform = ConvertMatrix(mpHMD->GetEyeToHeadTransform(vr::Hmd_Eye::Eye_Left));
        mLeftTransform = glm::inverse(mLeftTransform);
        mRightTransform = ConvertMatrix(mpHMD->GetEyeToHeadTransform(vr::Hmd_Eye::Eye_Right));
        mRightTransform = glm::inverse(mRightTransform);
    }
    {
        float nearZ = 0.1f;
        float farZ = 200.f;
        mLeftProjection = ConvertMatrix(mpHMD->GetProjectionMatrix(vr::Hmd_Eye::Eye_Left, nearZ, farZ));
        mRightProjection = ConvertMatrix(mpHMD->GetProjectionMatrix(vr::Hmd_Eye::Eye_Right, nearZ, farZ));
    }
	
    mRightDirection = glm::vec3(mHMDTransform[0][0],mHMDTransform[1][0], mHMDTransform[2][0]);
    mUpDirection = glm::vec3(mHMDTransform[0][1], mHMDTransform[1][1], mHMDTransform[2][1]);
    mFrontDirection = glm::vec3(mHMDTransform[0][2], mHMDTransform[1][2], mHMDTransform[2][2]);
    glm::mat4 translation = glm::translate(glm::mat4(), glm::vec3(mPosition.x, mPosition.y, mPosition.z));

    mOrientationMatrix = glm::transpose(glm::mat4(
        glm::vec4(mRightDirection, 0.f),
        glm::vec4(mUpDirection, 0.f),
        glm::vec4(mFrontDirection, 0.f),
        glm::vec4(0.f, 0.f, 0.f, 1.f)
    ));

    mLeftView = mOrientationMatrix * mLeftTransform * mHMDTransform * translation;
    mRightView = mOrientationMatrix * mRightTransform * mHMDTransform * translation;

    // GetCurrentViewProjectionMatrix.
    //mLeftMVP = mLeftProjection * mLeftTransform * mHMDTransform * translation;
    //mRightMVP = mRightProjection * mRightTransform * mHMDTransform * translation;
}

void VRDevice::Submit()
{
    assert(mpLeftFrameBuffer != nullptr && mpRightFrameBuffer != nullptr);
    {   // Submit left eye.
        vr::Texture_t leftEyeTexture = { mpLeftFrameBuffer->GetFrameBuffer()->mColTex, vr::TextureType_DirectX, vr::ColorSpace_Auto };
        vr::EVRCompositorError eError = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        if (eError != vr::VRCompositorError_None) std::cout << "HMD Error rendering left eye" << std::endl;
    }
    {   // Submit right eye.
        vr::Texture_t rightEyeTexture = { mpRightFrameBuffer->GetFrameBuffer()->mColTex, vr::TextureType_DirectX, vr::ColorSpace_Auto };
        vr::EVRCompositorError eError = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
        if (eError != vr::VRCompositorError_None) std::cout << "HMD Error rendering right eye" << std::endl;
    }
}

void VRDevice::ClearFrameBuffers()
{
    assert(mpLeftFrameBuffer != nullptr && mpRightFrameBuffer != nullptr);
    mpLeftFrameBuffer->Clear(0.2f, 0.2f, 0.2f, 0.f);
    mpRightFrameBuffer->Clear(0.2f, 0.2f, 0.2f, 0.f);
}

glm::mat4 VRDevice::ConvertMatrix(const vr::HmdMatrix34_t& mat)
{
    glm::mat4 glmMat(
        mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,
        mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
        mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,
        mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
    );
    return glmMat;
}

glm::mat4 VRDevice::ConvertMatrix(const vr::HmdMatrix44_t& mat)
{
    glm::mat4 glmMat(
        mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
    );
    return glmMat;
}

std::uint32_t VRDevice::GetWidth()
{
    if(mpHMD != nullptr) return mWidth;
    std::cout << "HMD not initialised." << std::endl;
    return 0;
}

std::uint32_t VRDevice::GetHeight()
{
    if (mpHMD != nullptr) return mHeight;
    std::cout << "HMD not initialised." << std::endl;
    return 0;
}
