#include "VRDevice.hpp"
#include "DxAssert.hpp"

#include <assert.h>
#include <iostream>

VRDevice::VRDevice(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mpHMD = nullptr;
    mpRenderModels = nullptr;
    mHmdLeftTex = nullptr;
    mHmdRightTex = nullptr;
    mHmdLeftRTV = nullptr;
    mHmdRightRTV = nullptr;
    if (InitHMD())
    {
        InitD3D();
    }
}

VRDevice::~VRDevice()
{
    if (mHmdLeftTex != nullptr) mHmdLeftTex->Release();
    if (mHmdRightTex != nullptr) mHmdRightTex->Release();
    if (mHmdLeftRTV != nullptr) mHmdLeftRTV->Release();
    if (mHmdRightRTV != nullptr) mHmdRightRTV->Release();
    if (mHmdLeftSRV != nullptr) mHmdLeftSRV->Release();
    if (mHmdRightSRV != nullptr) mHmdRightSRV->Release();
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

void VRDevice::InitD3D()
{
    assert(mpHMD != nullptr);
    assert(mRenderWidth != 0 && mRenderHeight != 0);

    D3D11_TEXTURE2D_DESC texDesc;
    {   // --- Create VR render targets --- //
        ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
        texDesc.Width = mRenderWidth;
        texDesc.Height = mRenderHeight;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mHmdLeftTex), S_OK);;
        DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mHmdRightTex), S_OK);;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        DxAssert(mpDevice->CreateShaderResourceView(mHmdLeftTex, &srvDesc, &mHmdLeftSRV), S_OK);;
        DxAssert(mpDevice->CreateShaderResourceView(mHmdRightTex, &srvDesc, &mHmdRightSRV), S_OK);;

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        DxAssert(mpDevice->CreateRenderTargetView(mHmdLeftTex, &rtvDesc, &mHmdLeftRTV), S_OK);;
        DxAssert(mpDevice->CreateRenderTargetView(mHmdRightTex, &rtvDesc, &mHmdRightRTV), S_OK);;

        // Clear render targets.
        {
            float clrColor[4] = { 0.2f, 0.f, 0.f, 0.f };
            mpDeviceContext->ClearRenderTargetView(mHmdLeftRTV, clrColor);
        }
        {
            float clrColor[4] = { 0.f, 0.f, 0.2f, 0.f };
            mpDeviceContext->ClearRenderTargetView(mHmdRightRTV, clrColor);
        }
    }
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
        mProjectionLeft = glm::inverse(mProjectionLeft);
        mProjectionRight = ConvertMatrix(mpHMD->GetProjectionMatrix(vr::Hmd_Eye::Eye_Right, nearZ, farZ));
        mProjectionRight = glm::inverse(mProjectionRight);
    }

    // GetCurrentViewProjectionMatrix.
    mMVPLeft = mProjectionLeft * mEyePosLeft * mHMDTransform;
    mMVPRight = mProjectionRight * mEyePosRight *  mHMDTransform;
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
