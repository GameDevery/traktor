#ifndef traktor_ui_Application_H
#define traktor_ui_Application_H

#include "Ui/EventSubject.h"
#include "Ui/Enums.h"
#include "Ui/Itf/IEventLoop.h"
#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Clipboard;

/*! \brief User interface application.
 * \ingroup UI
 *
 * The Application singleton is used to register
 * which widget factory implementation to use.
 * It also provides system event handling and
 * clipboard access.
 */
class T_DLLCLASS Application : public EventSubject
{
	T_RTTI_CLASS;

public:
	static Application* getInstance();

	/*! \brief Initialize UI application.
	 *
	 * This must be called prior to any UI related
	 * calls are being made.
	 *
	 * \param eventLoop System event loop instance.
	 * \param widgetFactory System widget factory instance.
	 * \return True if application initialization succeeded.
	 */
	bool initialize(IEventLoop* eventLoop, IWidgetFactory* widgetFactory);
	
	/*! \brief Cleanup UI application. */
	void finalize();

	/*! \brief Process system events.
	 *
	 * Process a single system event.
	 * This method will return as soon as there are no
	 * more system events pending.
	 */
	bool process();

	/*! \brief Execute application.
	 *
	 * Process all system events.
	 * This method will block until the application
	 * terminates.
	 */
	int execute();

	/*! \brief Exit application.
	 *
	 * Should be called from the application when
	 * it wants to be terminated.
	 */
	void exit(int exitCode);

	/*! \brief Get system event loop. */
	IEventLoop* getEventLoop();

	/*! \brief Get system widget factory. */
	IWidgetFactory* getWidgetFactory();

	/*! \brief Get clipboard. */
	Ref< Clipboard > getClipboard();

	/*! \name Virtual key translation. */
	//@{

	/*! \brief Translate from key name to code.
	 *
	 * \note Key name is unlocalized, i.e. it's always in English.
	 */
	VirtualKey translateVirtualKey(const std::wstring& keyName) const;

	/*! \brief Translate from key code to name.
	 *
	 * \note Key name is unlocalized, i.e. it's always in English.
	 */
	std::wstring translateVirtualKey(VirtualKey virtualKey) const;

	//@}

private:
	IEventLoop* m_eventLoop;
	IWidgetFactory* m_widgetFactory;
	Ref< Clipboard > m_clipboard;
};

/*! \brief Convenience wrappers. */
inline Color4ub getSystemColor(SystemColor systemColor)
{
	Color4ub color;
	Application::getInstance()->getWidgetFactory()->getSystemColor(systemColor, color);
	return color;
}

	}
}

#endif	// traktor_ui_Application_H
