#include <limits>
#include "Core/Log/Log.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineSettings.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineFactory", PipelineFactory, Object)

PipelineFactory::PipelineFactory(const Settings* settings)
{
	std::vector< const TypeInfo* > pipelineTypes;
	type_of< IPipeline >().findAllOf(pipelineTypes, false);

	for (std::vector< const TypeInfo* >::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< IPipeline > pipeline = dynamic_type_cast< IPipeline* >((*i)->createInstance());
		if (pipeline)
		{
			PipelineSettings pipelineSettings(settings);
			if (pipeline->create(&pipelineSettings))
				m_pipelines.push_back(std::make_pair(
				pipeline,
				pipelineSettings.getHash()
			));
			else
				log::error << L"Failed to create pipeline \"" << type_name(pipeline) << L"\"" << Endl;
		}
	}
}

PipelineFactory::~PipelineFactory()
{
	for (std::vector< std::pair< Ref< IPipeline >, uint32_t > >::iterator i = m_pipelines.begin(); i != m_pipelines.end(); ++i)
		i->first->destroy();
}

bool PipelineFactory::findPipeline(const TypeInfo& sourceType, Ref< IPipeline >& outPipeline, uint32_t& outPipelineHash) const
{
	uint32_t best = std::numeric_limits< uint32_t >::max();
	for (std::vector< std::pair< Ref< IPipeline >, uint32_t > >::const_iterator i = m_pipelines.begin(); i != m_pipelines.end(); ++i)
	{
		TypeInfoSet typeSet = i->first->getAssetTypes();
		for (TypeInfoSet::iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			uint32_t distance = 0;

			// Calculate distance in type hierarchy.
			const TypeInfo* type = &sourceType;
			while (type)
			{
				if (type == *j)
					break;

				++distance;
				type = type->getSuper();
			}

			// Keep closest matching type.
			if (type && distance < best)
			{
				outPipeline = i->first;
				outPipelineHash = i->second;
				if ((best = distance) == 0)
					break;
			}
		}
	}
	return bool(outPipeline != 0);
}
	}
}
