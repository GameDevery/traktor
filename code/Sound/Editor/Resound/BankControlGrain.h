/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_BankControlGrain_H
#define traktor_sound_BankControlGrain_H

#include "Core/Ref.h"
#include "Ui/Bitmap.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace sound
	{

class IGrainData;

class BankControlGrain : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	BankControlGrain(BankControlGrain* parent, IGrainData* grain, const std::wstring& text, int32_t image);

	BankControlGrain* getParent() const;

	IGrainData* getGrain() const;

	int32_t getImage() const;

	std::wstring getText() const;

	void setActive(bool active);

	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position) override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) override final;

private:
	Ref< ui::Bitmap > m_bitmapGrain;
	BankControlGrain* m_parent;
	Ref< IGrainData > m_grain;
	std::wstring m_text;
	int32_t m_image;
	bool m_active;
};

	}
}

#endif	// traktor_sound_BankControlGrain_H
