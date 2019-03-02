#pragma once

namespace traktor
{
	namespace render
	{

void* cglwCreateContext(void* nativeWindowHandle, void* sharedContext, int depthBits, int stencilBits, int multisample);

void cglwUpdate(void* context);

void cglwGetSize(void* context, int32_t& outWidth, int32_t& outHeight);

bool cglwMakeCurrent(void* context);

void cglwSwapBuffers(void* context, int32_t waitVBlanks);

bool cglwCheckHardwarePath();

	}
}

