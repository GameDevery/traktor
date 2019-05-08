#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/ES2/Platform.h"
#include "Render/OpenGL/ES2/ProgramResourceOpenGLES2.h"
#include "Render/OpenGL/ES2/CubeTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetDepthOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Render/OpenGL/ES2/StateCache.h"
#include "Render/OpenGL/ES2/VolumeTextureOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/ContextOpenGLES2.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES2/Emscripten/ContextOpenGLES2.h"
#elif defined(__PNACL__)
#	include "Render/OpenGL/ES2/PNaCl/ContextOpenGLES2.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/ContextOpenGLES2.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES2/Linux/ContextOpenGLES2.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteProgramCallback : public ContextOpenGLES2::IDeleteCallback
{
	GLuint m_programName;

	DeleteProgramCallback(GLuint programName)
	:	m_programName(programName)
	{
	}

	virtual ~DeleteProgramCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteProgram(m_programName));
		delete this;
	}
};

bool storeIfNotEqual(const float* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (dest[i] != source[i])
		{
			for (; i < length; ++i)
				dest[i] = source[i];
			return true;
		}
	}
	return false;
}

bool storeIfNotEqual(const Vector4* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (Vector4::loadAligned(&dest[i * 4]) != source[i])
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4]);
			return true;
		}
	}
	return false;
}

bool storeIfNotEqual(const Matrix44* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (Matrix44::loadAligned(&dest[i * 4 * 4]) != source[i])
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4 * 4]);
			return true;
		}
	}
	return false;
}

void bindAttribute(GLuint programObject, DataUsage usage, int32_t index)
{
	std::string attributeName = wstombs(VertexAttribute::getName(usage, index));

	int32_t attributeLocation = VertexAttribute::getLocation(usage, index);
	if (attributeLocation < 0)
		return;

	T_OGL_SAFE(glBindAttribLocation(
		programObject,
		attributeLocation,
		attributeName.c_str()
	));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGLES2", ProgramOpenGLES2, IProgram)

ProgramOpenGLES2* ProgramOpenGLES2::ms_current = 0;

ProgramOpenGLES2::~ProgramOpenGLES2()
{
	destroy();
}

Ref< ProgramOpenGLES2 > ProgramOpenGLES2::create(ContextOpenGLES2* resourceContext, const ProgramResource* resource)
{
	const ProgramResourceOpenGLES2* resourceOpenGL = mandatory_non_null_type_cast< const ProgramResourceOpenGLES2* >(resource);
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	const std::string& vertexShader = resourceOpenGL->getVertexShader();
	const std::string& fragmentShader = resourceOpenGL->getFragmentShader();

	GLuint vertexObject = resourceContext->createShaderObject(vertexShader.c_str(), GL_VERTEX_SHADER);
	if (!vertexObject)
	{
		log::error << L"Unable to create vertex object" << Endl;
		return 0;
	}

	GLuint fragmentObject = resourceContext->createShaderObject(fragmentShader.c_str(), GL_FRAGMENT_SHADER);
	if (!fragmentObject)
	{
		log::error << L"Unable to create fragment object" << Endl;
		return 0;
	}

	GLuint programObject = glCreateProgram();
	T_ASSERT(programObject != 0);

	T_OGL_SAFE(glAttachShader(programObject, vertexObject));
	T_OGL_SAFE(glAttachShader(programObject, fragmentObject));

	for (int j = 0; j < T_OGL_MAX_INDEX; ++j)
	{
		bindAttribute(programObject, DuPosition, j);
		bindAttribute(programObject, DuNormal, j);
		bindAttribute(programObject, DuTangent, j);
		bindAttribute(programObject, DuBinormal, j);
		bindAttribute(programObject, DuColor, j);
		bindAttribute(programObject, DuCustom, j);
	}

	T_OGL_SAFE(glLinkProgram(programObject));

	T_OGL_SAFE(glGetProgramiv(programObject, GL_LINK_STATUS, &status));
	if (status != GL_TRUE)
	{
		T_OGL_SAFE(glGetProgramInfoLog(programObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program link failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return 0;
		}
	}

	// Prewarm shader to prevent stall when first used.
	while (glGetError() != GL_NO_ERROR)
		;

#if !defined(__EMSCRIPTEN__)
	glUseProgram(programObject);
	glDrawArrays(GL_TRIANGLES, 0, 0);

	if (glGetError() != GL_NO_ERROR)
		log::warning << L"Prewarming shader failed; might cause stall during normal render" << Endl;
#endif

	return new ProgramOpenGLES2(resourceContext, programObject, resource);
}

void ProgramOpenGLES2::destroy()
{
	if (ms_current == this)
		ms_current = 0;

	if (m_program)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteProgramCallback(m_program));
		m_program = 0;
	}
}

void ProgramOpenGLES2::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGLES2::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGLES2::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setMatrixArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGLES2::setTextureParameter(handle_t handle, ITexture* texture)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_textures[i->second] = texture;
}

void ProgramOpenGLES2::setStructBufferParameter(handle_t handle, StructBuffer* structBuffer)
{
	T_FATAL_ERROR;
}

void ProgramOpenGLES2::setStencilReference(uint32_t stencilReference)
{
	m_renderState.stencilRef = stencilReference;
}

bool ProgramOpenGLES2::activate(StateCache* stateCache, float targetSize[2], float postTransform[4], bool invertCull, uint32_t instanceID)
{
	// Bind program and set state display list.
	stateCache->setRenderState(m_renderState, invertCull);
	stateCache->setProgram(m_program);

	// Update dirty uniforms.
	for (AlignedVector< Uniform >::iterator i = m_uniforms.begin(); i != m_uniforms.end(); ++i)
	{
		if (!i->dirty)
			continue;

		const float* uniformData = &m_uniformData[i->offset];
		switch (i->type)
		{
		case GL_FLOAT:
			T_OGL_SAFE(glUniform1fv(i->location, i->length, uniformData));
			break;

		case GL_FLOAT_VEC4:
			T_OGL_SAFE(glUniform4fv(i->location, i->length, uniformData));
			break;

		case GL_FLOAT_MAT4:
			T_OGL_SAFE(glUniformMatrix4fv(i->location, i->length, GL_FALSE, uniformData));
			break;

		default:
			T_ASSERT(0);
		}

		i->dirty = false;
	}

	// Update target size uniform if necessary.
	if (m_locationTargetSize != -1)
	{
		if (m_targetSize[0] != targetSize[0] || m_targetSize[1] != targetSize[1])
		{
			m_targetSize[0] = targetSize[0];
			m_targetSize[1] = targetSize[1];
			T_OGL_SAFE(glUniform4fv(m_locationTargetSize, 1, m_targetSize));
		}
	}

	// Update post transform.
	if (m_locationPostTransform != -1)
	{
		T_OGL_SAFE(glUniform4fv(m_locationPostTransform, 1, postTransform));
	}

	// Update instance id.
	if (m_locationInstanceID != -1)
	{
		T_OGL_SAFE(glUniform1f(m_locationInstanceID, GLfloat(instanceID)));
	}

	// Bind textures.
	T_ASSERT(m_samplers.size() <= 8);
	uint32_t nsamplers = m_samplers.size();
	for (uint32_t i = 0; i < nsamplers; ++i)
	{
		const Sampler& sampler = m_samplers[i];
		const SamplerStateOpenGL& samplerState = m_renderState.samplerStates[sampler.unit];

		if (!m_textures[sampler.texture])
			continue;

		Ref< ITexture > resolved = m_textures[sampler.texture]->resolve();
		if (!resolved)
			continue;

		ITextureBinding* binding = 0;

		if (SimpleTextureOpenGLES2* st = dynamic_type_cast< SimpleTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(st);
		else if (CubeTextureOpenGLES2* ct = dynamic_type_cast< CubeTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(ct);
		else if (VolumeTextureOpenGLES2* vt = dynamic_type_cast< VolumeTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(vt);
		else if (RenderTargetOpenGLES2* rt = dynamic_type_cast< RenderTargetOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(rt);
		else if (RenderTargetDepthOpenGLES2* rtd = dynamic_type_cast< RenderTargetDepthOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(rtd);

		if (binding)
		{
			binding->bindSampler(
				i,
				samplerState,
				sampler.locationTexture
			);
		}
	}

	// Bind texture sizes.
	uint32_t ntextureSize = m_textureSize.size();
	for (uint32_t i = 0; i < ntextureSize; ++i)
	{
		const TextureSize& textureSize = m_textureSize[i];

		ITexture* texture = m_textures[textureSize.texture];
		if (!texture)
			continue;

		Ref< ITexture > resolved = texture->resolve();
		if (!resolved)
			continue;

		ITextureBinding* binding = 0;

		if (SimpleTextureOpenGLES2* st = dynamic_type_cast< SimpleTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(st);
		else if (CubeTextureOpenGLES2* ct = dynamic_type_cast< CubeTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(ct);
		else if (VolumeTextureOpenGLES2* vt = dynamic_type_cast< VolumeTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(vt);
		else if (RenderTargetOpenGLES2* rt = dynamic_type_cast< RenderTargetOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(rt);
		else if (RenderTargetDepthOpenGLES2* rtd = dynamic_type_cast< RenderTargetDepthOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(rtd);

		if (binding)
			binding->bindSize(textureSize.location);
	}

	ms_current = this;
	return true;
}

ProgramOpenGLES2::ProgramOpenGLES2(ContextOpenGLES2* resourceContext, GLuint program, const ProgramResource* resource)
:	m_resourceContext(resourceContext)
,	m_program(program)
,	m_locationTargetSize(0)
,	m_locationPostTransform(0)
,	m_locationInstanceID(0)
{
	const ProgramResourceOpenGLES2* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGLES2* >(resource);

	m_targetSize[0] =
	m_targetSize[1] =
	m_targetSize[2] =
	m_targetSize[3] = 0.0f;

	// Get target size parameter.
	m_locationTargetSize = glGetUniformLocation(m_program, "_gl_targetSize");
	m_locationPostTransform = glGetUniformLocation(m_program, "_gl_postTransform");
	m_locationInstanceID = glGetUniformLocation(m_program, "_gl_instanceID");

	const std::vector< std::wstring >& textures = resourceOpenGL->getTextures();
	const std::vector< SamplerBindingOpenGL >& samplers = resourceOpenGL->getSamplers();

	// Map texture parameters.
	for (std::vector< SamplerBindingOpenGL >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		const std::wstring& texture = textures[i->texture];

		handle_t handle = getParameterHandle(texture);
		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textures.size();
			m_textures.push_back(0);
		}

		std::wstring samplerName = i->name;

		Sampler sampler;
		sampler.locationTexture = glGetUniformLocation(m_program, wstombs(samplerName).c_str());
		sampler.texture = m_parameterMap[handle];
		sampler.unit = i->unit;

		m_samplers.push_back(sampler);

		if (sampler.locationTexture < 0)
			log::warning << L"No GL sampler defined for texture \"" << texture << L"\"" << Endl;
	}

	// Map texture size parameters.
	for (std::vector< std::wstring >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		const std::wstring& texture = *i;
		std::wstring textureSizeName = L"_gl_textureSize_" + texture;

		GLint location = glGetUniformLocation(m_program, wstombs(textureSizeName).c_str());
		if (location <= 0)
			continue;

		handle_t handle = getParameterHandle(texture);

		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textures.size();
			m_textures.push_back(0);
		}

		TextureSize textureSize;
		textureSize.location = location;
		textureSize.texture = m_parameterMap[handle];

		m_textureSize.push_back(textureSize);
	}

	const std::vector< NamedUniformType >& uniforms = resourceOpenGL->getUniforms();
	for (uint32_t i = 0; i < uint32_t(uniforms.size()); ++i)
	{
		handle_t handle = getParameterHandle(uniforms[i].name);
		T_FATAL_ASSERT_M (m_parameterMap.find(handle) == m_parameterMap.end(), L"Duplicated uniform in resource");

		std::string uniformName = wstombs(uniforms[i].name);

		GLint location = glGetUniformLocation(m_program, uniformName.c_str());
		if (location < 0)
		{
			// This probably happen when the GLSL optimizer of the driver is more intelligent than
			// our GLSL shader generator, thus the uniform has been discarded when the program was loaded.
			T_DEBUG(L"No location of GL uniform \"" << uniforms[i].name << L"\"; shader parameter ignored.");
			continue;
		}

		uint32_t offsetUniform = uint32_t(m_uniforms.size());
		uint32_t offsetData = uint32_t(m_uniformData.size());
		uint32_t allocSize = 0;

		switch (uniforms[i].type)
		{
		case GL_FLOAT:
			allocSize = alignUp(1 * uniforms[i].length, 4);
			break;

		case GL_FLOAT_VEC4:
			allocSize = 4 * uniforms[i].length;
			break;

		case GL_FLOAT_MAT4:
			allocSize = 16 * uniforms[i].length;
			break;

		default:
			T_FATAL_ERROR;
			break;
		}

		m_parameterMap[handle] = offsetUniform;

		m_uniforms.push_back(Uniform());
		m_uniforms.back().location = location;
		m_uniforms.back().type = uniforms[i].type;
		m_uniforms.back().offset = offsetData;
		m_uniforms.back().length = uniforms[i].length;
		m_uniforms.back().dirty = true;

		m_uniformData.resize(offsetData + allocSize, 0.0f);
	}

	// Create a display list from the render states.
	m_renderState = resourceOpenGL->getRenderState();
}

	}
}
