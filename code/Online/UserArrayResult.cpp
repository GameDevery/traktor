/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/UserArrayResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserArrayResult", UserArrayResult, Result)

void UserArrayResult::succeed(const RefArray< IUser >& users)
{
	m_users = users;
	Result::succeed();
}

const RefArray< IUser >& UserArrayResult::get() const
{
	wait();
	return m_users;
}

	}
}
