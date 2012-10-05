#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Resound/TriggerGrainData.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankPipeline.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

void buildGrainDependencies(editor::IPipelineDepends* pipelineDepends, const IGrainData* grain)
{
	if (const EnvelopeGrainData* envelopeGrain = dynamic_type_cast< const EnvelopeGrainData* >(grain))
	{
		const std::vector< EnvelopeGrainData::GrainData >& grains = envelopeGrain->getGrains();
		for (std::vector< EnvelopeGrainData::GrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, i->grain);
	}

	if (const RepeatGrainData* repeatGrain = dynamic_type_cast< const RepeatGrainData* >(grain))
		buildGrainDependencies(pipelineDepends, repeatGrain->getGrain());

	if (const RandomGrainData* randomGrain = dynamic_type_cast< const RandomGrainData* >(grain))
	{
		const RefArray< IGrainData >& grains = randomGrain->getGrains();
		for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const SequenceGrainData* sequenceGrain = dynamic_type_cast< const SequenceGrainData* >(grain))
	{
		const RefArray< IGrainData >& grains = sequenceGrain->getGrains();
		for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const TriggerGrainData* triggerGrain = dynamic_type_cast< const TriggerGrainData* >(grain))
		buildGrainDependencies(pipelineDepends, triggerGrain->getGrain());

	if (const PlayGrainData* playGrain = dynamic_type_cast< const PlayGrainData* >(grain))
		pipelineDepends->addDependency(playGrain->getSound(), editor::PdfBuild);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankPipeline", 5, BankPipeline, editor::DefaultPipeline)

TypeInfoSet BankPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BankAsset >());
	return typeSet;
}

bool BankPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset*, false >(sourceAsset);
	
	const RefArray< IGrainData >& grains = bankAsset->getGrains();
	for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
		buildGrainDependencies(pipelineDepends, *i);

	Ref< const SoundCategory > category = pipelineDepends->getObjectReadOnly< SoundCategory >(bankAsset->m_category);
	if (category)
		pipelineDepends->addDependency(bankAsset->m_category, editor::PdfUse);

	while (category)
	{
		Ref< const SoundCategory > parent = pipelineDepends->getObjectReadOnly< SoundCategory >(category->getParent());
		if (parent)
			pipelineDepends->addDependency(category->getParent(), editor::PdfUse);

		category = parent;
	}

	return true;
}

bool BankPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset*, false >(sourceAsset);

	float volume = 1.0f;
	float presence = bankAsset->m_presence;
	float presenceRate = bankAsset->m_presenceRate;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(bankAsset->m_category);
	while (category)
	{
		volume *= category->getVolume();

		if (presence <= FUZZY_EPSILON)
		{
			presence = category->getPresence();
			presenceRate = category->getPresenceRate();
		}

		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	log::info << L"Category volume " << int32_t(volume * 100.0f) << L"%" << Endl;
	log::info << L"Category presence " << presence << L", rate " << int32_t(presenceRate * 100.0f) << L" d%" << Endl;

	Ref< BankResource > bankResource = new BankResource(
		bankAsset->m_grains,
		volume,
		presence,
		presenceRate
	);

	return editor::DefaultPipeline::buildOutput(
		pipelineBuilder,
		bankResource,
		sourceAssetHash,
		buildParams,
		outputPath,
		outputGuid,
		reason
	);
}

	}
}
