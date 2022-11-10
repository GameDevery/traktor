/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmPrimitive.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveBoolean", RfmPrimitiveBoolean, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveInt8", RfmPrimitiveInt8, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveUInt8", RfmPrimitiveUInt8, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveInt16", RfmPrimitiveInt16, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveUInt16", RfmPrimitiveUInt16, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveInt32", RfmPrimitiveInt32, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveUInt32", RfmPrimitiveUInt32, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveInt64", RfmPrimitiveInt64, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveUInt64", RfmPrimitiveUInt64, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveFloat", RfmPrimitiveFloat, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveDouble", RfmPrimitiveDouble, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveString", RfmPrimitiveString, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveWideString", RfmPrimitiveWideString, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveGuid", RfmPrimitiveGuid, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitivePath", RfmPrimitivePath, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveColor4ub", RfmPrimitiveColor4ub, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveColor4f", RfmPrimitiveColor4f, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveScalar", RfmPrimitiveScalar, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveVector2", RfmPrimitiveVector2, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveVector4", RfmPrimitiveVector4, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveMatrix33", RfmPrimitiveMatrix33, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveMatrix44", RfmPrimitiveMatrix44, ReflectionMember)
T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmPrimitiveQuaternion", RfmPrimitiveQuaternion, ReflectionMember)

}
