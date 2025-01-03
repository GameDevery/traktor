/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionSetGuid", ActionSetGuid, Action)

ActionSetGuid::ActionSetGuid(const Path& instancePath, const Guid& newGuid, bool create)
:	m_instancePath(instancePath)
,	m_newGuid(newGuid)
,	m_create(create)
,	m_editMeta(false)
{
}

bool ActionSetGuid::execute(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	const Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Ref< LocalInstanceMeta > instanceMeta;

	if (!m_create)
	{
		instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
		if (!instanceMeta)
		{
			log::error << L"Unable to read instance meta data, \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
			return false;
		}

		m_editMeta = fileStore->edit(instanceMetaPath);
		if (!m_editMeta)
		{
			log::error << L"Unable to open \"" << instanceMetaPath.getPathName() << L"\" for edit." << Endl;
			return false;
		}
	}
	else
		instanceMeta = new LocalInstanceMeta();

	instanceMeta->setGuid(m_newGuid);

	if (!writePhysicalObject(instanceMetaPath, instanceMeta, context.preferBinary()))
	{
		log::error << L"Unable to write instance meta data, \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
		return false;
	}

	if (m_create)
		fileStore->add(instanceMetaPath);

	return true;
}

bool ActionSetGuid::undo(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	const Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editMeta)
	{
		fileStore->rollback(instanceMetaPath);
		m_editMeta = false;
	}
	else if (m_create)
	{
		return FileSystem::getInstance().remove(
			instanceMetaPath
		);
	}

	return true;
}

void ActionSetGuid::clean(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	const Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editMeta)
		fileStore->clean(instanceMetaPath);
}

bool ActionSetGuid::redundant(const Action* action) const
{
	return false;
}

}
