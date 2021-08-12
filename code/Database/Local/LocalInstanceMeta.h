#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

/*! Local instance meta.
 * \ingroup Database
 */
class LocalInstanceMeta : public ISerializable
{
	T_RTTI_CLASS;

public:
	LocalInstanceMeta() = default;

	explicit LocalInstanceMeta(const Guid& guid, const std::wstring& primaryType);

	void setGuid(const Guid& guid);

	const Guid& getGuid() const;

	void setPrimaryType(const std::wstring& primaryType);

	const std::wstring& getPrimaryType() const;

	void setBlob(const std::wstring& name);

	void removeBlob(const std::wstring& name);

	void removeAllBlobs();

	bool haveBlob(const std::wstring& name) const;

	const SmallSet< std::wstring >& getBlobs() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_guid;
	std::wstring m_primaryType;
	SmallSet< std::wstring > m_blobs;
};

	}
}

