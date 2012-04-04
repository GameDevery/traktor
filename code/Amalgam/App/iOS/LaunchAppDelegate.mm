#import "Amalgam/App/iOS/EAGLView.h"
#import "Amalgam/App/iOS/LaunchAppDelegate.h"

@implementation LaunchAppDelegate

@synthesize window;
@synthesize glView;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	// Create window.
	window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
	[window setBackgroundColor: [UIColor blackColor]];

	// Create view.
	glView = [[EAGLView alloc] initWithFrame: window.bounds];
	[window addSubview: glView];

	// Make window visible.
	[window makeKeyAndVisible];
	
	// Begin animation thread.
	[glView startAnimation];
	
    return YES;
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	[glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	[glView startAnimation];
}

- (void) dealloc
{
	[window release];	
	[super dealloc];
}

@end
