#include "VRDevice.hpp"
#include "DxAssert.hpp"
#include "FrameBuffer.hpp"

#include <assert.h>
#include <iostream>

VRDevice::VRDevice()
{
    mpHMD = nullptr;
    mpRenderModels = nullptr;
    mLeftEyeFB = nullptr;
    mRightEyeFB = nullptr;
    InitHMD();
}

VRDevice::~VRDevice()
{
    if (mLeftEyeFB != nullptr) delete mLeftEyeFB;
    if (mRightEyeFB != nullptr) delete mRightEyeFB;
    if (IsActive()) Shutdown();
}

bool VRDevice::InitHMD()
{
    if (mpHMD != nullptr)
    {
        std::cout << "HMD Already initialised." << std::endl;
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
    mpHMD->GetRecommendedRenderTargetSize(&mRenderWidth, &mRenderHeight);

    // Get focus.
    vr::VRCompositor()->WaitGetPoses(NULL, 0, NULL, 0);

    return true;
}

void VRDevice::InitD3D(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    assert(mpHMD != nullptr);
    assert(mRenderWidth != 0 && mRenderHeight != 0);

    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mLeftEyeFB = new FrameBuffer(mpDevice, mpDeviceContext, mRenderWidth, mRenderHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    mRightEyeFB = new FrameBuffer(mpDevice, mpDeviceContext, mRenderWidth, mRenderHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
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

void VRDevice::Update()
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
        mEyePosLeft = ConvertMatrix(mpHMD->GetEyeToHeadTransform(vr::Hmd_Eye::Eye_Left));
        mEyePosLeft = glm::inverse(mEyePosLeft);
        mEyePosRight = ConvertMatrix(mpHMD->GetEyeToHeadTransform(vr::Hmd_Eye::Eye_Right));
        mEyePosRight = glm::inverse(mEyePosRight);
    }
    {
        float nearZ = 0.1f;
        float farZ = 200.f;
        mProjectionLeft = ConvertMatrix(mpHMD->GetProjectionMatrix(vr::Hmd_Eye::Eye_Left, nearZ, farZ));
        mProjectionRight = ConvertMatrix(mpHMD->GetProjectionMatrix(vr::Hmd_Eye::Eye_Right, nearZ, farZ));
    }

    // GetCurrentViewProjectionMatrix.
    mMVPLeft = mProjectionLeft * mEyePosLeft * mHMDTransform;
    mMVPRight = mProjectionRight * mEyePosRight * mHMDTransform;
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

std::uint32_t VRDevice::GetRenderWidth()
{
    if(mpHMD != nullptr) return mRenderWidth;
    std::cout << "HMD not initialised." << std::endl;
    return 0;
}

std::uint32_t VRDevice::GetRenderHeight()
{
    if (mpHMD != nullptr) return mRenderHeight;
    std::cout << "HMD not initialised." << std::endl;
    return 0;
}
