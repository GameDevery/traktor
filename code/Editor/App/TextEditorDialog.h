/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_TextEditorDialog_H
#define traktor_editor_TextEditorDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class RichEdit;

		}
	}

	namespace editor
	{

class TextEditorDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, const std::wstring& initialText);

	virtual std::wstring getText() const T_OVERRIDE T_FINAL;

private:
	Ref< ui::custom::RichEdit > m_edit;
};

	}
}

#endif	// traktor_editor_TextEditorDialog_H
