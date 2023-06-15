/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Misc/SafeDestroy.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor::ui
{
	namespace
	{

#pragma pack(1)
struct ImageEntry
{
	uint16_t dpi;
	uint32_t offset;
};
#pragma pack()

#pragma pack(1)
struct ImageHeader
{
	uint16_t count;
	ImageEntry entry[1];
};
#pragma pack()

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StyleBitmap", StyleBitmap, IBitmap)

StyleBitmap::StyleBitmap(const wchar_t* const name, int32_t index)
:	m_name(name)
,	m_index(index)
{
}

StyleBitmap::~StyleBitmap()
{
	destroy();
}

void StyleBitmap::destroy()
{
	safeDestroy(m_bitmap);
}

Size StyleBitmap::getSize(const Widget* reference) const
{
	const int32_t dpi = reference != nullptr ? reference->dpi() : 96;
	return resolve(dpi) ? m_bitmap->getSize() : Size();
}

Ref< drawing::Image > StyleBitmap::getImage(const Widget* reference) const
{
	const int32_t dpi = reference != nullptr ? reference->dpi() : 96;
	return resolve(dpi) ? m_bitmap->getImage() : nullptr;
}

ISystemBitmap* StyleBitmap::getSystemBitmap(const Widget* reference) const
{
	const int32_t dpi = reference != nullptr ? reference->dpi() : 96;
	return resolve(dpi) ? m_bitmap : nullptr;
}

bool StyleBitmap::resolve(int32_t dpi) const
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
		return false;

	const Path fileName = ss->getValue(m_name);

	if (m_bitmap != nullptr && dpi == m_dpi && fileName.getPathName() == m_fileName)
		return true;

	safeDestroy(m_bitmap);
	m_fileName = L"";
	m_dpi = -1;

	if (fileName.empty() || fileName.getExtension() != L"image")
		return false;

	Ref< IStream > s = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!s)
		return false;

	DynamicMemoryStream dms(false, true);
	StreamCopy(&dms, s).execute();

	safeClose(s);

	const void* resource = &dms.getBuffer()[0];
	const uint32_t size = (uint32_t)dms.getBuffer().size();

	int32_t bestFit = std::numeric_limits< int32_t >::max();
	int32_t bestFitIndex = 0;

	const ImageHeader* h = static_cast<const ImageHeader*>(resource);
	for (uint32_t i = 0; i < h->count; ++i)
	{
		if (abs(dpi - h->entry[i].dpi) < bestFit)
		{
			bestFit = abs(dpi - h->entry[i].dpi);
			bestFitIndex = i;
		}
	}

	MemoryStream ms(
		static_cast<const uint8_t*>(resource) + h->entry[bestFitIndex].offset,
		size - h->entry[bestFitIndex].offset
	);
	Ref< drawing::Image > image = drawing::Image::load(&ms, L"png");
	if (!image)
		return false;

	ISystemBitmap* bm;
	if (!(bm = Application::getInstance()->getWidgetFactory()->createBitmap()))
		return false;

	const int32_t iw = image->getWidth();
	const int32_t ih = image->getHeight();

	if (m_index < 0)
	{
		if (!bm->create(iw, ih))
		{
			safeDestroy(bm);
			return false;
		}
		bm->copySubImage(
			image,
			Rect(0, 0, iw, ih),
			Point(0, 0)
		);
	}
	else
	{
		if (!bm->create(ih, ih))
		{
			safeDestroy(bm);
			return false;
		}
		bm->copySubImage(
			image,
			Rect(Point(m_index * ih, 0), Size(ih, ih)),
			Point(0, 0)
		);
	}

	m_bitmap = bm;
	m_fileName = fileName.getPathName();
	m_dpi = dpi;
	return true;
}

}
