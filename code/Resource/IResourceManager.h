#pragma once

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Resource/Id.h"
#include "Resource/IdProxy.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceFactory;
class ResourceBundle;
class ResourceHandle;

/*! \brief Resource manager statistics.
 * \ingroup Resource
 */
struct ResourceManagerStatistics
{
	uint32_t residentCount;		//!< Number of resident resources.
	uint32_t exclusiveCount;	//!< Number of exclusive (non-shareable) resources.

	ResourceManagerStatistics()
	:	residentCount(0)
	,	exclusiveCount(0)
	{
	}
};

/*! \brief Resource manager interface.
 * \ingroup Resource
 */
class T_DLLCLASS IResourceManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! \brief Add resource factory to manager.
	 *
	 * \param factory Resource factory.
	 */
	virtual void addFactory(const IResourceFactory* factory) = 0;

	/*! \brief Remove resource factory from manager.
	 *
	 * \param factory Resource factory.
	 */
	virtual void removeFactory(const IResourceFactory* factory) = 0;

	/*! \brief Remove all resource factories. */
	virtual void removeAllFactories() = 0;

	/*! \brief Load all resources in bundle.
	 *
	 * \param bundle Resource bundle.
	 * \return True if all resources loaded successfully.
	 */
	virtual bool load(const ResourceBundle* bundle) = 0;

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param productType Type of product.
	 * \param guid Resource identifier.
	 * \return Resource handle.
	 */
	virtual Ref< ResourceHandle > bind(const TypeInfo& productType, const Guid& guid) = 0;

	/*! \brief Reload resource.
	 *
	 * \param guid Resource identifier.
	 * \param flushedOnly Reload flushed resources only.
	 */
	virtual void reload(const Guid& guid, bool flushedOnly) = 0;

	/*! \brief Reload all resources of given type.
	 *
	 * \param productType Type of product.
	 * \param flushedOnly Reload flushed resources only.
	 */
	virtual void reload(const TypeInfo& productType, bool flushedOnly) = 0;

	/*! \brief Unload all resources of given type.
	 *
	 * \param productType Type of product.
	 */
	virtual void unload(const TypeInfo& productType) = 0;

	/*! \brief Unload externally unused, resident, resources.
	 *
	 * Call this when unused resources which are resident can
	 * be unloaded.
	 */
	virtual void unloadUnusedResident() = 0;

	/*! \brief Get statistics. */
	virtual void getStatistics(ResourceManagerStatistics& outStatistics) const = 0;

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param id Resource identifier.
	 * \return Resource proxy.
	 */
	template <
		typename ResourceType,
		typename ProductType
	>
	bool bind(const Id< ResourceType >& id, Proxy< ProductType >& outProxy)
	{
		Ref< ResourceHandle > handle = bind(type_of< ProductType >(), id);
		if (!handle)
			return false;

		outProxy = Proxy< ProductType >(handle);
		return bool(handle->get() != 0);
	}

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param proxy Resource identifier proxy.
	 */
	template <
		typename ProductType
	>
	bool bind(IdProxy< ProductType >& outProxy)
	{
		Ref< ResourceHandle > handle = bind(type_of< ProductType >(), outProxy.getId());
		if (!handle)
			return false;

		outProxy.replace(handle);
		return bool(handle->get() != 0);
	}
};

	}
}

