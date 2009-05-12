#include "Render/Dx9/Platform.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/CubeTextureDx9.h"
#include "Render/Dx9/ContextDx9.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureDx9", CubeTextureDx9, CubeTexture)

CubeTextureDx9::CubeTextureDx9(ContextDx9* context)
:	m_context(context)
,	m_format(TfInvalid)
,	m_side(0)
,	m_lock(0)
{
}

CubeTextureDx9::~CubeTextureDx9()
{
	destroy();
}

bool CubeTextureDx9::create(IDirect3DDevice9* d3dDevice, const CubeTextureCreateDesc& desc)
{
	D3DLOCKED_RECT rc;
	HRESULT hr;

	hr = d3dDevice->CreateCubeTexture(
		desc.side,
		desc.mipCount,
		0,
		c_d3dFormat[desc.format],
		D3DPOOL_MANAGED,
		&m_d3dCubeTexture.getAssign(),
		NULL
	);
	if (FAILED(hr))
		return false;

	if (desc.immutable)
	{
		for (int side = 0; side < 6; ++side)
		{
			for (int i = 0; i < desc.mipCount; ++i)
			{
				if (FAILED(m_d3dCubeTexture->LockRect((D3DCUBEMAP_FACES)side, i, &rc, NULL, 0)))
					return false;

				const uint8_t* s = static_cast< const uint8_t* >(desc.initialData[side * desc.mipCount + i].data);
				uint8_t* d = static_cast< uint8_t* >(rc.pBits);

				for (int j = 0; j < (desc.side >> i); ++j)
				{
					textureCopy(d, s, rc.Pitch, desc.format);
					s += desc.initialData[side * desc.mipCount + i].pitch;
					d += rc.Pitch;
				}

				m_d3dCubeTexture->UnlockRect((D3DCUBEMAP_FACES)side, i);
			}
		}
	}

	m_format = desc.format;
	m_side = desc.side;
	return true;
}

void CubeTextureDx9::destroy()
{
#if !defined(_XBOX) && !defined(T_USE_XDK)
	if (m_context)
		m_context->releaseComRef(m_d3dCubeTexture);
#endif
}

int CubeTextureDx9::getWidth() const
{
	return m_side;
}

int CubeTextureDx9::getHeight() const
{
	return m_side;
}

int CubeTextureDx9::getDepth() const
{
	return m_side;
}

bool CubeTextureDx9::lock(int side, int level, Lock& lock)
{
	if (m_lock)
		return false;

	uint32_t size = getTextureMipPitch(m_format, m_side, m_side, level);
	m_lock = new uint8_t [size];

	lock.bits = m_lock;
	lock.pitch = getTextureRowPitch(m_format, m_side, level);

	return true;
}

void CubeTextureDx9::unlock(int side, int level)
{
	D3DLOCKED_RECT rc;

	if (SUCCEEDED(m_d3dCubeTexture->LockRect((D3DCUBEMAP_FACES)side, level, &rc, NULL, 0)))
	{
		uint32_t size = getTextureMipPitch(m_format, m_side, m_side, level);
		textureCopy(rc.pBits, m_lock, size, m_format);

		m_d3dCubeTexture->UnlockRect((D3DCUBEMAP_FACES)side, level);
	}

	delete[] m_lock;
	m_lock = 0;
}

IDirect3DBaseTexture9* CubeTextureDx9::getD3DBaseTexture() const
{
	T_ASSERT (m_d3dCubeTexture);
	return static_cast< IDirect3DBaseTexture9* >(m_d3dCubeTexture);
}

	}
}
