#ifndef traktor_ui_FileDialogWin32_H
#define traktor_ui_FileDialogWin32_H

#include "Core/Misc/TString.h"
#include "Ui/Itf/IFileDialog.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief
 * \ingroup UIW32
 */
class FileDialogWin32 : public IFileDialog
{
public:
	FileDialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save);

	virtual void destroy();

	virtual int showModal(Path& outPath);

	virtual int showModal(std::vector< Path >& outPaths);

private:
	EventSubject* m_owner;
	Window m_hWnd;
	bool m_save;

	OPENFILENAME m_ofn;
	TCHAR m_filters[256];
	TCHAR m_title[64];
	TCHAR m_fileName[MAX_PATH * 64];
};

	}
}

#endif	// traktor_ui_FileDialogWin32_H
