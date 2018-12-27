/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_MsgIntResult_H
#define traktor_db_MsgIntResult_H

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

/*! \brief Integer value result.
 * \ingroup Database
 */
class T_DLLCLASS MsgIntResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgIntResult(int32_t value = 0);

	uint32_t get() const { return m_value; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_value;
};

	}
}

#endif	// traktor_db_MsgIntResult_H