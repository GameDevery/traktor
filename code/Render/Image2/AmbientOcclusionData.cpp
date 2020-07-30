#include "Core/Math/RandomGeometry.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Image2/AmbientOcclusion.h"
#include "Render/Image2/AmbientOcclusionData.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.AmbientOcclusionData", 0, AmbientOcclusionData, ImagePassOpData)

Ref< const ImagePassOp > AmbientOcclusionData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
	Ref< AmbientOcclusion > instance = new AmbientOcclusion();
	RandomGeometry random;

	// Bind shader.
	if (!resourceManager->bind(m_shader, instance->m_shader))
		return nullptr;

	// Get handles of sources.
	for (const auto& source : m_sources)
	{
		instance->m_sources.push_back({
			getParameterHandle(source.textureId),
			getParameterHandle(source.parameter)
		});
	}

	// Create offsets.
	for (int i = 0; i < sizeof_array(instance->m_offsets); ++i)
	{
		float r = random.nextFloat() * (1.0f - 0.1f) + 0.1f;
		instance->m_offsets[i] = random.nextUnit().xyz0() + Vector4(0.0f, 0.0f, 0.0f, r);
	}

	// Create directions.
	for (int i = 0; i < sizeof_array(instance->m_directions); ++i)
	{
		float a =  TWO_PI * float(i) / sizeof_array(instance->m_directions);
		float c = std::cos(a);
		float s = std::sin(a);
		instance->m_directions[i] = Vector4(c, s, 0.0f, 0.0f);
	}

	// Create random normals texture.
	AutoArrayPtr< uint8_t > data(new uint8_t [256 * 256 * 4]);
	for (uint32_t y = 0; y < 256; ++y)
	{
		for (uint32_t x = 0; x < 256; ++x)
		{
			Vector4 normal = random.nextUnit();
			normal = normal * Scalar(0.5f) + Scalar(0.5f);

			data[(x + y * 256) * 4 + 0] = (uint8_t)(normal.x() * 255);
			data[(x + y * 256) * 4 + 1] = (uint8_t)(normal.y() * 255);
			data[(x + y * 256) * 4 + 2] = (uint8_t)(normal.z() * 255);
			data[(x + y * 256) * 4 + 3] = 0;
		}
	}

	SimpleTextureCreateDesc desc;
	desc.width = 256;
	desc.height = 256;
	desc.mipCount = 1;
	desc.format = TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 256 * 4;
	desc.initialData[0].slicePitch = 0;

	instance->m_randomNormals = renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
	if (!instance->m_randomNormals)
		return nullptr;

	// Create random rotations texture.
	for (uint32_t y = 0; y < 256; ++y)
	{
		for (uint32_t x = 0; x < 256; ++x)
		{
			float a = random.nextFloat() * TWO_PI;
			float c = std::cos(a);
			float s = std::sin(a);
			float j = random.nextFloat();

			data[(x + y * 256) * 4 + 0] = (uint8_t)((c * 0.5f + 0.5f) * 255);
			data[(x + y * 256) * 4 + 1] = (uint8_t)((s * 0.5f + 0.5f) * 255);
			data[(x + y * 256) * 4 + 2] = (uint8_t)(j * 255);
			data[(x + y * 256) * 4 + 3] = 0;
		}
	}

	desc.width = 256;
	desc.height = 256;
	desc.mipCount = 1;
	desc.format = TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 256 * 4;
	desc.initialData[0].slicePitch = 0;

	instance->m_randomRotations = renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
	if (!instance->m_randomRotations)
		return nullptr;

	return instance; 
}

	}
}