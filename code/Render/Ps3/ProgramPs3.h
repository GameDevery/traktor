#ifndef traktor_render_ProgramPs3_H
#define traktor_render_ProgramPs3_H

#include <map>
#include <vector>
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/IProgram.h"
#include "Render/Ps3/TypesPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class MemoryHeap;
class MemoryHeapObject;
class ProgramResourcePs3;
class StateCachePs3;

class T_DLLCLASS ProgramPs3 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramPs3(int32_t& counter);

	virtual ~ProgramPs3();

	bool create(MemoryHeap* memoryHeap, const ProgramResourcePs3* resource);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);

	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	void bind(StateCachePs3& stateCache, const float targetSize[], uint32_t frameCounter);

	static void unbind();

	inline const std::vector< uint8_t >& getInputSignature() const { return m_inputSignature; }

private:
	static ProgramPs3* ms_activeProgram;

	enum DirtyFlags
	{
		DfVertex = SuVertex,
		DfPixel = SuPixel,
		DfTexture = 4
	};

	enum
	{
		PatchQueues = 2,
		MaxPatchInQueue = 64
	};

	Ref< MemoryHeap > m_memoryHeap;
	Ref< const ProgramResourcePs3 > m_resource;
	CGprogram m_vertexProgram;
	CGprogram m_pixelProgram;
	MemoryHeapObject* m_vertexShaderUCode;
	MemoryHeapObject* m_pixelShaderUCode;
	MemoryHeapObject* m_patchPixelShaderUCode[PatchQueues][MaxPatchInQueue];
	MemoryHeapObject* m_patchedPixelShaderUCode;
	uint32_t m_patchFrame;
	uint32_t m_patchCounter;
	std::vector< uint8_t > m_inputSignature;
	RenderState m_renderState;
	std::vector< ProgramScalar > m_vertexScalars;
	std::vector< ProgramScalar > m_pixelScalars;
	std::vector< uint32_t > m_pixelTargetSizeUCodeOffsets;
	std::vector< ProgramSampler > m_vertexSamplers;
	std::vector< ProgramSampler > m_pixelSamplers;
	std::map< handle_t, ScalarParameter > m_scalarParameterMap;
	std::map< handle_t, uint32_t > m_textureParameterMap;
	AlignedVector< float > m_scalarParameterData;
	RefArray< ITexture > m_textureParameterData;
	uint8_t m_dirty;
	float m_targetSize[2];
	int32_t& m_counter;
};

	}
}

#endif	// traktor_render_ProgramPs3_H
