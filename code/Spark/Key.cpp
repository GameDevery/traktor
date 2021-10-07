#include <cstring>
#include "Spark/Key.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Key", Key, Object)

Key::Key()
:	m_lastKeyCode(0)
{
	std::memset(m_keyState, 0, sizeof(m_keyState));
}

int32_t Key::getAscii() const
{
	return m_lastKeyCode;
}

int32_t Key::getCode() const
{
	return m_lastKeyCode;
}

bool Key::isDown(int32_t keyCode) const
{
	return (keyCode >= 0 && keyCode < 256) ? m_keyState[keyCode] : false;
}

void Key::eventKeyDown(int32_t keyCode)
{
	m_keyState[keyCode] = true;
	m_lastKeyCode = keyCode;
	m_eventKeyDown.issue();
}

void Key::eventKeyUp(int32_t keyCode)
{
	m_keyState[keyCode] = false;
	m_eventKeyUp.issue();
}

	}
}
