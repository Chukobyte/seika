#include "window.h"

// The !PLATFORM_EMSCRIPTEN exclusion is defensive: seika/seika/defines.h derives
// PLATFORM_EMSCRIPTEN from __EMSCRIPTEN__, but if emcc's toolchain also ever leaves __linux__
// defined in some configuration, PLATFORM_LINUX would end up defined alongside PLATFORM_EMSCRIPTEN
// (there's no __ANDROID__-style redefinition trick removing it, unlike the Android case) --
// without this guard this file would compile side by side with window_emscripten.c, needing real
// Xlib headers unavailable under emcc.
#if defined(PLATFORM_LINUX) && !defined(PLATFORM_EMSCRIPTEN)

// This backend targets Xlib/GLX only, not native Wayland. On Wayland-native sessions the vast
// majority of current desktops (GNOME, KDE, etc.) transparently run X11 apps via XWayland, so
// this works unmodified there -- it will not run standalone on a Wayland-only compositor with no
// XWayland compatibility layer. Native Wayland (wl_compositor/xdg-shell/EGL) is a substantially
// larger, separate undertaking and is out of scope here.

#include <string.h>

// Must come before <GL/glx.h> below: glad.h defines the same include guards as the system
// <GL/gl.h>, so pulling it in first makes glx.h's own internal '#include <GL/gl.h>' a no-op
// instead of hitting glad's "OpenGL header already included" #error (glad insists on being the
// sole source of GL declarations).
#if SKA_RENDERING
#include "seika/rendering/renderer.h"
#include "seika/rendering/screen_capture.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include "seika/logger.h"
#include "seika/assert.h"
#include "seika/input/input_x11.h"

static SkaWindow main_window = {0};

typedef struct SkaX11Data {
    Display* display;
    Window window;
    GLXContext glx_context;
    Colormap colormap;
    Atom wm_delete_window;
    int screen;
} SkaX11Data;

static SkaX11Data x11_data = {0};

#if SKA_RENDERING
static bool is_open_gl_initialized = false;

typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*, GLXDrawable, int);

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_PROFILE_MASK_ARB  0x9126
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

static GLXFBConfig choose_fb_config(void) {
    const int visual_attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 32, // matches the win32 PFD's cDepthBits=32
        GLX_STENCIL_SIZE, 8, // matches the win32 PFD's cStencilBits=8
        GLX_DOUBLEBUFFER, True,
        None
    };
    int fb_count = 0;
    GLXFBConfig* fb_configs = glXChooseFBConfig(x11_data.display, x11_data.screen, visual_attribs, &fb_count);
    SKA_ASSERT_REL_FMT(fb_configs && fb_count > 0, "No matching GLX FB config found!");
    GLXFBConfig best = fb_configs[0];
    XFree(fb_configs);
    return best;
}

// Unlike WGL, glXGetProcAddressARB can resolve extension function pointers (including
// glXCreateContextAttribsARB itself) against the X Display alone -- no temporary legacy
// context needs to be current first, so (unlike opengl_init in window_win32.c) there's no
// "create temp context, look up ARB function, create real context" two-step dance here.
static void opengl_init(GLXFBConfig fb_config) {
    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
        (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
    SKA_ASSERT_REL_FMT(glXCreateContextAttribsARB != NULL, "GLX_ARB_create_context not supported!");

    const int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    x11_data.glx_context = glXCreateContextAttribsARB(x11_data.display, fb_config, NULL, True, context_attribs);
    SKA_ASSERT_REL_FMT(x11_data.glx_context != NULL, "Failed to create GLX context!");
    glXMakeCurrent(x11_data.display, x11_data.window, x11_data.glx_context);

    if (!gladLoadGL()) {
        ska_logger_error("Could not initialize GLAD!");
    } else {
        ska_logger_debug("OpenGL Version %d", GLVersion.major);
    }

    const char* glx_extensions = glXQueryExtensionsString(x11_data.display, x11_data.screen);
    if (strstr(glx_extensions, "GLX_EXT_swap_control")) {
        PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT =
            (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
        glXSwapIntervalEXT(x11_data.display, x11_data.window, 1);
        ska_logger_debug("VSynch enabled");
    } else {
        ska_logger_error("GLX_EXT_swap_control not supported");
    }
    is_open_gl_initialized = true;
}
#endif // SKA_RENDERING

// Idempotency guard -- see the matching comment in window_win32.c.
static bool s_win_system_initialized = false;

bool ska_win_init(void) {
    if (s_win_system_initialized) return true;

    x11_data.display = XOpenDisplay(NULL);
    if (!x11_data.display) {
        ska_logger_error("Failed to open X11 display");
        return false;
    }
    x11_data.screen = DefaultScreen(x11_data.display);
    s_win_system_initialized = true;
    return true;
}

SkaWindow* ska_win_create_window(SkaCreateWindowProps props) {
    const char* title = props.title;
    const int32 width = (int32)props.width;
    const int32 height = (int32)props.height;

#if SKA_RENDERING
    GLXFBConfig fb_config = choose_fb_config();
    XVisualInfo* visual_info = glXGetVisualFromFBConfig(x11_data.display, fb_config);
#else
    XVisualInfo visual_info_storage;
    XVisualInfo* visual_info = &visual_info_storage;
    XMatchVisualInfo(x11_data.display, x11_data.screen, 24, TrueColor, visual_info);
#endif

    const Window root = RootWindow(x11_data.display, visual_info->screen);
    x11_data.colormap = XCreateColormap(x11_data.display, root, visual_info->visual, AllocNone);

    XSetWindowAttributes swa = {0};
    swa.colormap = x11_data.colormap;
    swa.event_mask = ExposureMask | StructureNotifyMask
#if SKA_INPUT
                    | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
#endif
                    ;

    // XCreateWindow's width/height IS the client/drawable area already -- window manager
    // decorations (title bar, borders) are drawn outside this rect by the WM, unlike Win32
    // where CreateWindowEx's size includes the frame and AdjustWindowRectEx is needed to
    // compensate. No equivalent adjustment is needed here.
    Window window = XCreateWindow(
        x11_data.display, root,
        props.pos_x, props.pos_y, (unsigned int)width, (unsigned int)height,
        0, visual_info->depth, InputOutput, visual_info->visual,
        CWColormap | CWEventMask, &swa);
    x11_data.window = window;

#if SKA_RENDERING
    XFree(visual_info);
#endif

    XStoreName(x11_data.display, window, title);

    // X11 has no WM_DESTROY-equivalent by default -- without registering WM_DELETE_WINDOW the
    // window manager's close button just kills the X connection instead of giving the app a
    // chance to react.
    x11_data.wm_delete_window = XInternAtom(x11_data.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x11_data.display, window, &x11_data.wm_delete_window, 1);

    XMapWindow(x11_data.display, window);
    XFlush(x11_data.display);

#if SKA_RENDERING
    opengl_init(fb_config);
#endif

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
#if SKA_RENDERING
    is_open_gl_initialized = false;
#endif
}

void ska_win_process_events(SkaWindow* window) {
    while (XPending(x11_data.display) > 0) {
        XEvent event;
        XNextEvent(x11_data.display, &event);
        switch (event.type) {
            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == x11_data.wm_delete_window) {
                    main_window.is_active = false;
                }
                break;
            }
            case ConfigureNotify: {
#if SKA_RENDERING
                if (is_open_gl_initialized) {
                    ska_win_update_size(&main_window, event.xconfigure.width, event.xconfigure.height);
                }
#endif
                break;
            }
#if SKA_INPUT
            case KeyPress:
            case KeyRelease: {
                const SkaInputKey key = ska_input_x11_keysym_to_keycode(x11_data.display, event.xkey.keycode);
                ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, key,
                    event.type == KeyPress ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED);
                break;
            }
            case ButtonPress:
            case ButtonRelease: {
                SkaInputKey mouse_key = SkaInputKey_INVALID;
                switch (event.xbutton.button) {
                    case Button1: mouse_key = SkaInputKey_MOUSE_BUTTON_LEFT; break;
                    case Button2: mouse_key = SkaInputKey_MOUSE_BUTTON_MIDDLE; break;
                    case Button3: mouse_key = SkaInputKey_MOUSE_BUTTON_RIGHT; break;
                    default: break; // scroll wheel = Button4/5, unhandled (matches win32 not handling WM_MOUSEWHEEL)
                }
                if (mouse_key != SkaInputKey_INVALID) {
                    ska_input_register_input_event3(SkaInputSourceType_MOUSE, mouse_key,
                        event.type == ButtonPress ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED);
                }
                break;
            }
            case MotionNotify: {
                const SkaVector2i new_position = { event.xmotion.x, event.xmotion.y };
                ska_input_register_mouse_move_event(new_position);
                break;
            }
#endif // SKA_INPUT
            default:
                break;
        }
    }
}

void ska_win_swap_window(SkaWindow* window) {
#if SKA_RENDERING
    ska_screen_capture_on_frame_end(window->width, window->height);
    glXSwapBuffers(x11_data.display, x11_data.window);
#endif
}

void ska_win_update_size(SkaWindow* window, int32 width, int32 height) {
    window->width = width;
    window->height = height;
#if SKA_RENDERING
    ska_renderer_update_window_size(width, height);
#endif
}

#endif // #if defined(PLATFORM_LINUX) && !defined(PLATFORM_EMSCRIPTEN)
