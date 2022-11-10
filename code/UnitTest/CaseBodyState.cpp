/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/BodyState.h"
#include "UnitTest/CaseBodyState.h"
#include "UnitTest/MathCompare.h"

namespace traktor
{

void CaseBodyState::run()
{
	physics::BodyState S0, S1;

	S0.setTransform(Transform(
		Vector4(-1.0f, -2.0f, -3.0f, 1.0f),
		Quaternion::fromEulerAngles(
			deg2rad(90.0f),
			deg2rad(0.0f),
			deg2rad(0.0f)
		)
	));
	S0.setLinearVelocity(Vector4(-1.0f, -2.0f, -3.0f, 0.0f));
	S0.setAngularVelocity(Vector4(1.0f, 0.0f, 0.0f, 0.0f));

	S1.setTransform(Transform(
		Vector4(1.0f, 2.0f, 3.0f, 1.0f),
		Quaternion::fromEulerAngles(
			deg2rad(0.0f),
			deg2rad(90.0f),
			deg2rad(0.0f)
		)
	));
	S1.setLinearVelocity(Vector4(1.0f, 2.0f, 3.0f, 0.0f));
	S1.setAngularVelocity(Vector4(0.0f, 1.0f, 0.0f, 0.0f));

	physics::BodyState R0 = S0.interpolate(S1, Scalar(0.0f));
	physics::BodyState R1 = S0.interpolate(S1, Scalar(1.0f));

	CASE_ASSERT_COMPARE(S0.getTransform(), R0.getTransform(), compareTransformEqual);
	CASE_ASSERT_COMPARE(S0.getLinearVelocity(), R0.getLinearVelocity(), compareVectorEqual);
	CASE_ASSERT_COMPARE(S0.getAngularVelocity(), R0.getAngularVelocity(), compareVectorEqual);

	CASE_ASSERT_COMPARE(S1.getTransform(), R1.getTransform(), compareTransformEqual);
	CASE_ASSERT_COMPARE(S1.getLinearVelocity(), R1.getLinearVelocity(), compareVectorEqual);
	CASE_ASSERT_COMPARE(S1.getAngularVelocity(), R1.getAngularVelocity(), compareVectorEqual);

	physics::BodyState R2 = S0.interpolate(S1, Scalar(0.5f));
	physics::BodyState R3 = S1.interpolate(S0, Scalar(0.5f));

	CASE_ASSERT_COMPARE(R2.getTransform(), R3.getTransform(), compareTransformEqual);
	CASE_ASSERT_COMPARE(R2.getLinearVelocity(), R3.getLinearVelocity(), compareVectorEqual);
	CASE_ASSERT_COMPARE(R2.getAngularVelocity(), R3.getAngularVelocity(), compareVectorEqual);
}

}
