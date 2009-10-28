
#import <UIKit/UIKit.h>

namespace traktor
{
	namespace input
	{

struct ITouchViewCallback
{
	virtual void touchesBegan(NSSet* touches, UIEvent* event) = 0;

	virtual void touchesMoved(NSSet* touches, UIEvent* event) = 0;

	virtual void touchesEnded(NSSet* touches, UIEvent* event) = 0;

	virtual void touchesCancelled(NSSet* touches, UIEvent* event) = 0;
};
	
	}
}

@interface UITouchView : UIView
{
	traktor::input::ITouchViewCallback* m_callback;
}

- (void) setCallback: (traktor::input::ITouchViewCallback*) callback;

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event;

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event;

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event;

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event;

@end
