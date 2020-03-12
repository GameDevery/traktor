#pragma once

#include "Core/Math/Color4f.h"
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"
#include "Scene/Editor/IModifier.h"
#include "Terrain/TerrainComponent.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace drawing
	{

class Image;

	}

	namespace hf
	{

class Heightfield;
class HeightfieldAsset;

	}

	namespace render
	{

class ISimpleTexture;

	}

	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

	}

	namespace terrain
	{

class IBrush;
class IFallOff;
class TerrainComponentData;

class TerrainEditModifier : public scene::IModifier
{
	T_RTTI_CLASS;

public:
	TerrainEditModifier(scene::SceneEditorContext* context);

	virtual void selectionChanged() override final;

	virtual bool cursorMoved(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual bool begin(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		int32_t mouseButton
	) override final;

	virtual void apply(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta
	) override final;

	virtual void end(const scene::TransformChain& transformChain) override final;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const override final;

	void setBrush(const TypeInfo& brushType);

	void setFallOff(const std::wstring& fallOff);

	void setStrength(float strength);

	void setColor(const Color4f& color);

	void setMaterial(int32_t material);

	void setAttribute(int32_t attribute);

	void setVisualizeMode(TerrainComponent::VisualizeMode visualizeMode);

	void setFallOffImage(drawing::Image* fallOffImage);

private:
	scene::SceneEditorContext* m_context;
	Ref< terrain::TerrainComponent > m_terrainComponent;
	Ref< terrain::TerrainComponentData > m_terrainComponentData;
	Ref< db::Instance > m_terrainInstance;
	Ref< db::Instance > m_heightfieldInstance;
	Ref< hf::HeightfieldAsset > m_heightfieldAsset;
	resource::Proxy< hf::Heightfield > m_heightfield;
	Ref< drawing::Image > m_splatImage;
	Ref< render::ISimpleTexture > m_splatMap;
	Ref< drawing::Image > m_colorImage;
	Ref< drawing::Image > m_colorImageLowPrecision;
	Ref< render::ISimpleTexture > m_colorMap;
	AutoArrayPtr< uint8_t > m_normalData;
	Ref< render::ISimpleTexture > m_normalMap;
	AutoArrayPtr< uint8_t > m_cutData;
	Ref< render::ISimpleTexture > m_cutMap;
	AutoArrayPtr< uint8_t > m_attributeData;
	Ref< render::ISimpleTexture > m_attributeMap;
	Ref< IBrush > m_brush;
	uint32_t m_brushMode;
	Ref< IFallOff > m_fallOff;
	Ref< drawing::Image > m_fallOffImage;
	float m_strength;
	Color4f m_color;
	int32_t m_material;
	int32_t m_attribute;
	TerrainComponent::VisualizeMode m_visualizeMode;
	Vector4 m_center;
	uint32_t m_updateRegion[4];
	bool m_applied;

	bool begin(bool inverted);

	void apply(const Vector4& center);

	void end();

	void applySpline(bool alignToGround);
};

	}
}

