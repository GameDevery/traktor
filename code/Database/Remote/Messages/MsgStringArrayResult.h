/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_MsgStringArrayResult_H
#define traktor_db_MsgStringArrayResult_H

#include <string>
#include <vector>
#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Array of string result.
 * \ingroup Database
 */
class T_DLLCLASS MsgStringArrayResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgStringArrayResult();

	explicit MsgStringArrayResult(const std::vector< std::wstring >& values);

	void add(const std::wstring& value);

	uint32_t count();

	const std::wstring& get(uint32_t index) const;

	const std::vector< std::wstring >& get() const { return m_values; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::vector< std::wstring > m_values;
};

	}
}

#endif	// traktor_db_MsgStringArrayResult_H
