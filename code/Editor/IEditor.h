#ifndef traktor_editor_IEditor_H
#define traktor_editor_IEditor_H

#include <set>
#include <vector>
#include "Core/Config.h"
#include "Core/RefArray.h"
#include "Core/Rtti/ITypedObject.h"

namespace traktor
{

class Guid;
class ISerializable;
class Object;
class Settings;
class TypeInfo;

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class IEditorPage;
class IBrowseFilter;
class PipelineDependency;

/*! \brief Editor base interface.
 * \ingroup Editor
 *
 * This is the interface by which other modules
 * communicate with the Editor application.
 */
class T_NOVTABLE IEditor
{
public:
	/*! \brief Get editor settings. */
	virtual Ref< Settings > getSettings() = 0;

	/*! \brief Get source asset database. */
	virtual Ref< db::Database > getSourceDatabase() = 0;

	/*! \brief Get output database. */
	virtual Ref< db::Database > getOutputDatabase() = 0;

	/*! \brief Browse for rtti type. */
	virtual const TypeInfo* browseType(const TypeInfo* base = 0) = 0;

	/*! \brief Browse database instance. */
	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter = 0) = 0;

	/*! \brief Open instance in appropriate editor. */
	virtual bool openEditor(db::Instance* instance) = 0;

	/*! \brief Get active editor. */
	virtual Ref< IEditorPage > getActiveEditorPage() = 0;

	/*! \brief Set active editor. */
	virtual void setActiveEditorPage(IEditorPage* editorPage) = 0;

	/*! \brief Build assets. */
	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) = 0;

	/*! \brief Build asset. */
	virtual void buildAsset(const Guid& assetGuid, bool rebuild) = 0;

	/*! \brief Build assets. */
	virtual void buildAssets(bool rebuild) = 0;

	/*! \brief Build asset dependencies.
	 *
	 * \param asset Source asset.
	 * \param recursionDepth Max dependency recursion depth.
	 * \param outDependencies Set of dependency asset guid;s.
	 * \return True if successful.
	 */
	virtual bool buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth, RefArray< PipelineDependency >& outDependencies) = 0;

	/*! \brief Set object in object store.
	 *
	 * \param name Name of object.
	 * \param object Object.
	 */
	virtual void setStoreObject(const std::wstring& name, Object* object) = 0;

	/*! \brief Get object from object store.
	 *
	 * \param name Name of object.
	 * \return Object, null if no such named object exists in store.
	 */
	virtual Object* getStoreObject(const std::wstring& name) const = 0;

	/*! \brief Get object from object store.
	 *
	 * \param name Name of object.
	 * \return Object, null if no such named object exists in store.
	 */
	template < typename ObjectType >
	ObjectType* getStoreObject(const std::wstring& name) const
	{
		return dynamic_type_cast< ObjectType* >(getStoreObject(name));
	}
};

	}
}

#endif	// traktor_editor_IEditor_H
