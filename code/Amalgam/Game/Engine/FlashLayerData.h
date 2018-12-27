/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_FlashLayerData_H
#define traktor_amalgam_FlashLayerData_H

#include <map>
#include "Amalgam/Game/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Movie;

	}

	namespace render
	{

class ImageProcessSettings;

	}

	namespace amalgam
	{

/*! \brief Stage Flash layer persistent data.
 * \ingroup Amalgam
 */
class T_DLLCLASS FlashLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	FlashLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StagePipeline;

	resource::Id< flash::Movie > m_movie;
	std::map< std::wstring, resource::Id< flash::Movie > > m_externalMovies;
	resource::Id< render::ImageProcessSettings > m_imageProcess;
	bool m_clearBackground;
	bool m_enableShapeCache;
	bool m_enableDirtyRegions;
	bool m_enableSound;
	uint32_t m_contextSize;
};

	}
}

#endif	// traktor_amalgam_FlashLayerData_H
