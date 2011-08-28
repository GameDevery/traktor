#ifndef traktor_render_Shader_H
#define traktor_render_Shader_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

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
class IProgram;
class IRenderView;
class ProgramParameters;

/*! \brief Shader
 * \ingroup Render
 *
 * An shader is actually only a facad� for several
 * programs which is different combinations of a
 * source shader graph.
 */
class T_DLLCLASS Shader : public Object
{
	T_RTTI_CLASS;

public:
	Shader();

	virtual ~Shader();

	void destroy();

	bool hasTechnique(handle_t handle) const;

	/*! \brief Set shader technique.
	 *
	 * \note
	 * Shader parameters are invalid when changing
	 * technique.
	 *
	 * \param handle Technique handle.
	 */
	void setTechnique(handle_t handle);

	/*! \brief Get set of shader techniques.
	 *
	 * \param outHandles Set of technique handles.
	 */
	void getTechniques(std::set< render::handle_t >& outHandles) const;

	/*! \brief Set shader combination.
	 *
	 * Select proper permutation from shader
	 * branches.
	 *
	 * \note
	 * Shader parameters are invalid when changing
	 * combination.
	 *
	 * \param handle Branch parameter handle.
	 * \param param Branch path.
	 */
	void setCombination(handle_t handle, bool param);

	void setFloatParameter(handle_t handle, float param);

	void setFloatArrayParameter(handle_t handle, const float* param, int length);

	void setVectorParameter(handle_t handle, const Vector4& param);

	void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	void setMatrixParameter(handle_t handle, const Matrix44& param);

	void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	void setTextureParameter(handle_t handle, const resource::Proxy< ITexture >& texture);

	void setStencilReference(uint32_t stencilReference);

	/*! \brief Draw primitives with this shader.
	 *
	 * \param renderView Render primitives view.
	 * \param primitives Primitives.
	 */
	void draw(IRenderView* renderView, const Primitives& primitives);

	/*! \name Program access
	 */
	//@{

	IProgram* getCurrentProgram() const;

	void setProgramParameters(ProgramParameters* programParameters);

	//@}

	/*! \name Set parameter by name.
	 *
	 * These methods are implemented for backward compatibility and should not be used in
	 * time critical paths.
	 */
	//@{

	inline bool hasTechnique(const std::wstring& name) const { return hasTechnique(getParameterHandle(name)); }

	inline void setTechnique(const std::wstring& name) { setTechnique(getParameterHandle(name)); }

	inline void setCombination(const std::wstring& name, bool param) { setCombination(getParameterHandle(name), param); }

	inline void setFloatParameter(const std::wstring& name, float param) { setFloatParameter(getParameterHandle(name), param); }

	inline void setFloatArrayParameter(const std::wstring& name, const float* param, int length) { setFloatArrayParameter(getParameterHandle(name), param, length); }

	inline void setVectorParameter(const std::wstring& name, const Vector4& param) { setVectorParameter(getParameterHandle(name), param); }

	inline void setVectorArrayParameter(const std::wstring& name, const Vector4* param, int length) { setVectorArrayParameter(getParameterHandle(name), param, length); }

	inline void setMatrixParameter(const std::wstring& name, const Matrix44& param) { setMatrixParameter(getParameterHandle(name), param); }

	inline void setMatrixArrayParameter(const std::wstring& name, const Matrix44* param, int length) { setMatrixArrayParameter(getParameterHandle(name), param, length); }

	inline void setTextureParameter(const std::wstring& name, const resource::Proxy< ITexture >& texture) { setTextureParameter(getParameterHandle(name), texture); }

	//@}

private:
	friend class ShaderFactory;

	struct Combination
	{
		uint32_t parameterValue;
		Ref< IProgram > program;
	};

	struct Technique
	{
		uint32_t parameterMask;
		AlignedVector< Combination > combinations;
	};

	SmallMap< handle_t, Technique > m_techniques;
	SmallMap< handle_t, uint32_t > m_parameterBits;
	SmallMap< handle_t, resource::Proxy< ITexture > > m_textureProxies;
	uint32_t m_parameterValue;
	Technique* m_currentTechnique;
	IProgram* m_currentProgram;

	void updateCurrentProgram();
};

	}
}

#endif	// traktor_render_Shader_H
