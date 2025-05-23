/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"
#include "Ui/Unit.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;

/*! Tool form.
 * \ingroup UI
 */
class T_DLLCLASS ToolForm : public Container
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsDefault = WsResizable | WsSystemBox | WsCloseBox | WsCaption;

	bool create(Widget* parent, const std::wstring& text, Unit width, Unit height, uint32_t style = WsDefault, Layout* layout = 0);

	void setIcon(IBitmap* icon);

	IBitmap* getIcon() const;

	void setLayerImage(IBitmap* layerImage);

	IBitmap* getLayerImage() const;

	virtual DialogResult showModal();

	virtual void endModal(DialogResult result);

	bool isModal() const;

	virtual bool isEnable(bool includingParents) const override;

	virtual bool acceptLayout() const override;

private:
	Ref< IBitmap > m_icon;
	Ref< IBitmap > m_layerImage;
	bool m_modal = false;
};

}
