#pragma once

#include <string>
#include "Ui/IBitmap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Styled bitmap.
 * \ingroup UI
 */
class T_DLLCLASS StyleBitmap : public IBitmap
{
	T_RTTI_CLASS;

public:
	explicit StyleBitmap(const wchar_t* const name);

	explicit StyleBitmap(const wchar_t* const name, IBitmap* defaultBitmap);

	explicit StyleBitmap(const wchar_t* const name, const void* defaultBitmapResource, uint32_t defaultBitmapResourceSize);

	virtual ~StyleBitmap();

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual Ref< drawing::Image > getImage() const override final;

	virtual ISystemBitmap* getSystemBitmap() const override final;

private:
	const wchar_t* const m_name;
	Ref< IBitmap > m_defaultBitmap;
	bool m_ownDefaultBitmap;
	mutable std::wstring m_path;
	mutable Ref< IBitmap > m_bitmap;

	bool resolve() const;
};

	}
}

