#pragma once

#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;
class ProgramParameters;
class StructBuffer;

class T_DLLCLASS RenderClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const override final;
};

class T_DLLCLASS BoxedProgramParameters : public Object
{
	T_RTTI_CLASS;

public:
	BoxedProgramParameters() = default;

	explicit BoxedProgramParameters(ProgramParameters* programParameters);

	void setProgramParameters(ProgramParameters* programParameters);

	void setFloatParameter(const handle_t handle, float param);

	void setVectorParameter(const handle_t handle, const Vector4& param);

	void setVectorArrayParameter(const handle_t handle, const AlignedVector< Vector4 >& param);

	void setMatrixParameter(handle_t handle, const Matrix44& param);

	void setMatrixArrayParameter(handle_t handle, const AlignedVector< Matrix44 >& param);

	void setTextureParameter(const handle_t handle, ITexture* texture);

	void setStructBufferParameter(const handle_t handle, StructBuffer* structBuffer);

	void setStencilReference(uint32_t stencilReference);

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	ProgramParameters* m_programParameters = nullptr;
};

	}
}

