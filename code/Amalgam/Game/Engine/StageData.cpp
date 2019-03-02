#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/LayerData.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Database.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Resource/ResourceBundle.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.StageData", 10, StageData, ISerializable)

StageData::StageData()
:	m_fadeRate(1.5f)
{
}

Ref< Stage > StageData::createInstance(IEnvironment* environment, const Object* params) const
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< IRuntimeClass > clazz;
	resource::Proxy< render::Shader > shaderFade;

#if !defined(_DEBUG)
	// Load resource bundle.
	if (m_resourceBundle.isNotNull())
	{
		bool skipPreload = environment->getSettings()->getProperty< bool >(L"Amalgam.SkipPreloadResources", false);
		if (!skipPreload)
		{
			uint32_t preloadLimit = environment->getSettings()->getProperty< int32_t >(L"Amalgam.SkipPreloadLimit", 768) * 1024 * 1024;

			// Get amount of dedicated video memory; we cannot preload if too little amount of memory available or unknown vendor.
			render::RenderSystemInformation rsi;
			renderSystem->getInformation(rsi);
			if (
				(rsi.vendor == render::AvtNVidia || rsi.vendor == render::AvtAMD) &&
				rsi.dedicatedMemoryTotal >= preloadLimit
			)
			{
				Ref< const resource::ResourceBundle > resourceBundle = environment->getDatabase()->getObjectReadOnly< resource::ResourceBundle >(m_resourceBundle);
				if (resourceBundle)
				{
					log::info << L"Preloading bundle \"" << m_resourceBundle.format() << L"\"..." << Endl;
					resourceManager->load(resourceBundle);
				}
			}
			else
			{
				if (rsi.dedicatedMemoryTotal < preloadLimit)
					log::warning << L"Pre-loading of resources skipped due to limited graphics adapter (" << (rsi.dedicatedMemoryTotal / 1024) << L" < " << (preloadLimit / 1024) << L" KiB)." << Endl;
				else
					log::warning << L"Pre-loading of resources skipped due to unknown graphics adapter, only permitted for NVidia or AMD adapters." << Endl;
			}
		}
		else
			log::warning << L"Pre-loading of resources ignored" << Endl;
	}
#endif

	// Bind proxies to resource manager.
	if (m_class && !resourceManager->bind(m_class, clazz))
		return 0;
	if (m_shaderFade && !resourceManager->bind(m_shaderFade, shaderFade))
		return 0;

	// Create layers.
	Ref< Stage > stage = new Stage(m_name, environment, clazz, shaderFade, m_fadeRate, m_transitions, params);
	for (RefArray< LayerData >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Ref< Layer > layer = (*i)->createInstance(stage, environment);
		if (!layer)
			return 0;

		stage->addLayer(layer);
	}

	return stage;
}

void StageData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion() >= 10);

	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< Guid >(L"inherit", m_inherit, AttributeType(type_of< StageData >()));
	s >> MemberRefArray< LayerData >(L"layers", m_layers);
	s >> resource::Member< IRuntimeClass >(L"class", m_class);
	s >> resource::Member< render::Shader >(L"shaderFade", m_shaderFade);
	s >> Member< float >(L"fadeRate", m_fadeRate, AttributeRange(0.1f));
	s >> MemberStlMap< std::wstring, Guid >(L"transitions", m_transitions);
	s >> Member< Guid >(L"resourceBundle", m_resourceBundle, AttributeType(type_of< resource::ResourceBundle >()));
	s >> MemberRef< const PropertyGroup >(L"properties", m_properties);
}

	}
}
