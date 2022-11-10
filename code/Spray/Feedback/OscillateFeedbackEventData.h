/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/Feedback/IFeedbackListener.h"
#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS OscillateFeedbackEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	struct OscillatingValue
	{
		float duration;
		int32_t frequency;
		float magnitude;
		float noise;

		OscillatingValue();

		void serialize(ISerializer& s);
	};

	OscillateFeedbackEventData();

	virtual void serialize(ISerializer& s) override final;

	FeedbackType getType() const { return m_type; }

	const OscillatingValue& getValue(int32_t index) const { return m_values[index]; }

private:
	FeedbackType m_type;
	OscillatingValue m_values[4];
};

	}
}

