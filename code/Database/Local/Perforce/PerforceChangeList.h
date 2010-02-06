#ifndef traktor_db_PerforceChangeList_H
#define traktor_db_PerforceChangeList_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

class PerforceChangeListFile;

/*! \brief P4 change list.
 * \ingroup Database
 */
class PerforceChangeList : public ISerializable
{
	T_RTTI_CLASS;

public:
	PerforceChangeList();

	PerforceChangeList(
		uint32_t change ,
		const std::wstring& user,
		const std::wstring& client,
		const std::wstring& description
	);

	uint32_t getChange() const;

	const std::wstring& getUser() const;

	const std::wstring& getClient() const;

	const std::wstring& getDescription() const;

	void setFiles(const RefArray< PerforceChangeListFile >& files);

	const RefArray< PerforceChangeListFile >& getFiles() const;

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_change;
	std::wstring m_user;
	std::wstring m_client;
	std::wstring m_description;
	RefArray< PerforceChangeListFile > m_files;
};

	}
}

#endif	// traktor_db_PerforceChangeList_H
