/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

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

class IBitmap;

/*! Tool form.
 * \ingroup UI
 */
class T_DLLCLASS ToolForm : public Container
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsDefault = WsResizable | WsSystemBox | WsCloseBox | WsCaption
	};

	ToolForm();

	bool create(Widget* parent, const std::wstring& text, int width, int height, int style = WsDefault, Layout* layout = 0);

	void setIcon(IBitmap* icon);

	IBitmap* getIcon();

	virtual DialogResult showModal();

	virtual void endModal(DialogResult result);

	bool isModal() const;

	virtual bool acceptLayout() const override;

private:
	Ref< IBitmap > m_icon;
	bool m_modal;
};

	}
}

