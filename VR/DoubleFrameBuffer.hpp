#pragma once

#include <d3d11.h>

class FrameBuffer;

class DoubleFrameBuffer
{
	public:
		// Constructor.
		// pDevice Pointer to D3D11 device.
		// pDeviceContext Pointer to D3D11 device context.
		// width Width in pixels.
		// height Height in pixels.
		// bindFlags Bind flags.
		// miscFlags Misc flags.
		// initTexture Texture make frame buffer.
		DoubleFrameBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int width, unsigned int height, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, UINT miscFlags = 0U, ID3D11Texture2D* initTexture = nullptr);

		// Destructor.
		~DoubleFrameBuffer();

		// Clear color.
		void Clear(float r, float g, float b, float a, float depth);

		// Swap current frame buffer.
		void Swap();

		// Get current frame buffer.
		FrameBuffer* GetFrameBuffer();

		// Frame buffer width in pixels.
		unsigned int mWidth;
		// Frame buffer height in pixels.
		unsigned int mHeight;

		// Number of buffers.
		static const unsigned int mNumBuff = 2;

	private:
		ID3D11Device* mpDevice;
		ID3D11DeviceContext* mpDeviceContext;

		// Frame buffers.
		FrameBuffer* mFrameBuffers[mNumBuff];

		// Index of current frame buffer.
		unsigned int mIndex;
};
