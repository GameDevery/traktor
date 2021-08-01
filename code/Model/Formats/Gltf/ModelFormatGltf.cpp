#include "Core/Io/FileSystem.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"
#include "Json/JsonDocument.h"
#include "Json/JsonObject.h"
#include "Model/Formats/Gltf/ModelFormatGltf.h"
#include "Model/Model.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

Matrix44 parseTransform(const json::JsonObject* node)
{
	Matrix44 transform = Matrix44::identity();
	if (node->getMember(L"translation") != nullptr)
	{
		auto translation = node->getMemberValue(L"translation").getObject< json::JsonArray >();
		if (translation != nullptr && translation->size() >= 3)
		{
			transform = transform * translate(
				translation->get(0).getFloat(),
				translation->get(1).getFloat(),
				translation->get(2).getFloat()
			);
		}
	}
	if (node->getMember(L"rotation") != nullptr)
	{
		auto rotation = node->getMemberValue(L"rotation").getObject< json::JsonArray >();
		if (rotation != nullptr && rotation->size() >= 4)
		{
			transform = transform * Quaternion(
				rotation->get(0).getFloat(),
				rotation->get(1).getFloat(),
				rotation->get(2).getFloat(),
				rotation->get(3).getFloat()
			).toMatrix44();
		}
	}
	if (node->getMember(L"scale") != nullptr)
	{
		auto scale = node->getMemberValue(L"scale").getObject< json::JsonArray >();
		if (scale != nullptr && scale->size() >= 3)
		{
			transform = transform * traktor::scale(
				scale->get(0).getFloat(),
				scale->get(1).getFloat(),
				scale->get(2).getFloat()
			);
		}
	}
	return transform;
}

bool decodeAsIndices(
	int32_t index,
	json::JsonArray* accessors,
	json::JsonArray* bufferViews,
	const RefArray< IStream >& bufferStreams,
	AlignedVector< int32_t >& outData
)
{
	auto accessor = accessors->get(index).getObject< json::JsonObject >();

	int32_t bufferViewIndex = accessor->getMemberValue(L"bufferView").getInt32();
	int32_t componentType = accessor->getMemberValue(L"componentType").getInt32();
	std::wstring type = accessor->getMemberValue(L"type").getWideString();
	int32_t count = accessor->getMemberValue(L"count").getInt32();

	if (componentType != 5123)	// must be uint16
		return false;

	if (type != L"SCALAR")
		return false;

	auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
	if (!bufferView)
		return false;

	int32_t buffer = bufferView->getMemberValue(L"buffer").getInt32();
	int32_t byteOffset = bufferView->getMemberValue(L"byteOffset").getInt32();

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset);

	outData.resize(count);
	for (int32_t i = 0; i < count; ++i)
	{
		uint16_t v;
		if (bufferStream->read(&v, sizeof(v)) != sizeof(v))
			return false;
		outData[i] = int32_t(v);
	}

	return true;
}

bool decodeAsVectors(
	int32_t index,
	json::JsonArray* accessors,
	json::JsonArray* bufferViews,
	const RefArray< IStream >& bufferStreams,
	AlignedVector< Vector4 >& outData
)
{
	auto accessor = accessors->get(index).getObject< json::JsonObject >();

	int32_t bufferViewIndex = accessor->getMemberValue(L"bufferView").getInt32();
	int32_t componentType = accessor->getMemberValue(L"componentType").getInt32();
	std::wstring type = accessor->getMemberValue(L"type").getWideString();
	int32_t count = accessor->getMemberValue(L"count").getInt32();

	if (componentType != 5126)	// must be float
		return false;

	int32_t width = 0;
	if (type == L"SCALAR")
		width = 1;
	else if (type == L"VEC2")
		width = 2;
	else if (type == L"VEC3")
		width = 3;
	else if (type == L"VEC4")
		width = 4;
	else
		return false;

	auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
	if (!bufferView)
		return false;

	int32_t buffer = bufferView->getMemberValue(L"buffer").getInt32();
	int32_t byteOffset = bufferView->getMemberValue(L"byteOffset").getInt32();

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset);

	outData.resize(count);
	for (int32_t i = 0; i < count; ++i)
	{
		float v[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		if (bufferStream->read(v, width * sizeof(float)) != width * sizeof(float))
			return false;
		outData[i] = Vector4::loadUnaligned(v);
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatGltf", 0, ModelFormatGltf, ModelFormat)

void ModelFormatGltf::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"glTF Object";
	outExtensions.push_back(L"gltf");
}

bool ModelFormatGltf::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"gltf") == 0;
}

Ref< Model > ModelFormatGltf::read(const Path& filePath, const std::wstring& filter) const
{
	RefArray< IStream > bufferStreams;

	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!stream)
		return nullptr;

	json::JsonDocument doc;
	if (!doc.loadFromStream(stream))
		return nullptr;

	if (doc.empty())
		return nullptr;
	if (!doc.front().isObject< json::JsonObject >())
		return nullptr;

	auto docobj = doc.front().getObjectUnsafe< json::JsonObject >();
	T_ASSERT(docobj != nullptr);

	// Create buffer streams.
	auto buffers = docobj->getMemberValue(L"buffers").getObject< json::JsonArray >();
	if (buffers)
	{
		Path dirPath(filePath.getPathOnly());

		bufferStreams.resize(buffers->size());
		for (uint32_t i = 0; i < buffers->size(); ++i)
		{
			auto buffer = buffers->get(i).getObject< json::JsonObject >();
			if (!buffer)
				return nullptr;

			std::wstring uri = buffer->getMemberValue(L"uri").getWideString();

			if (startsWith(uri, L"data:application/octet-stream;base64,"))
			{
				AlignedVector< uint8_t > data = Base64().decode(uri.substr(37));
				Ref< DynamicMemoryStream > ms = new DynamicMemoryStream(true, false);
				ms->getBuffer().swap(data);
				bufferStreams[i] = ms;
			}
			else
			{
				if ((bufferStreams[i] = FileSystem::getInstance().open(dirPath + Path(uri), File::FmRead)) == nullptr)
					return nullptr;
			}
		}
	}

	auto bufferViews = docobj->getMemberValue(L"bufferViews").getObject< json::JsonArray >();
	auto accessors = docobj->getMemberValue(L"accessors").getObject< json::JsonArray >();
	auto textures = docobj->getMemberValue(L"textures").getObject< json::JsonArray >();
	auto images = docobj->getMemberValue(L"images").getObject< json::JsonArray >();

	Ref< Model > md = new Model();
	md->addUniqueTexCoordChannel(L"UV0");

	// Parse materials.
	auto materials = docobj->getMemberValue(L"materials").getObject< json::JsonArray >();
	if (materials)
	{
		for (uint32_t i = 0; i < materials->size(); ++i)
		{
			auto material = materials->get(i).getObject< json::JsonObject >();
			if (!material)
				return nullptr;

			Material mt;

			// Name
			auto name = material->getMemberValue(L"name").getWideString();
			mt.setName(name);

			// Normal map
			auto normalTexture = material->getMemberValue(L"normalTexture").getObject< json::JsonObject >();
			if (normalTexture)
			{
				int32_t index = normalTexture->getMemberValue(L"index").getInt32();
				if (!textures || index < 0 || index >= (int32_t)textures->size())
					return nullptr;

				auto texture = textures->get(index).getObject< json::JsonObject >();
				int32_t source = texture->getMemberValue(L"source").getInt32();
				if (!images || source < 0 || source >= (int32_t)images->size())
					return nullptr;

				auto image = images->get(source).getObject< json::JsonObject >();
				auto uri = image->getMemberValue(L"uri").getWideString();
				mt.setNormalMap(Material::Map(uri, L"UV0", true));
			}

			auto pbrMetallicRoughness = material->getMemberValue(L"pbrMetallicRoughness").getObject< json::JsonObject >();
			if (pbrMetallicRoughness)
			{
				auto baseColorTexture = pbrMetallicRoughness->getMemberValue(L"baseColorTexture").getObject< json::JsonObject >();
				if (baseColorTexture)
				{
					int32_t index = baseColorTexture->getMemberValue(L"index").getInt32();
					if (!textures || index < 0 || index >= (int32_t)textures->size())
						return nullptr;

					auto texture = textures->get(index).getObject< json::JsonObject >();
					int32_t source = texture->getMemberValue(L"source").getInt32();
					if (!images || source < 0 || source >= (int32_t)images->size())
						return nullptr;

					auto image = images->get(source).getObject< json::JsonObject >();

					std::wstring name;
					if (image->getMember(L"uri") != nullptr)
						name = image->getMemberValue(L"uri").getWideString();
					else if (image->getMember(L"name") != nullptr)
						name = image->getMemberValue(L"name").getWideString();
					if (name.empty())
						return nullptr;

					mt.setDiffuseMap(Material::Map(name, L"UV0", true));
				}

				auto metallicRoughnessTexture = pbrMetallicRoughness->getMemberValue(L"metallicRoughnessTexture").getObject< json::JsonObject >();
				if (metallicRoughnessTexture)
				{
					int32_t index = metallicRoughnessTexture->getMemberValue(L"index").getInt32();
					if (!textures || index < 0 || index >= (int32_t)textures->size())
						return nullptr;

					auto texture = textures->get(index).getObject< json::JsonObject >();
					int32_t source = texture->getMemberValue(L"source").getInt32();
					if (!images || source < 0 || source >= (int32_t)images->size())
						return nullptr;

					auto image = images->get(source).getObject< json::JsonObject >();

					std::wstring name;
					if (image->getMember(L"uri") != nullptr)
						name = image->getMemberValue(L"uri").getWideString();
					else if (image->getMember(L"name") != nullptr)
						name = image->getMemberValue(L"name").getWideString();
					if (name.empty())
						return nullptr;

					mt.setMetalnessMap(Material::Map(name, L"UV0", true));
					mt.setRoughnessMap(Material::Map(name, L"UV0", true));
				}

				float metallicFactor = pbrMetallicRoughness->getMember(L"metallicFactor") != nullptr ? pbrMetallicRoughness->getMemberValue(L"metallicFactor").getFloat() : 1.0f;
				mt.setMetalness(metallicFactor);

				float roughnessFactor = pbrMetallicRoughness->getMemberValue(L"roughnessFactor").getFloat();
				mt.setRoughness(roughnessFactor);

				auto baseColorFactor = pbrMetallicRoughness->getMemberValue(L"baseColorFactor").getObject< json::JsonArray >();
				if (baseColorFactor != nullptr && baseColorFactor->size() >= 4)
				{
					mt.setColor(Color4f(
						baseColorFactor->get(0).getFloat(),
						baseColorFactor->get(1).getFloat(),
						baseColorFactor->get(2).getFloat(),
						baseColorFactor->get(3).getFloat()
					));
				}
			}

			md->addMaterial(mt);
		}
	}

	// Parse geometry.
	auto nodes = docobj->getMemberValue(L"nodes").getObject< json::JsonArray >();
	auto meshes = docobj->getMemberValue(L"meshes").getObject< json::JsonArray >();
	if (nodes && meshes)
	{
		const Matrix44 Tpost = scale(1.0f, 1.0f, -1.0f);

		for (uint32_t i = 0; i < nodes->size(); ++i)
		{
			auto node = nodes->get(i).getObject< json::JsonObject >();
			if (!node)
				return nullptr;

			if (node->getMember(L"mesh") == nullptr)
				continue;

			const int32_t meshIndex = node->getMemberValue(L"mesh").getInt32();
			if (meshIndex < 0 || meshIndex >= (int32_t)meshes->size())
				continue;

			auto mesh = meshes->get(meshIndex).getObject< json::JsonObject >();
			if (!mesh)
				return nullptr;

			Matrix44 Tnode = Tpost * parseTransform(node);

			auto primitives = mesh->getMemberValue(L"primitives").getObject< json::JsonArray >();
			if (!primitives)
				return nullptr;

			for (uint32_t j = 0; j < primitives->size(); ++j)
			{
				auto prim = primitives->get(j).getObject< json::JsonObject >();
				if (!prim)
					return nullptr;

				int32_t material = prim->getMemberValue(L"material").getInt32();
				int32_t indices = prim->getMemberValue(L"indices").getInt32();

				auto attributes = prim->getMemberValue(L"attributes").getObject< json::JsonObject >();
				if (!attributes)
					return nullptr;

				int32_t position = attributes->getMemberValue(L"POSITION").getInt32();
				int32_t normal = attributes->getMemberValue(L"NORMAL").getInt32();
				int32_t texCoord0 = attributes->getMemberValue(L"TEXCOORD_0").getInt32();

				AlignedVector< int32_t > dataIndices;
				AlignedVector< Vector4 > dataPositions;
				AlignedVector< Vector4 > dataNormals;
				AlignedVector< Vector4 > dataTexCoord0s;

				decodeAsIndices(
					indices,
					accessors,
					bufferViews,
					bufferStreams,
					dataIndices
				);
				decodeAsVectors(
					position,
					accessors,
					bufferViews,
					bufferStreams,
					dataPositions
				);
				decodeAsVectors(
					normal,
					accessors,
					bufferViews,
					bufferStreams,
					dataNormals
				);
				decodeAsVectors(
					texCoord0,
					accessors,
					bufferViews,
					bufferStreams,
					dataTexCoord0s
				);

				const uint32_t vertexBase = md->getVertexCount();

				for (uint32_t k = 0; k < dataPositions.size(); ++k)
				{
					Vertex vx;
					vx.setPosition(md->addPosition(
						Tnode * dataPositions[k].xyz1()
					));

					if (dataNormals.size() == dataPositions.size())
						vx.setNormal(md->addUniqueNormal(
							Tnode * dataNormals[k].xyz0()
						));

					if (dataTexCoord0s.size() == dataPositions.size())
						vx.setTexCoord(0, md->addUniqueTexCoord(
							Vector2(
								dataTexCoord0s[k].x(),
								dataTexCoord0s[k].y()
							)
						));

					md->addVertex(vx);
				}

				for (uint32_t k = 0; k < dataIndices.size(); k += 3)
				{
					Polygon pol;
					pol.setMaterial(material);
					pol.addVertex(vertexBase + dataIndices[k + 0]);
					pol.addVertex(vertexBase + dataIndices[k + 1]);
					pol.addVertex(vertexBase + dataIndices[k + 2]);
					md->addPolygon(pol);
				}
			}
		}
	}

	return md;
}

bool ModelFormatGltf::write(const Path& filePath, const Model* model) const
{
	return false;
}

	}
}
