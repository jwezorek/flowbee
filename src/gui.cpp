#include "gui.hpp"
#include "types.hpp"
#include "matrix.hpp"
#include "canvas.hpp"
#include "brush.hpp"
#include "util.hpp"
#include "paint_particle.hpp"
#include <windows.h>
#include <windowsx.h>

/*------------------------------------------------------------------------------------------------*/

namespace {

    flo::canvas g_canvas;
    HBITMAP g_hbm;
    bool g_is_dirty = true;
    auto g_brush = flo::create_simple_brush(flo::make_one_color_paint(5, 2, 20000.0), 5.0, 0.75, 0.1);
    bool g_stroke_in_progress = false;

    LRESULT handle_paint(HWND hwnd);
    LRESULT handle_mouse_down(HWND hwnd, WPARAM w_param, LPARAM l_param);
    LRESULT handle_mouse_move(HWND hwnd, WPARAM w_param, LPARAM l_param);
    LRESULT handle_mouse_up(HWND hwnd, WPARAM w_param, LPARAM l_param);
    LRESULT handle_key_down(HWND hwnd, WPARAM w_param, LPARAM l_param);

    LRESULT CALLBACK window_procedure(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
        switch (u_msg) {
        case WM_PAINT:
            return handle_paint(hwnd);

        case WM_LBUTTONDOWN:
            return handle_mouse_down(hwnd, w_param, l_param);

        case WM_MOUSEMOVE:
            return handle_mouse_move(hwnd, w_param, l_param);

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

    flo::point to_point(LPARAM l_param) {
        int x_pos = GET_X_LPARAM(l_param);
        int y_pos = GET_Y_LPARAM(l_param);

        // Convert the coordinates to double and store in point
        return { static_cast<double>(x_pos), static_cast<double>(y_pos) };
    }

    LRESULT handle_mouse_down(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        g_is_dirty = true;
        g_stroke_in_progress = true;
        flo::apply_brush(g_canvas, g_brush, to_point(l_param), 1.0, 4);
        InvalidateRect(hwnd, NULL, false);
        return 0;
    }

    LRESULT handle_mouse_move(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        if (g_stroke_in_progress) {
            g_is_dirty = true;
            flo::apply_brush(g_canvas, g_brush, to_point(l_param), 1.0, 4);
            InvalidateRect(hwnd, NULL, false);
        }
        return 0;
    }

    LRESULT handle_mouse_up(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        g_stroke_in_progress = false;
        InvalidateRect(hwnd, NULL, false); 
        return 0;
    }

    LRESULT handle_key_down(HWND hwnd, WPARAM w_param, LPARAM l_param) {
        // Placeholder for key down logic
        return 0;
    }

    void ResizeWindowToClientSize(HWND hWnd, int desiredClientWidth, int desiredClientHeight) {
        // Get the current window style and extended style
        DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
        DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

        // Get the current window rectangle
        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);

        // Create a rectangle representing the desired client area
        RECT desiredRect = { 0, 0, desiredClientWidth, desiredClientHeight };

        // Adjust the rectangle for the window's non-client area
        AdjustWindowRectEx(&desiredRect, dwStyle, FALSE, dwExStyle);

        // Calculate the width and height for the entire window
        int newWindowWidth = desiredRect.right - desiredRect.left;
        int newWindowHeight = desiredRect.bottom - desiredRect.top;

        // Resize the window, keeping the current position
        SetWindowPos(hWnd, nullptr,
            windowRect.left, windowRect.top,
            newWindowWidth, newWindowHeight,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }

    HBITMAP img_to_bmp(const flo::image& mat) {
        const int width = mat.cols();
        const int height = mat.rows();

        // Define the bitmap information header
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // Negative to create a top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32; // 32 bits per pixel
        bmi.bmiHeader.biCompression = BI_RGB; // No compression
        bmi.bmiHeader.biSizeImage = 0; // Can be 0 for BI_RGB

        // Pointer to the pixel data in the DIB
        void* dibPixels = nullptr;

        // Create a DIB section
        HBITMAP hBitmap = CreateDIBSection(
            nullptr,              // Use the default device context
            &bmi,                 // Bitmap information
            DIB_RGB_COLORS,       // Color table type (not used for 32-bit)
            &dibPixels,           // Pointer to the actual pixel data
            nullptr,              // No file mapping
            0                     // File mapping offset
        );

        if (!hBitmap) {
            return nullptr; // Return nullptr if bitmap creation failed
        }

        // Pointer to the pixel data in the matrix
        const uint32_t* srcPixels = reinterpret_cast<const uint32_t*>(mat.data());
        uint32_t* dstPixels = reinterpret_cast<uint32_t*>(dibPixels);

        // Copy and convert the pixel data from RGBA to BGRA
        for (int i = 0; i < width * height; ++i) {
            uint32_t pixel = srcPixels[i];
            uint32_t r = (pixel & 0x000000FF);
            uint32_t g = (pixel & 0x0000FF00);
            uint32_t b = (pixel & 0x00FF0000);
            uint32_t a = (pixel & 0xFF000000);
            dstPixels[i] = (b >> 16) | g | (r << 16) | a; // Swap R and B channels
        }

        return hBitmap;
    }

    LRESULT handle_paint(HWND hwnd) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        if (g_is_dirty) {
            g_hbm = img_to_bmp(flo::canvas_to_image(g_canvas));
        }

        HDC hdc_scr = GetDC(NULL);
        HDC hdc_bmp = CreateCompatibleDC(hdc_scr);
        auto hbm_old = SelectObject(hdc_bmp, g_hbm);

        BitBlt(hdc, 0, 0, g_canvas.cols(), g_canvas.rows(), hdc_bmp, 0, 0, SRCCOPY);

        SelectObject(hdc_bmp, hbm_old);
        DeleteDC(hdc_bmp);
        ReleaseDC(NULL, hdc_scr);

        EndPaint(hwnd, &ps);

        g_is_dirty = false;

        return 0;
    }

}

void flo::do_gui(const std::string& img_file, int n) {

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

    g_canvas = flo::image_to_canvas(flo::img_from_file(img_file), n);
    auto dims = g_canvas.bounds();
    ResizeWindowToClientSize(hwnd, dims.wd, dims.hgt);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}