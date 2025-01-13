#if SKA_UI

#include "ui.h"

#include "seika/math/math.h"

typedef uint32 SUIWidgetID;

typedef enum SUIMouseButton {
    SUIMouseButton_LEFT,
    SUIMouseButton_MIDDLE,
    SUIMouseButton_RIGHT,
} SUIMouseButton;

typedef struct SUIMouseState {
    SkaVector2 position;
    // left, middle, right
    bool button_pressed[3];
    bool button_just_pressed[3];
    bool button_just_released[3];
} SUIMouseState;

typedef struct SUIWidgetState {
    SUIWidgetID active_widget;
    SUIWidgetID hovered_widget;
} SUIWidgetState;

typedef struct SUIWindow {
    struct SUIWindow* parent;
    struct SUIWindow** children;
} SUIWindow;

typedef struct SUIContext {
    SUIMouseState mouse_state;
    SUIWidgetState widget_state;
    SUIWidgetID widget_counter;
} SUIContext;

static SUIContext context;


void sui_init() {
    context = (SUIContext){0};
}

void sui_finalize() {}

void sui_new_frame() {}

void sui_render() {}

bool sui_begin_window(const char* label, SUIWindowSettings* settings) {
    return false;
}

bool sui_button(const char* label) {
    return false;
}


#endif // #if SKA_UI
