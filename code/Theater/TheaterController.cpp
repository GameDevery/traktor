#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Theater/Act.h"
#include "Theater/TheaterController.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterController", TheaterController, scene::ISceneController)

TheaterController::TheaterController(const RefArray< const Act >& acts, float totalDuration)
:	m_acts(acts)
,	m_totalDuration(totalDuration)
{
}

bool TheaterController::play(const std::wstring& actName)
{
	auto it = std::find_if(m_acts.begin(), m_acts.end(), [&](const Act* act) {
		return actName == act->getName();
	});
	if (it == m_acts.end())
	{
		log::warning << L"No act \"" << actName << L"\" found." << Endl;
		return false;
	}

	if (*it != m_act)
	{
		m_act = *it;
		m_timeLast = -1.0f;
	}

	m_timeStart = -1.0f;
	return true;
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (m_timeStart < 0.0f)
		m_timeStart = time;

	if (m_act != nullptr)
	{
		float timeAct = time - m_timeStart;
		float duration = m_act->getEnd() - m_act->getStart();
		if (timeAct < -FUZZY_EPSILON || timeAct > duration + FUZZY_EPSILON)
			m_act = nullptr;
	}

	if (m_act == nullptr)
		return;

	// Do nothing if no time has passed since last update.
	if (traktor::abs(time - m_timeLast) <= FUZZY_EPSILON)
		return;

	// Evaluate current act.
	m_act->update(scene, time - m_timeStart, deltaTime);

	m_timeLast = time;
}

	}
}
