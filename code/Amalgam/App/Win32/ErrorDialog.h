#ifndef traktor_amalgam_ErrorDialog_H
#define traktor_amalgam_ErrorDialog_H

#include "Core/Ref.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class LogList;

		}
	}

	namespace amalgam
	{

class ErrorDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	bool create();

	void addErrorString(const std::wstring& errorString);

private:
	Ref< ui::custom::LogList > m_listLog;

	void eventButtonCopyQuit(ui::Event* event);

	void eventButtonClickQuit(ui::Event* event);
};

	}
}

#endif	// traktor_amalgam_ErrorDialog_H
