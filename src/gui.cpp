#include "gui.hpp"
#include "types.hpp"
#include "matrix.hpp"
#include <windows.h>
#include <windowsx.h>

namespace {
    LRESULT handle_paint(HWND hwnd);
    LRESULT handle_mouse_down(HWND hwnd, WPARAM w_param, LPARAM l_param);
    LRESULT handle_mouse_up(HWND hwnd, WPARAM w_param, LPARAM l_param);
    LRESULT handle_key_down(HWND hwnd, WPARAM w_param, LPARAM l_param);

    LRESULT CALLBACK window_procedure(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
        switch (u_msg) {
        case WM_PAINT:
            return handle_paint(hwnd);

        case WM_LBUTTONDOWN:
            return handle_mouse_down(hwnd, w_param, l_param);

        case WM_LBUTTONUP:
            return handle_mouse_up(hwnd, w_param, l_param);

        case WM_KEYDOWN:
            return handle_key_down(hwnd, w_param, l_param);

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, u_msg, w_param, l_param);
        }
    }

    LRESULT handle_paint(HWND hwnd) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // Placeholder for painting logic
        EndPaint(hwnd, &ps);
        return 0;
    }

    LRESULT handle_mouse_down(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        // Placeholder for mouse down logic
        return 0;
    }

    LRESULT handle_mouse_up(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        // Placeholder for mouse up logic
        return 0;
    }

    LRESULT handle_key_down(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        // Placeholder for key down logic
        return 0;
    }
}

void flo::do_gui() {
    const char class_name[] = "flowbee_window_class";
    HINSTANCE h_instance = GetModuleHandle(NULL);
    WNDCLASS wc = {};
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = h_instance;
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        class_name,                     // Window class
        "flowbee",                     // Window name
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,                          // Parent window
        NULL,                          // Menu
        h_instance,                    // Instance handle
        NULL                           // Additional application data
    );

    if (!hwnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}