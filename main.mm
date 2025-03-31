#import <Cocoa/Cocoa.h>
#import "Renderer.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#import "Metal/Metal.h"
#import "QuartzCore/QuartzCore.h"

@interface MetalAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation MetalAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    CAMetalLayer *metalLayer = [CAMetalLayer layer];
    Renderer renderer{1024, 768, metalLayer};
    NSWindow *nsWindow = glfwGetCocoaWindow(renderer.initialize());
    nsWindow.contentView.layer = metalLayer;
    nsWindow.contentView.wantsLayer = YES;
    renderer.run();
    [NSApp terminate:nil];
}
@end

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        MetalAppDelegate* delegate = [[MetalAppDelegate alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}

