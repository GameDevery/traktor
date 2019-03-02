#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/FlashLayer.h"
#include "Amalgam/Game/Engine/FlashLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/Movie.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.FlashLayerData", 0, FlashLayerData, LayerData)

FlashLayerData::FlashLayerData()
:	m_clearBackground(false)
,	m_enableShapeCache(false)
,	m_enableDirtyRegions(false)
,	m_enableSound(true)
,	m_contextSize(1)
{
}

Ref< Layer > FlashLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< flash::Movie > movie;
	resource::Proxy< render::ImageProcessSettings > imageProcess;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_movie, movie))
		return 0;

	// Bind external movies.
	std::map< std::wstring, resource::Proxy< flash::Movie > > externalMovies;
	for (std::map< std::wstring, resource::Id< flash::Movie > >::const_iterator i = m_externalMovies.begin(); i != m_externalMovies.end(); ++i)
	{
		if (!resourceManager->bind(i->second, externalMovies[i->first]))
			return 0;
	}

	// Bind optional post processing.
	if (m_imageProcess)
	{
		if (!resourceManager->bind(m_imageProcess, imageProcess))
			return 0;
	}

	// Create layer instance.
	return new FlashLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		movie,
		externalMovies,
		imageProcess,
		m_clearBackground,
		m_enableShapeCache,
		m_enableDirtyRegions,
		m_enableSound,
		m_contextSize
	);
}

void FlashLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< flash::Movie >(L"movie", m_movie);

	s >> MemberStlMap<
		std::wstring,
		resource::Id< flash::Movie >,
		MemberStlPair<
			std::wstring,
			resource::Id< flash::Movie >,
			Member< std::wstring >,
			resource::Member< flash::Movie >
		>
	>(L"externalMovies", m_externalMovies);

	s >> resource::Member< render::ImageProcessSettings >(L"imageProcess", m_imageProcess);
	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);
	s >> Member< bool >(L"enableShapeCache", m_enableShapeCache);
	s >> Member< bool >(L"enableDirtyRegions", m_enableDirtyRegions);
	s >> Member< uint32_t >(L"contextSize", m_contextSize);
}

	}
}
