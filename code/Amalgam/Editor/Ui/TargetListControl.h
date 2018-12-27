/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_TargetListControl_H
#define traktor_amalgam_TargetListControl_H

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstanceListItem;

/*! \brief
 * \ingroup Amalgam
 */
class TargetListControl : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(TargetInstanceListItem* item);

	void removeAll();

private:
	RefArray< TargetInstanceListItem > m_items;

	virtual void layoutCells(const ui::Rect& rc) override final;
};

	}
}

#endif	// traktor_amalgam_TargetListControl_H
