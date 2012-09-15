#include "Online/Score.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Score", Score, Object)

Score::Score(const IUser* user, int32_t score)
:	m_user(user)
,	m_score(score)
{
}

	}
}
