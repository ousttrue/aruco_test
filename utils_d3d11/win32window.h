#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <list>

namespace win32 {


#ifdef UNICODE
    typedef std::wstring tstring;
#else
    typedef std::string tstring;
#endif


class WindowClass
{
    HINSTANCE m_hInstance;
    tstring m_className;
	WNDCLASSEX m_wndclass;
    ATOM m_atom;

	WindowClass(HINSTANCE hInstance, const tstring &className);
public:
	~WindowClass();
    ATOM GetAtom()const{ return m_atom; }

    static std::shared_ptr<WindowClass> Register(HINSTANCE hInstance
            , const tstring &className);
};


class Window
{
    std::shared_ptr<WindowClass> m_windowClass;
    HWND m_hWnd;

    // mouse
    bool m_mouseLeftDown;
    bool m_mouseMiddleDown;
    bool m_mouseRightDown;
    int m_mouseX;
    int m_mouseY;
    typedef std::function<void(int,int)> MouseFunc;

    Window(const std::shared_ptr<WindowClass> &windowClass);
public:
    ~Window();
    HWND GetHandle()const{ return m_hWnd; }
    LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    static std::shared_ptr<Window> Create(HINSTANCE hInstance
            , const std::shared_ptr<WindowClass> &windowClass
            , const tstring &windowName
            , int width
            , int height
            );

    typedef std::function<void(int, int)> ResizeCallback_t;
    std::list<ResizeCallback_t> m_ResizeCallbacks;
    void addResizeCallback(const ResizeCallback_t &callback)
    {
        m_ResizeCallbacks.push_back(callback); 
    }

    typedef std::function<void(HWND, int)> KeyDownCallback_t;
    std::list<KeyDownCallback_t> m_KeyDownCallbacks;
    void addKeyDownCallback(const KeyDownCallback_t &callback)
    {
        m_KeyDownCallbacks.push_back(callback);
    }

    std::list<MouseFunc> onLeftMouseDragFuncs;
    std::list<MouseFunc> onMiddleMouseDragFuncs;
    std::list<MouseFunc> onRightMouseDragFuncs;
    std::list<std::function<void(int)>> onMouseWheelFuncs;
    void OnLButtonDown();
    void OnLButtonUp();
    void OnMButtonDown();
    void OnMButtonUp();
    void OnRButtonDown();
    void OnRButtonUp();
    void OnMouseMove(int x, int y);
    void OnMouseWheel(int d);

	typedef std::function<void(HDC)> PaintCallback_t;
	PaintCallback_t m_PaintCallback;
	void setPaintCallback(const PaintCallback_t &callback){ m_PaintCallback = callback; }
};


std::shared_ptr<Window> RegisterAndCreateWindow(
        HINSTANCE hInstance, const tstring &className
        , const tstring &windowName, int width, int height
        );

bool PollMessage();

} // namespace

