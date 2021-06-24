#pragma once

#include <map>
#include "Core/Guid.h"
#include "Core/Math/Vector4.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

/*! Mesh asset.
 * \ingroup Mesh
 */
class T_DLLCLASS MeshAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	enum MeshType
	{
		MtBlend = 0,
		MtIndoor = 1,
		MtInstance = 2,
		MtLod = 3,
		MtPartition = 4,
		MtProc = 5,
		MtSkinned = 6,
		MtStatic = 7,
		MtStream = 8
	};

	virtual void serialize(ISerializer& s) override final;

	/*! Set import filter. */
	void setImportFilter(const std::wstring& importFilter) { m_importFilter = importFilter; }

	/*! Get import filter. */
	const std::wstring& getImportFilter() const { return m_importFilter; }

	/*! Set type of runtime mesh. */
	void setMeshType(MeshType meshType) { m_meshType = meshType; }

	/*! Get type of runtime mesh. */
	MeshType getMeshType() const { return m_meshType; }

	/*! Set material shader templates. */
	void setMaterialTemplates(const std::map< std::wstring, Guid >& materialTemplates) { m_materialTemplates = materialTemplates; }

	/*! Get material shader templates. */
	const std::map< std::wstring, Guid >& getMaterialTemplates() const { return m_materialTemplates; }

	/*! Set explicit material shaders. */
	void setMaterialShaders(const std::map< std::wstring, Guid >& materialShaders) { m_materialShaders = materialShaders; }

	/*! Get explicit material shaders. */
	const std::map< std::wstring, Guid >& getMaterialShaders() const { return m_materialShaders; }

	/*! Set material textures. */
	void setMaterialTextures(const std::map< std::wstring, Guid >& materialTextures) { m_materialTextures = materialTextures; }

	/*! Get material textures. */
	const std::map< std::wstring, Guid >& getMaterialTextures() const { return m_materialTextures; }

	/*! Set texture set. */
	void setTextureSet(const Guid& textureSet) { m_textureSet = textureSet; }

	/*! Get texture set. */
	const Guid& getTextureSet() const { return m_textureSet; }

	/*! Set scale factor. */
	void setScaleFactor(float scaleFactor) { m_scaleFactor = scaleFactor; }

	/*! Get scale factor. */
	float getScaleFactor() const { return m_scaleFactor; }

	/*! Set offset. */
	void setOffset(const Vector4& offset) { m_offset = offset; }

	/*! Get offset. */
	const Vector4& getOffset() const { return m_offset; }

	/*! Set if new normals should be calculated. */
	void setRenormalize(bool renormalize) { m_renormalize = renormalize; }

	/*! Check if new normals should be calculated. */
	bool getRenormalize() const { return m_renormalize; }

	/*! Set if model should be centered around origo before converted. */
	void setCenter(bool center) { m_center = center; }

	/*! Check if model should be centered around origo. */
	bool getCenter() const { return m_center; }

	/*! Set number of manual lod steps. */
	void setLodSteps(int32_t lodSteps) { m_lodSteps = lodSteps; }

	/*! Get number of manual lod steps. */
	int32_t getLodSteps() const { return m_lodSteps; }

	/*! Set distance when lowest lod should be used. */
	void setLodMaxDistance(float lodMaxDistance) { m_lodMaxDistance = lodMaxDistance; }

	/*! Get distance when lowest lod should be used. */
	float getLodMaxDistance() const { return m_lodMaxDistance; }

	/*! Set distance to when mesh should no longer be rendererd. */
	void setLodCullDistance(float lodCullDistance) { m_lodCullDistance = lodCullDistance; }

	/*! Get distance to when mesh should no longer be rendererd. */
	float getLodCullDistance() const { return m_lodCullDistance; }

	/*! */
	void setPreviewAngle(float previewAngle) { m_previewAngle = previewAngle; }

	/*! */
	float getPreviewAngle() const { return m_previewAngle; }

private:
	std::wstring m_importFilter;
	MeshType m_meshType = MtStatic;
	std::map< std::wstring, Guid > m_materialTemplates;
	std::map< std::wstring, Guid > m_materialShaders;
	std::map< std::wstring, Guid > m_materialTextures;
	Guid m_textureSet;
	float m_scaleFactor = 1.0f;
	Vector4 m_offset = Vector4::zero();
	bool m_renormalize = false;
	bool m_center = false;
	int32_t m_lodSteps = 8;
	float m_lodMaxDistance = 100.0f;
	float m_lodCullDistance = 200.0f;
	float m_previewAngle = 0.0f;
};

	}
}

