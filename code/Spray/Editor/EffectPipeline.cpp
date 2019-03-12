#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Spray/EffectData.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/TrailData.h"
#include "Spray/Editor/EffectPipeline.h"
#include "Spray/Sources/PointSetSourceData.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

void buildEffectDependencies(editor::IPipelineDepends* pipelineDepends, const EffectData* effectData)
{
	const RefArray< EffectLayerData >& layers = effectData->getLayers();
	for (RefArray< EffectLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		const EmitterData* emitter = (*i)->getEmitter();
		if (emitter)
		{
			pipelineDepends->addDependency(emitter->getShader(), editor::PdfBuild | editor::PdfResource);
			pipelineDepends->addDependency(emitter->getMesh(), editor::PdfBuild | editor::PdfResource);

			if (emitter->getEffect())
				buildEffectDependencies(pipelineDepends, emitter->getEffect());

			const PointSetSourceData* pointSetSource = dynamic_type_cast< const PointSetSourceData* >(emitter->getSource());
			if (pointSetSource)
				pipelineDepends->addDependency(pointSetSource->getPointSet(), editor::PdfBuild | editor::PdfResource);
		}

		const TrailData* trail = (*i)->getTrail();
		if (trail)
			pipelineDepends->addDependency(trail->getShader(), editor::PdfBuild | editor::PdfResource);

		const SequenceData* sequence = (*i)->getSequence();
		if (sequence)
		{
			for (std::vector< SequenceData::Key >::const_iterator i = sequence->getKeys().begin(); i != sequence->getKeys().end(); ++i)
				pipelineDepends->addDependency(i->event);
		}

		pipelineDepends->addDependency((*i)->getTriggerEnable());
		pipelineDepends->addDependency((*i)->getTriggerDisable());
	}
}

bool effectLayerPred(EffectLayerData* layerData)
{
	if (!layerData)
		return true;

	bool haveEmitter = false;
	if (layerData->getEmitter() != 0)
	{
		if (
			layerData->getEmitter()->getSource() != 0 &&
			(
				layerData->getEmitter()->getShader() ||
				layerData->getEmitter()->getMesh() ||
				layerData->getEmitter()->getEffect()
			)
		)
			haveEmitter = true;
	}

	bool haveTrail = false;
	if (layerData->getTrail() != 0)
	{
		if (layerData->getTrail()->getShader())
			haveTrail = true;
	}

	bool haveSequence = false;
	if (layerData->getSequence() != 0)
	{
		if (!layerData->getSequence()->getKeys().empty())
			haveSequence = true;
	}

	bool haveTrigger = false;
	if (layerData->getTriggerEnable() != 0 || layerData->getTriggerDisable() != 0)
		haveTrigger = true;

	if (haveEmitter || haveTrail || haveSequence || haveTrigger)
		return false;

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectPipeline", 4, EffectPipeline, editor::IPipeline)

bool EffectPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EffectPipeline::destroy()
{
}

TypeInfoSet EffectPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EffectData >();
	return typeSet;
}

bool EffectPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const EffectData* effectData = checked_type_cast< const EffectData* >(sourceAsset);
	buildEffectDependencies(pipelineDepends, effectData);
	return true;
}

bool EffectPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const EffectData* effectData = checked_type_cast< const EffectData* >(sourceAsset);

	RefArray< EffectLayerData > effectLayers = effectData->getLayers();

	RefArray< EffectLayerData >::iterator i = std::remove_if(effectLayers.begin(), effectLayers.end(), effectLayerPred);
	effectLayers.erase(i, effectLayers.end());

	Ref< EffectData > outEffectData = new EffectData(
		effectData->getDuration(),
		effectData->getLoopStart(),
		effectData->getLoopEnd(),
		effectLayers
	);

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
		return false;

	instance->setObject(outEffectData);

	return instance->commit();
}

Ref< ISerializable > EffectPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
