/**********************************************************************************************
 * rcore_desktop_gtk - stubbed platform implementation
 *
 * A compact, single-version GTK/desktop stub that provides the
 * minimal platform API used by the rest of the codebase so the
 * project can be configured and built without GLFW present.
 *
 * This file intentionally avoids any GLFW symbols or APIs.
 **********************************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

extern CoreData CORE; /* defined in core module */

/* Minimal opaque platform handle placeholder */
typedef struct { void *ptr; } PlatformData;
static PlatformData platform = { NULL };

/* Forward declarations */
int InitPlatform(void);
void ClosePlatform(void);

bool WindowShouldClose(void)
{
    return CORE.Window.ready ? CORE.Window.shouldClose : true;
}

void ToggleFullscreen(void)
{
    CORE.Window.fullscreen = !CORE.Window.fullscreen;
    if (CORE.Window.fullscreen) CORE.Window.flags |= FLAG_FULLSCREEN_MODE;
    else CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;
}

// Set window state: minimized
void MinimizeWindow(void)
{
    CORE.Window.flags |= FLAG_WINDOW_MINIMIZED;
}

void SwapScreenBuffer(void) { /* No native buffer swap in stub */ }

double GetTime(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) return (double)ts.tv_sec + (double)ts.tv_nsec/1e9;
    return 0.0;
}

int InitPlatform(void)
{
    /* Basic initialization so other systems relying on CORE see a valid state */
    CORE.Window.ready = true;

    if (CORE.Window.display.width == 0) CORE.Window.display.width = (CORE.Window.screen.width > 0) ? CORE.Window.screen.width : 800;
    if (CORE.Window.display.height == 0) CORE.Window.display.height = (CORE.Window.screen.height > 0) ? CORE.Window.screen.height : 600;

    if (CORE.Window.screen.width == 0) CORE.Window.screen.width = CORE.Window.display.width;
    if (CORE.Window.screen.height == 0) CORE.Window.screen.height = CORE.Window.display.height;

    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;

    CORE.Window.shouldClose = false;

    InitTimer();
    CORE.Storage.basePath = GetWorkingDirectory();

    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (GTK STUB): Initialized");

    return 0;
}

void ClosePlatform(void)
{
    CORE.Window.ready = false;
}

void PollInputEvents(void)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
    UpdateGestures();
#endif

    /* Reset transient input state (keyboard, mouse, touch, gamepads) */
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;
    CORE.Input.Gamepad.lastButtonPressed = 0;

    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];

    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        CORE.Input.Gamepad.ready[i] = false;
        CORE.Input.Gamepad.axisCount[i] = 0;
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];
    }

    CORE.Window.resizedLastFrame = false;
}

/* Minimal implementations of other platform helpers used across the codebase */
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

void SetMouseCursor(int cursor) { CORE.Input.Mouse.cursor = cursor; }

const char *GetKeyName(int key) { (void)key; return ""; }

void OpenURL(const char *url)
{
    if (!url) return;
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL contains '\'' - refusing to run shell command");
    else
    {
        size_t len = strlen(url) + 32;
        char *cmd = (char *)RL_CALLOC(len, sizeof(char));
#if defined(_WIN32)
        sprintf(cmd, "explorer \"%s\"", url);
#elif defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
#else
        sprintf(cmd, "xdg-open '%s'", url);
#endif
        int result = system(cmd);
        if (result == -1) TRACELOG(LOG_WARNING, "OpenURL() child process could not be created");
        RL_FREE(cmd);
    }
}

int SetGamepadMappings(const char *mappings) { (void)mappings; return 0; }
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{ (void)gamepad; (void)leftMotor; (void)rightMotor; (void)duration; TRACELOG(LOG_WARNING, "SetGamepadVibration() not available on stub platform"); }

/* Monitor/window helpers return conservative defaults */
int GetMonitorCount(void) { return 1; }
int GetCurrentMonitor(void) { return 0; }
Vector2 GetMonitorPosition(int monitor) { (void)monitor; return (Vector2){ 0.0f, 0.0f }; }
int GetMonitorWidth(int monitor) { (void)monitor; return (int)(CORE.Window.display.width ? CORE.Window.display.width : CORE.Window.screen.width); }
int GetMonitorHeight(int monitor) { (void)monitor; return (int)(CORE.Window.display.height ? CORE.Window.display.height : CORE.Window.screen.height); }
const char *GetMonitorName(int monitor) { (void)monitor; return ""; }
Vector2 GetWindowPosition(void) { return (Vector2){ (float)CORE.Window.position.x, (float)CORE.Window.position.y }; }
Vector2 GetWindowScaleDPI(void) { return (Vector2){ 1.0f, 1.0f }; }

void SetWindowTitle(const char *title) { CORE.Window.title = title; }
void SetWindowSize(int width, int height)
{
    CORE.Window.screen.width = width; CORE.Window.screen.height = height;
    CORE.Window.render.width = width; CORE.Window.render.height = height;
}

void SetWindowPosition(int x, int y) { CORE.Window.position.x = x; CORE.Window.position.y = y; }
void SetWindowMinSize(int width, int height) { CORE.Window.screenMin.width = width; CORE.Window.screenMin.height = height; }
void SetWindowMaxSize(int width, int height) { CORE.Window.screenMax.width = width; CORE.Window.screenMax.height = height; }
void SetWindowState(unsigned int flags) { CORE.Window.flags |= flags; }
void ClearWindowState(unsigned int flags) { CORE.Window.flags &= ~flags; }

/* No-op clipboard stubs */
void SetClipboardText(const char *text) { (void)text; }
const char *GetClipboardText(void) { return ""; }
Image GetClipboardImage(void) { Image img = {0}; return img; }

/* Cursor helpers */
void ShowCursor(void) { CORE.Input.Mouse.cursorHidden = false; }
void HideCursor(void) { CORE.Input.Mouse.cursorHidden = true; }
void EnableCursor(void) { SetMousePosition((int)CORE.Window.screen.width/2, (int)CORE.Window.screen.height/2); CORE.Input.Mouse.cursorLocked = false; }
void DisableCursor(void) { SetMousePosition((int)CORE.Window.screen.width/2, (int)CORE.Window.screen.height/2); CORE.Input.Mouse.cursorLocked = true; }

/* End of stub */

