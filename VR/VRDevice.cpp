#include "VRDevice.hpp"

#include <assert.h>
#include <iostream>

VRDevice::VRDevice()
{
    mpHMD = nullptr;
    mpRenderModels = nullptr;
}

VRDevice::~VRDevice()
{
    Shutdown();
}

bool VRDevice::Init()
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
}

bool VRDevice::IsActive()
{
    return mpHMD != nullptr;
}

void VRDevice::Shutdown()
{
    mpHMD = nullptr;
    mpRenderModels = nullptr;
    vr::VR_Shutdown();
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
        vr::HmdMatrix34_t matEye;
        matEye = mpHMD->GetEyeToHeadTransform(vr::Hmd_Eye::Eye_Left);
        mProjectionLeft = glm::mat4(
            matEye.m[0][0], matEye.m[1][0], matEye.m[2][0], 0.0,
            matEye.m[0][1], matEye.m[1][1], matEye.m[2][1], 0.0,
            matEye.m[0][2], matEye.m[1][2], matEye.m[2][2], 0.0,
            matEye.m[0][3], matEye.m[1][3], matEye.m[2][3], 1.0f
        );
        mProjectionLeft = glm::inverse(mProjectionLeft);

        matEye = mpHMD->GetEyeToHeadTransform(vr::Hmd_Eye::Eye_Right);
        mProjectionRight = glm::mat4(
            matEye.m[0][0], matEye.m[1][0], matEye.m[2][0], 0.0,
            matEye.m[0][1], matEye.m[1][1], matEye.m[2][1], 0.0,
            matEye.m[0][2], matEye.m[1][2], matEye.m[2][2], 0.0,
            matEye.m[0][3], matEye.m[1][3], matEye.m[2][3], 1.0f
        );
        mProjectionRight = glm::inverse(mProjectionRight);
    }

    // GetCurrentViewProjectionMatrix.
    mMVPLeft = mProjectionLeft * mEyeTransformLeft * mHMDTransform; // TODO order!
    mMVPRight = mProjectionRight * mEyeTransformRight *  mHMDTransform;
}

glm::mat4 VRDevice::ConvertMatrix(const vr::HmdMatrix34_t& matPose)
{
    glm::mat4 matrixObj(
        matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
        matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
        matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
        matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
    );
    return matrixObj;
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
