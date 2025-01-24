#include <customizePlatformWindow.h>

LRESULT CALLBACK customizePlatformWindow(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//	logger(LogLevel::DEBUG, "Window message received : " + LogUtils::toString(msg));
	COLORREF titlebar_color = 0x22181205;
	switch (msg) {
		char buffer[128];
		sprintf_s(buffer, "Received message: 0x%04X\n", msg);
		OutputDebugStringA(buffer);
		
        case WM_ACTIVATE:
        	logger(LogLevel::DEBUG, "The window has received ACTIVATE message");
            // Handle title-bar color change on activation
            if (wParam != WA_INACTIVE) {
				logger(LogLevel::DEBUG, "The window isn't inactive, changing title bar color");
				
//				COLORREF titlebar_color = 0x051218;
//				DwmSetWindowAttribute(
//					hwnd, DWMWINDOWATTRIBUTE::DWMWA_BORDER_COLOR,
//					&titlebar_color, sizeof(titlebar_color)
//				);
//			
//				DwmSetWindowAttribute(
//					hwnd, DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR,
//					&titlebar_color, sizeof(titlebar_color)
//				);
			}
            break;

        // Handle other messages as needed
        
        default:
        	break;
    }

    // Call the original window procedure
    WNDPROC originalWindowProc = (WNDPROC)GetProp(hwnd, ORIGINAL_WNDPROC_PROP);
    if (originalWindowProc) {
        // Forward the message to the original procedure
        return CallWindowProc(originalWindowProc, hwnd, msg, wParam, lParam);
    }

    // Fallback to default processing if originalProc is missing
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
