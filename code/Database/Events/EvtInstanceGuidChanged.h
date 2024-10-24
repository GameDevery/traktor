/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Events/EvtInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Instance guid changed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceGuidChanged : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceGuidChanged() = default;

	explicit EvtInstanceGuidChanged(const Guid& instanceGuid, const Guid& instancePreviousGuid);

	/*! Instance's previous guid.
	 *
	 * \return Previous guid of instance.
	 */
	const Guid& getInstancePreviousGuid() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_instancePreviousGuid;
};

}
