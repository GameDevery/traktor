#pragma once

#include <string>
#include "Core/Config.h"
#include "Core/Guid.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace model
	{

/*! Material descriptor.
 * \ingroup Model
 */
class T_DLLCLASS Material : public PropertyGroup
{
	T_RTTI_CLASS;

public:
	enum BlendOperator
	{
		BoDecal = 0,
		BoAdd = 1,
		BoMultiply = 2,
		BoAlpha = 3,
		BoAlphaTest = 4
	};

	struct Map
	{
		std::wstring name;
		std::wstring channel;
		bool anisotropic;
		Guid texture;
		Ref< drawing::Image > image;	//!< Not serializable.

		Map()
		:	name(L"")
		,	channel(L"")
		,	anisotropic(false)
		{
		}

		Map(const std::wstring& name_, const std::wstring& channel_, bool anisotropic_ = true, const Guid& texture_ = Guid())
		:	name(name_)
		,	channel(channel_)
		,	anisotropic(anisotropic_)
		,	texture(texture_)
		{
		}

		void serialize(ISerializer& s);
	};

	Material();

	explicit Material(const std::wstring& name);

	explicit Material(const std::wstring& name, const Color4f& color);

	void setName(const std::wstring& name);

	const std::wstring& getName() const { return m_name; }

	void setDiffuseMap(const Map& diffuseMap);

	const Map& getDiffuseMap() const { return m_diffuseMap; }

	void setSpecularMap(const Map& specularMap);

	const Map& getSpecularMap() const { return m_specularMap; }

	void setRoughnessMap(const Map& roughnessMap);

	const Map& getRoughnessMap() const { return m_roughnessMap; }

	void setMetalnessMap(const Map& metalnessMap);

	const Map& getMetalnessMap() const { return m_metalnessMap; }

	void setTransparencyMap(const Map& transparencyMap);

	const Map& getTransparencyMap() const { return m_transparencyMap; }

	void setEmissiveMap(const Map& emissiveMap);

	const Map& getEmissiveMap() const { return m_emissiveMap; }

	void setReflectiveMap(const Map& reflectiveMap);

	const Map& getReflectiveMap() const { return m_reflectiveMap; }

	void setNormalMap(const Map& normalMap);

	const Map& getNormalMap() const { return m_normalMap; }

	void setLightMap(const Map& lightMap);

	const Map& getLightMap() const { return m_lightMap; }

	void setColor(const Color4f& color);

	const Color4f& getColor() const { return m_color; }

	void setDiffuseTerm(float diffuseTerm);

	float getDiffuseTerm() const { return m_diffuseTerm; }

	void setSpecularTerm(float specularTerm);

	float getSpecularTerm() const { return m_specularTerm; }

	void setRoughness(float roughness);

	float getRoughness() const { return m_roughness; }

	void setMetalness(float metalness);

	float getMetalness() const { return m_metalness; }

	void setTransparency(float transparency);

	float getTransparency() const { return m_transparency; }

	void setEmissive(float emissive);

	float getEmissive() const { return m_emissive; }

	void setReflective(float reflective);

	float getReflective() const { return m_reflective; }

	void setRimLightIntensity(float rimLightIntensity);

	float getRimLightIntensity() const { return m_rimLightIntensity; }

	void setBlendOperator(BlendOperator blendOperator);

	BlendOperator getBlendOperator() const { return m_blendOperator; }

	void setDoubleSided(bool doubleSided);

	bool isDoubleSided() const { return m_doubleSided; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	Map m_diffuseMap;
	Map m_specularMap;
	Map m_roughnessMap;
	Map m_metalnessMap;
	Map m_transparencyMap;
	Map m_emissiveMap;
	Map m_reflectiveMap;
	Map m_normalMap;
	Map m_lightMap;
	Color4f m_color;
	float m_diffuseTerm;
	float m_specularTerm;
	float m_roughness;
	float m_metalness;
	float m_transparency;
	float m_emissive;
	float m_reflective;
	float m_rimLightIntensity;
	BlendOperator m_blendOperator;
	bool m_doubleSided;
};

	}
}

