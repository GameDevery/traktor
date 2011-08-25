#ifndef traktor_db_LocalInstance_H
#define traktor_db_LocalInstance_H

#include "Database/Provider/IProviderInstance.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class Context;
class LocalGroup;
class Transaction;

/*! \brief Local instance.
 * \ingroup Database
 */
class LocalInstance : public IProviderInstance
{
	T_RTTI_CLASS;

public:
	LocalInstance(Context* context, const Path& instancePath);

	bool internalCreateNew(const Guid& instanceGuid);

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool openTransaction();

	virtual bool commitTransaction();

	virtual bool closeTransaction();

	virtual std::wstring getName() const;

	virtual bool setName(const std::wstring& name);

	virtual Guid getGuid() const;

	virtual bool setGuid(const Guid& guid);

	virtual bool remove();

	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType);

	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual bool removeAllData();

	virtual Ref< IStream > readData(const std::wstring& dataName);

	virtual Ref< IStream > writeData(const std::wstring& dataName);

private:
	Ref< Context > m_context;
	Path m_instancePath;
	Ref< Transaction > m_transaction;
	std::wstring m_transactionName;
};

	}
}

#endif	// traktor_db_LocalInstance_H
