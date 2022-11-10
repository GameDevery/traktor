/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

/*! Rotation modifier. */
class RotateModifier : public IModifier
{
	T_RTTI_CLASS;

public:
	explicit RotateModifier(SceneEditorContext* context);

	/*! \name Notifications */
	//\{

	virtual bool activate() override final;

	virtual void deactivate() override final;

	virtual void selectionChanged() override final;

	virtual bool cursorMoved(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	//\}

	/*! \name Modifications */
	//\{

	virtual bool begin(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		int32_t mouseButton
	) override final;

	virtual void apply(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		bool snapOverrideEnable
	) override final;

	virtual void end(const TransformChain& transformChain) override final;

	//\}

	/*! \name Preview */
	//\{

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const override final;

	//\}

private:
	SceneEditorContext* m_context;
	RefArray< EntityAdapter > m_entityAdapters;
	AlignedVector< Transform > m_baseTransforms;
	Vector4 m_center;
	float m_baseHead;
	float m_basePitch;
	float m_baseBank;
	float m_deltaHead;
	float m_deltaPitch;
	float m_deltaBank;
	uint32_t m_axisEnable;
};

	}
}

