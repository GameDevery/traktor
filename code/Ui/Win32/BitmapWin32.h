#pragma once

#define _WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#if defined(T_USE_GDI_PLUS)
// \hack As we don't want min/max defined but gdiplus.h require them
// we temporarily define them first.
#	define max(a,b) (((a) > (b)) ? (a) : (b))
#	define min(a,b) (((a) < (b)) ? (a) : (b))
#	include <gdiplus.h>
#	undef min
#	undef max
#endif
#include "Core/Misc/AutoPtr.h"
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{

/*! Win32 system bitmap.
 * \ingroup UIW32
 */
class BitmapWin32 : public ISystemBitmap
{
public:
	BitmapWin32();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	const void* getBits() const { return m_bits.c_ptr(); }

	const void* getBitsPreMulAlpha() const { return m_bitsPreMulAlpha.c_ptr(); }

	bool haveAlpha() const { return m_haveAlpha; }

	HICON createIcon() const;

#if defined(T_USE_GDI)
	HBITMAP getHBitmap() const { return m_hBitmap; }

	HBITMAP getHBitmapPreMulAlpha() const { return m_hBitmapPreMulAlpha; }
#endif

	int32_t getTag() const { return m_tag; }

	int32_t getRevision() const { return m_revision; }

private:
	static std::atomic< int32_t > ms_nextTag;
	int32_t m_tag = 0;
	std::atomic< int32_t > m_revision = 0;
	AutoArrayPtr< uint32_t > m_bits;
	AutoArrayPtr< uint32_t > m_bitsPreMulAlpha;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	bool m_haveAlpha = false;

#if defined(T_USE_GDI)
	HBITMAP m_hBitmap = NULL;
	HBITMAP m_hBitmapPreMulAlpha = NULL;
#endif
};

	}
}

