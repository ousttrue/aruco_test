#include "win32window.h"
#include <assert.h>
#include <Windowsx.h>


namespace win32 {


//////////////////////////////////////////////////////////////////////////////
// WndProc
//////////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK DelegateWndProc(
        HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // delegate
    return ((Window*)GetWindowLongPtr(
                hwnd, GWLP_USERDATA))->WndProc(hwnd, message, wParam, lParam);
}


static LRESULT CALLBACK InitialWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			{
				Window *window=(Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
                // set this pointer
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
                // repalce WndProc
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)DelegateWndProc);
				return window->WndProc(hwnd, message, wParam, lParam);
			}

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
}


//////////////////////////////////////////////////////////////////////////////
// WindowClass
//////////////////////////////////////////////////////////////////////////////
WindowClass::WindowClass(HINSTANCE hInstance, const tstring &className)
    : m_hInstance(hInstance), m_className(className), m_atom(0)
{
    ZeroMemory(&m_wndclass, sizeof(m_wndclass));
    m_wndclass.cbSize=sizeof(m_wndclass);
    m_wndclass.style = CS_HREDRAW | CS_VREDRAW;
    m_wndclass.lpfnWndProc = InitialWndProc;
    m_wndclass.cbClsExtra = 0;
    m_wndclass.cbWndExtra = 0;
    m_wndclass.hInstance = m_hInstance;
    m_wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    m_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    m_wndclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH) ;
    m_wndclass.lpszMenuName = NULL;
    m_wndclass.lpszClassName = m_className.c_str();
}

WindowClass::~WindowClass()
{
    if(m_atom){
        UnregisterClass((LPTSTR)MAKELONG(m_atom, 0), m_hInstance);
        m_atom=0;
    }
}

std::shared_ptr<WindowClass> WindowClass::Register(HINSTANCE hInstance, const tstring &className)
{
    auto wndClass=std::shared_ptr<WindowClass>(new WindowClass(hInstance, className));
    wndClass->m_atom=RegisterClassEx(&wndClass->m_wndclass);
    if(wndClass->m_atom==0){
        return nullptr;
    }
    return wndClass;
}


//////////////////////////////////////////////////////////////////////////////
// Window
//////////////////////////////////////////////////////////////////////////////
Window::Window(const std::shared_ptr<WindowClass> &windowClass)
    : m_windowClass(windowClass), m_hWnd(NULL)
    , m_mouseLeftDown(false)
    , m_mouseMiddleDown(false)
    , m_mouseRightDown(false)
    , m_mouseX(0)
    , m_mouseY(0)
{
}

Window::~Window()
{
}

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
			if (m_PaintCallback)
			{
				PAINTSTRUCT ps;
				auto hDC = BeginPaint(hWnd, &ps);
				m_PaintCallback(hDC);
				EndPaint(hWnd, &ps);
			}
			else{
				ValidateRect(hWnd, NULL);
			}
            return 0;

        case WM_ERASEBKGND:
            // ‰½‚à‚µ‚È‚¢
            return 1;

        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                for(auto it=m_ResizeCallbacks.begin(); it!=m_ResizeCallbacks.end(); ++it){
                    (*it)(width, height);
                }
            }
            return 0;

        case WM_KEYDOWN:
            {
                for(auto it=m_KeyDownCallbacks.begin(); it!=m_KeyDownCallbacks.end(); ++it){
                    (*it)(hWnd, static_cast<int>(wParam));
                }
            }
            break;

        case WM_LBUTTONDOWN:
            OnLButtonDown();
            return 0;

        case WM_LBUTTONUP: 
            OnLButtonUp();
            return 0;

        case WM_MBUTTONDOWN:
            OnMButtonDown();
            return 0;

        case WM_MBUTTONUP: 
            OnMButtonUp();
            return 0;

        case WM_RBUTTONDOWN:
            OnRButtonDown();
            return 0;

        case WM_RBUTTONUP: 
            OnRButtonUp();
            return 0;

        case WM_MOUSEMOVE: 
            OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_MOUSEWHEEL:
            OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Window::OnLButtonDown()
{
    m_mouseLeftDown=true;
}

void Window::OnLButtonUp()
{
    m_mouseLeftDown=false;
}

void Window::OnMButtonDown()
{
    m_mouseMiddleDown=true;
}

void Window::OnMButtonUp()
{
    m_mouseMiddleDown=false;
}

void Window::OnRButtonDown()
{
    m_mouseRightDown=true;
}

void Window::OnRButtonUp()
{
    m_mouseRightDown=false;
}

void Window::OnMouseMove(int x, int y)
{
    int dx=x-m_mouseX;
    int dy=y-m_mouseY;

    if(m_mouseLeftDown){
        for(auto it=onLeftMouseDragFuncs.begin(); it!=onLeftMouseDragFuncs.end(); ++it){
            (*it)(dx, dy);
        }
    }
    if(m_mouseMiddleDown){
        for(auto it=onMiddleMouseDragFuncs.begin(); it!=onMiddleMouseDragFuncs.end(); ++it){
            (*it)(dx, dy);
        }
    }
    if(m_mouseRightDown){
        for(auto it=onRightMouseDragFuncs.begin(); it!=onRightMouseDragFuncs.end(); ++it){
            (*it)(dx, dy);
        }
    }

    m_mouseX=x;
    m_mouseY=y;
}

void Window::OnMouseWheel(int d)
{
    for(auto it=onMouseWheelFuncs.begin(); it!=onMouseWheelFuncs.end(); ++it){
        (*it)(d);
    }
}


std::shared_ptr<Window> Window::Create(HINSTANCE hInstance
        , const std::shared_ptr<WindowClass> &windowClass
        , const tstring &windowName
        , int width, int height
        )
{
    auto w=std::shared_ptr<Window>(new Window(windowClass)); 

	auto style = WS_OVERLAPPEDWINDOW;

	// adjust client rect
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 640;
	r.bottom = 480;
	AdjustWindowRect(&r, style, FALSE);

    DWORD dwExStyle=0;
    w->m_hWnd=CreateWindowEx(dwExStyle,
            (LPTSTR)MAKELONG(windowClass->GetAtom(), 0), 
            windowName.c_str(),
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
			r.right - r.left, 
			r.bottom - r.top,
            NULL,
            NULL,
            hInstance,
            w.get());
    if(!w->m_hWnd){
        return nullptr;
    }

    return w;
}


//////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Window> RegisterAndCreateWindow(
        HINSTANCE hInstance, const tstring &className
        , const tstring &windowName, int width, int height
        )
{
    auto windowClass=WindowClass::Register(hInstance, className);
    if(!windowClass){
        return nullptr;
    }
    return Window::Create(hInstance, windowClass, windowName, width, height);
}

bool PollMessage()
{
    MSG msg;
    while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
        if(msg.message==WM_QUIT ){
            return false;
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    return true;
}


} // namespace
