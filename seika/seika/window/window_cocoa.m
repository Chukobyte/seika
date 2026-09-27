#include "window.h"

#if defined(PLATFORM_MACOS)

// Legacy NSOpenGL (not Metal) -- matches CMakeLists.txt's APPLE branch linking OpenGL.framework,
// and matches every other backend here (win32/x11) targeting desktop GL 3.3 core via glad, not a
// separate rendering API. NSOpenGL is deprecated by Apple in favor of Metal but still fully
// functional through current macOS releases.
#import <Cocoa/Cocoa.h>

#if SKA_RENDERING
#include "seika/rendering/renderer.h"
#include "seika/rendering/screen_capture.h"
#endif

#if SKA_INPUT
#include "seika/input/input.h"
#include "seika/input/input_cocoa.h"
#endif

#include "seika/logger.h"
#include "seika/assert.h"

static SkaWindow main_window = {0};

typedef struct SkaCocoaData {
    NSWindow* window;
#if SKA_RENDERING
    NSOpenGLContext* gl_context;
#endif
} SkaCocoaData;

static SkaCocoaData cocoa_data = {0};

// Bridges NSWindowDelegate notifications (close/resize) back to the plain-C SkaWindow state --
// mirrors what WM_DESTROY/WM_SIZE do inline in window_win32.c's WindowProc and what
// ClientMessage/ConfigureNotify do inline in window_x11.c's ska_win_process_events, since Cocoa
// delivers these as delegate callbacks instead of messages/events pulled from the same queue.
@interface SkaWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation SkaWindowDelegate
- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    main_window.is_active = false;
}
- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
#if SKA_RENDERING
    if (cocoa_data.gl_context) {
        NSRect frame = [[cocoa_data.window contentView] frame];
        ska_win_update_size(&main_window, (int32)frame.size.width, (int32)frame.size.height);
    }
#endif
}
@end

static SkaWindowDelegate* window_delegate = nil;

// Idempotency guard -- see the matching comment in window_win32.c.
static bool s_win_system_initialized = false;

bool ska_win_init(void) {
    if (s_win_system_initialized) return true;

    [NSApplication sharedApplication];
    // Required for a bare (non .app-bundled) executable to become the frontmost app and receive
    // real keyboard/mouse focus -- without this, a plain command-line-launched Cocoa app can end
    // up with no Dock presence and never actually gets OS input focus no matter what's clicked.
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];

    s_win_system_initialized = true;
    return true;
}

SkaWindow* ska_win_create_window(SkaCreateWindowProps props) {
    const char* title = props.title;
    const int32 width = (int32)props.width;
    const int32 height = (int32)props.height;

    // Cocoa's screen origin is bottom-left (Y-up); win32/x11 (and every caller of this API --
    // e.g. koa/koa/std/game.c hardcodes pos_y=100 meaning "100px from the TOP") assume top-left
    // (Y-down). Without flipping, a window positioned identically on Windows/Linux would appear
    // near the BOTTOM of the screen on macOS instead of near the top.
    const CGFloat screen_height = [[NSScreen mainScreen] frame].size.height;
    const CGFloat flipped_y = screen_height - (CGFloat)props.pos_y - (CGFloat)height;
    NSRect frame = NSMakeRect((CGFloat)props.pos_x, flipped_y, (CGFloat)width, (CGFloat)height);
    NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                             | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                     styleMask:style
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
    [window setTitle:[NSString stringWithUTF8String:title]];
    // Off by default -- AppKit only delivers NSEventTypeMouseMoved while this is set; without it,
    // plain mouse movement (no button held) never generates any event at all, only drags do.
    [window setAcceptsMouseMovedEvents:YES];

    window_delegate = [[SkaWindowDelegate alloc] init];
    [window setDelegate:window_delegate];

    cocoa_data.window = window;

#if SKA_RENDERING
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        // matches the win32 PFD's cDepthBits=32 / GLX's GLX_DEPTH_SIZE=32
        NSOpenGLPFADepthSize, 32,
        // matches the win32 PFD's cStencilBits=8 / GLX's GLX_STENCIL_SIZE=8
        NSOpenGLPFAStencilSize, 8,
        // The highest core-profile constant the legacy NSOpenGL API exposes -- in practice this
        // still grants whatever the actual highest core profile the driver supports is (commonly
        // 4.1 core on real hardware), same as how WGL_CONTEXT_MAJOR/MINOR_VERSION_ARB=3.3 doesn't
        // cap the GL version glad ends up reporting via GLVersion.major/minor after context
        // creation there either.
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        0
    };
    NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    SKA_ASSERT_REL_FMT(pixel_format != nil, "No matching NSOpenGL pixel format found!");

    NSOpenGLContext* gl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    SKA_ASSERT_REL_FMT(gl_context != nil, "Failed to create NSOpenGLContext!");
    [pixel_format release]; // NSOpenGLContext retains what it needs from this internally
    [gl_context setView:[window contentView]];
    [gl_context makeCurrentContext];
    cocoa_data.gl_context = gl_context;

    if (!gladLoadGL()) {
        ska_logger_error("Could not initialize GLAD!");
    } else {
        ska_logger_debug("OpenGL Version %d", GLVersion.major);
    }

    // Vsync: NSOpenGLCPSwapInterval is the Cocoa equivalent of wglSwapIntervalEXT/
    // glXSwapIntervalEXT used in the win32/x11 backends -- no extension-string probing needed,
    // it's always available on this API.
    GLint swap_interval = 1;
    [gl_context setValues:&swap_interval forParameter:NSOpenGLContextParameterSwapInterval];
#endif // SKA_RENDERING

    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    main_window = (SkaWindow){
        .title = title, .width = width, .height = height, .is_active = true};
    ska_win_update_size(&main_window, width, height);
    return &main_window;
}

SkaWindow* ska_win_get_primary_window() {
    return main_window.is_active ? &main_window : NULL;
}

bool ska_win_is_active(const SkaWindow* window) { return window->is_active; }

void ska_win_destroy_window(SkaWindow* window) {
    window->is_active = false;
}

void ska_win_process_events(SkaWindow* window) {
    (void)window;
    NSEvent* event;
    // Non-blocking poll (distantPast date), matching PeekMessage(..., PM_REMOVE) and the
    // XPending()-guarded XNextEvent loop in the other backends -- process everything currently
    // queued, then return immediately rather than blocking for the next event.
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES]) != nil) {
#if SKA_INPUT
        switch ([event type]) {
            case NSEventTypeKeyDown: {
                if (![event isARepeat]) {
                    const SkaInputKey key = ska_input_cocoa_keycode_to_keycode([event keyCode]);
                    ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, key, SkaInputTriggerType_PRESSED);
                }
                break;
            }
            case NSEventTypeKeyUp: {
                const SkaInputKey key = ska_input_cocoa_keycode_to_keycode([event keyCode]);
                ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, key, SkaInputTriggerType_RELEASED);
                break;
            }
            case NSEventTypeLeftMouseDown:
                ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_LEFT, SkaInputTriggerType_PRESSED);
                break;
            case NSEventTypeLeftMouseUp:
                ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_LEFT, SkaInputTriggerType_RELEASED);
                break;
            case NSEventTypeRightMouseDown:
                ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_RIGHT, SkaInputTriggerType_PRESSED);
                break;
            case NSEventTypeRightMouseUp:
                ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_RIGHT, SkaInputTriggerType_RELEASED);
                break;
            case NSEventTypeOtherMouseDown:
                if ([event buttonNumber] == 2) {
                    ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_MIDDLE, SkaInputTriggerType_PRESSED);
                }
                break;
            case NSEventTypeOtherMouseUp:
                if ([event buttonNumber] == 2) {
                    ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_MIDDLE, SkaInputTriggerType_RELEASED);
                }
                break;
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeOtherMouseDragged: {
                // Cocoa's window-local coords are bottom-left origin, Y-up; every other backend
                // here (win32/x11) reports top-left origin, Y-down -- flip to match.
                NSPoint loc = [event locationInWindow];
                NSRect content_frame = [[cocoa_data.window contentView] frame];
                const SkaVector2i new_position = {
                    (int32)loc.x,
                    (int32)(content_frame.size.height - loc.y)
                };
                ska_input_register_mouse_move_event(new_position);
                break;
            }
            default:
                break;
        }
#endif // SKA_INPUT
        // Let AppKit do its own normal processing too (title bar dragging, close button,
        // menu bar, etc.) -- not just our own switch above.
        [NSApp sendEvent:event];
    }
}

void ska_win_swap_window(SkaWindow* window) {
#if SKA_RENDERING
    ska_screen_capture_on_frame_end(window->width, window->height);
    [cocoa_data.gl_context flushBuffer];
#else
    (void)window;
#endif
}

void ska_win_update_size(SkaWindow* window, int32 width, int32 height) {
    window->width = width;
    window->height = height;
#if SKA_RENDERING
    ska_renderer_update_window_size(width, height);
    if (cocoa_data.gl_context) {
        [cocoa_data.gl_context update];
    }
#endif
}

#endif // #if defined(PLATFORM_MACOS)
