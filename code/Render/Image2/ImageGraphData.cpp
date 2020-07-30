#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/IImageStepData.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassOpData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/ImageTextureData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphData", 1, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
    Ref< ImageGraph > instance = new ImageGraph(m_name);

    for (auto textureData : m_textures)
    {
        Ref< const ImageTexture > texture = textureData->createInstance(resourceManager);
        if (!texture)
            return nullptr;
        instance->m_textures.push_back(texture);
    }

    for (auto targetSetData : m_targetSets)
    {
        Ref< const ImageTargetSet > targetSet = targetSetData->createInstance();
        if (!targetSet)
            return nullptr;
        instance->m_targetSets.push_back(targetSet);
    }

    for (auto stepData : m_steps)
    {
        Ref< const IImageStep > step = stepData->createInstance(resourceManager, renderSystem);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    for (auto opd : m_ops)
    {
        Ref< const ImagePassOp > op = opd->createInstance(resourceManager, renderSystem);
        if (!op)
            return nullptr;
        instance->m_ops.push_back(op);
    }

    return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
    T_ASSERT(s.getVersion< ImageGraphData >() >= 1);

    s >> Member< std::wstring >(L"name", m_name);
    s >> MemberRefArray< ImageTextureData >(L"textures", m_textures);
    s >> MemberRefArray< ImageTargetSetData >(L"targetSets", m_targetSets);
    s >> MemberRefArray< IImageStepData >(L"steps", m_steps);
    s >> MemberRefArray< ImagePassOpData >(L"ops", m_ops);
}

    }
}