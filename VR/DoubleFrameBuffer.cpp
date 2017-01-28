#include "DoubleFrameBuffer.hpp"
#include "DxHelp.hpp"
#include "FrameBuffer.hpp"

DoubleFrameBuffer::DoubleFrameBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int width, unsigned int height, UINT bindFlags, UINT miscFlags, ID3D11Texture2D* initTexture)
{
	mpDevice = pDevice;
	mpDeviceContext = pDeviceContext;
	mWidth = width;
	mHeight = height;
	mIndex = 0;

	for (unsigned int i = 0; i < mNumBuff; ++i)
		mFrameBuffers[i] = new FrameBuffer(mpDevice, mpDeviceContext, mWidth, mHeight, bindFlags, miscFlags, initTexture);
}

DoubleFrameBuffer::~DoubleFrameBuffer()
{
	for (unsigned int i = 0; i < mNumBuff; ++i)
		delete mFrameBuffers[i];
}

void DoubleFrameBuffer::Clear(float r, float g, float b, float a)
{
	mFrameBuffers[mIndex]->Clear(r, g, b, a);
}

void DoubleFrameBuffer::Swap()
{
	mIndex = (mIndex + 1) % mNumBuff;
}

FrameBuffer* DoubleFrameBuffer::GetFrameBuffer()
{
	return mFrameBuffers[mIndex];
}
