/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Skeleton;
class Pose;

/*! \ingroup Animation */
//@{

void T_DLLCLASS calculateJointLocalTransforms(
	const Skeleton* skeleton,
	AlignedVector< Transform >& outJointTransforms
);

void T_DLLCLASS calculateJointTransforms(
	const Skeleton* skeleton,
	AlignedVector< Transform >& outJointTransforms
);

void T_DLLCLASS calculatePoseLocalTransforms(
	const Skeleton* skeleton,
	const Pose* pose,
	AlignedVector< Transform >& outPoseLocalTransforms
);

void T_DLLCLASS calculatePoseTransforms(
	const Skeleton* skeleton,
	const Pose* pose,
	AlignedVector< Transform >& outPoseTransforms
);

Aabb3 T_DLLCLASS calculateBoundingBox(const Skeleton* skeleton);

Aabb3 T_DLLCLASS calculateBoundingBox(const Skeleton* skeleton, const Pose* pose);

void T_DLLCLASS blendPoses(
	const Pose* pose1,
	const Pose* pose2,
	const Scalar& blend,
	Pose* outPose
);

//@}

}
