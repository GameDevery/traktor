/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spray/SprayClassFactory.h"
#	include "Spray/Modifiers/BrownianModifierData.h"
#	include "Spray/Modifiers/DragModifierData.h"
#	include "Spray/Modifiers/GravityModifierData.h"
#	include "Spray/Modifiers/IntegrateModifierData.h"
#	include "Spray/Modifiers/PlaneCollisionModifierData.h"
#	include "Spray/Modifiers/SizeModifierData.h"
#	include "Spray/Modifiers/VortexModifierData.h"
#	include "Spray/Sources/BoxSourceData.h"
#	include "Spray/Sources/ConeSourceData.h"
#	include "Spray/Sources/DirectionalPointSourceData.h"
#	include "Spray/Sources/DiscSourceData.h"
#	include "Spray/Sources/LineSourceData.h"
#	include "Spray/Sources/PointSetSourceData.h"
#	include "Spray/Sources/PointSourceData.h"
#	include "Spray/Sources/QuadSourceData.h"
#	include "Spray/Sources/SphereSourceData.h"
#	include "Spray/Sources/VirtualSourceData.h"

namespace traktor
{
	namespace spray
	{

extern "C" void __module__Traktor_Spray()
{
	T_FORCE_LINK_REF(SprayClassFactory);
	T_FORCE_LINK_REF(BrownianModifierData);
	T_FORCE_LINK_REF(DragModifierData);
	T_FORCE_LINK_REF(GravityModifierData);
	T_FORCE_LINK_REF(IntegrateModifierData);
	T_FORCE_LINK_REF(PlaneCollisionModifierData);
	T_FORCE_LINK_REF(SizeModifierData);
	T_FORCE_LINK_REF(VortexModifierData);
	T_FORCE_LINK_REF(BoxSourceData);
	T_FORCE_LINK_REF(ConeSourceData);
	T_FORCE_LINK_REF(DirectionalPointSourceData);
	T_FORCE_LINK_REF(DiscSourceData);
	T_FORCE_LINK_REF(LineSourceData);
	T_FORCE_LINK_REF(PointSetSourceData);
	T_FORCE_LINK_REF(PointSourceData);
	T_FORCE_LINK_REF(QuadSourceData);
	T_FORCE_LINK_REF(SphereSourceData);
	T_FORCE_LINK_REF(VirtualSourceData);
}

	}
}

#endif
