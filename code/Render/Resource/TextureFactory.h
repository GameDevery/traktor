/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TextureFactory_H
#define traktor_render_TextureFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

/*! \brief Texture resource factory.
 * \ingroup Render
 */
class T_DLLCLASS TextureFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	TextureFactory(IRenderSystem* renderSystem, int32_t skipMips);

	void setSkipMips(int32_t skipMips);

	int32_t getSkipMips() const;

	virtual const TypeInfoSet getResourceTypes() const override final;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

	virtual bool isCacheable(const TypeInfo& productType) const override final;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;

private:
	Ref< IRenderSystem > m_renderSystem;
	int32_t m_skipMips;
};

	}
}

#endif	// traktor_render_TextureFactory_H
