#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{

struct INSWindowEventsCallback
{
	virtual void event_windowDidMove() = 0;

	virtual void event_windowDidResize() = 0;

	virtual bool event_windowShouldClose() = 0;

	virtual void event_windowDidBecomeKey() = 0;

	virtual void event_windowDidResignKey() = 0;

	virtual void event_windowDidBecomeMain() = 0;

	virtual void event_windowDidResignMain() = 0;
};

	}
}

@interface NSWindowDelegateProxy : NSObject < NSWindowDelegate >
{
	traktor::ui::INSWindowEventsCallback* m_eventsCallback;
}

- (id) init;

- (id) initWithCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback;

- (void) setCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback;

- (void) windowDidMove: (NSNotification*)notification;

- (void) windowDidResize: (NSNotification*)notification;

- (BOOL) windowShouldClose: (id)sender;

- (void)windowDidBecomeKey: (NSNotification*)notification;

- (void)windowDidResignKey: (NSNotification*)notification;

- (void)windowDidBecomeMain: (NSNotification*)notification;

- (void)windowDidResignMain: (NSNotification*)notification;

@end
