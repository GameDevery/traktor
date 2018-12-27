/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_MsgStringResult_H
#define traktor_db_MsgStringResult_H

#include <string>
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

/*! \brief Single string result.
 * \ingroup Database
 */
class T_DLLCLASS MsgStringResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgStringResult(const std::wstring& value = L"");

	const std::wstring& get() const { return m_value; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_value;
};

	}
}

#endif	// traktor_db_MsgStringResult_H
