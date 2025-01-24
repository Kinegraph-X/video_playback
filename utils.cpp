#include "utils.h" 

std::string GetExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

void subClassWindowProc(HWND hwnd) {
	logger(LogLevel::DEBUG, "subclassing window proc...");
    if (hwnd) {
		WNDPROC originalWindowProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
		SetProp(hwnd, ORIGINAL_WNDPROC_PROP, (HANDLE)originalWindowProc);
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)customizePlatformWindow);
    }
}

WindowSize getWindowBorders(HWND hwnd) {
	RECT wrect;
	GetWindowRect( hwnd, &wrect );
	RECT crect;
	GetClientRect( hwnd, &crect );
	POINT lefttop = { crect.left, crect.top }; // Practically both are 0
	ClientToScreen( hwnd, &lefttop );
	POINT rightbottom = { crect.right, crect.bottom };
	ClientToScreen( hwnd, &rightbottom );
	
	int left_border = lefttop.x - wrect.left; // Windows 10: includes transparent part
	int right_border = wrect.right - rightbottom.x; // As above
	int bottom_border = wrect.bottom - rightbottom.y; // As above
	int top_border_with_title_bar = lefttop.y - wrect.top; // There is no transparent part
	
	WindowSize heightStruct {
		0,
		top_border_with_title_bar
	};
	
	return heightStruct;
}