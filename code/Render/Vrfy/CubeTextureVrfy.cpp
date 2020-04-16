#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/CubeTextureVrfy.h"
#include "Render/Vrfy/Error.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureVrfy", CubeTextureVrfy, ICubeTexture)

CubeTextureVrfy::CubeTextureVrfy(ICubeTexture* texture)
:	m_texture(texture)
{
	m_locked[0] =
	m_locked[1] = -1;
}

void CubeTextureVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* CubeTextureVrfy::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return this;
}

int32_t CubeTextureVrfy::getMips() const
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return m_texture ? m_texture->getMips() : 0;
}

int32_t CubeTextureVrfy::getSide() const
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return m_texture ? m_texture->getSide() : 0;
}

bool CubeTextureVrfy::lock(int32_t side, int32_t level, Lock& lock)
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	T_CAPTURE_ASSERT (side >= 0, L"Invalid side index.");
	T_CAPTURE_ASSERT (m_locked[0] < 0, L"Already locked.");
	if (m_texture && m_texture->lock(side, level, lock))
	{
		m_locked[0] = side;
		m_locked[1] = level;
		return true;
	}
	else
		return false;
}

void CubeTextureVrfy::unlock(int32_t side, int32_t level)
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	T_CAPTURE_ASSERT (side >= 0, L"Invalid side index.");
	T_CAPTURE_ASSERT (m_locked[0] == side, L"Trying to unlock incorrect side.");
	T_CAPTURE_ASSERT (m_locked[1] == level, L"Trying to unlock incorrect level.");
	if (m_texture)
		m_texture->unlock(side, level);
	m_locked[0] =
	m_locked[1] = -1;
}

	}
}
