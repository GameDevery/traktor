/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_ActionRemove_H
#define traktor_db_ActionRemove_H

#include <vector>
#include "Database/Local/Action.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

/*! \brief Transaction remove action.
 * \ingroup Database
 */
class ActionRemove : public Action
{
	T_RTTI_CLASS;

public:
	ActionRemove(const Path& instancePath);

	virtual bool execute(Context* context) override final;

	virtual bool undo(Context* context) override final;

	virtual void clean(Context* context) override final;

	virtual bool redundant(const Action* action) const override final;

private:
	Path m_instancePath;
	std::vector< std::wstring > m_renamedFiles;
};

	}
}

#endif	// traktor_db_ActionRemove_H
