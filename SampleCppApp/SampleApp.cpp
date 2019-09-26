#include "pch.h"

#include "framework.h"
#include "SampleApp.h"
#include "XamlBridge.h"
#include <ShellScalingApi.h>
#include <winrt/Microsoft.Toolkit.Win32.UI.XamlHost.h>
using namespace winrt::Windows::UI::Xaml::Hosting;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst = nullptr; // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

class MyWindow : public DesktopWindowT<MyWindow>
{
public:
    MyWindow(HINSTANCE hInstance, int nCmdShow) noexcept
    {
        LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
        LoadStringW(hInstance, IDC_SAMPLECPPAPP, szWindowClass, MAX_LOADSTRING);

        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = DLGWINDOWEXTRA;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SAMPLECPPAPP));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        //wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SAMPLECPPAPP);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        WINRT_VERIFY(RegisterClassEx(&wcex));
        WINRT_ASSERT(!GetHandle());

        const auto wnd = InitInstance(hInstance, nCmdShow);
        WINRT_ASSERT(wnd);
    }

#define HANDLE_WM_EXITSIZEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)

    LRESULT MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) noexcept
    {
        HRESULT hr = S_OK;

        switch (message)
        {
            HANDLE_MSG(GetHandle(), WM_CREATE, OnCreate);
            HANDLE_MSG(GetHandle(), WM_COMMAND, OnCommand);
            HANDLE_MSG(GetHandle(), WM_DESTROY, OnDestroy);
            HANDLE_MSG(GetHandle(), WM_EXITSIZEMOVE, OnResize);
        default:
            return base_type::MessageHandler(message, wParam, lParam);
        }

        return base_type::MessageHandler(message, wParam, lParam);
    }

private:

    bool m_parentLayout = false;
    wil::unique_hwnd m_hWndXamlIsland = nullptr;
    winrt::MyApp::MainUserControl m_mainUserControl = nullptr;
    winrt::Windows::UI::Xaml::FrameworkElement::LayoutUpdated_revoker m_layoutUpdatedToken{};

    HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
    {
        hInst = hInstance; // Store instance handle in our global variable
        HWND hMainWnd = CreateWindow(
            szWindowClass,
            szTitle,
            WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, InitialWidth, InitialHeight,
            nullptr, nullptr, hInstance, this);

        if (!hMainWnd)
        {
            winrt::check_hresult(E_FAIL);
        }

        ShowWindow(hMainWnd, nCmdShow);
        UpdateWindow(hMainWnd);
        SetFocus(hMainWnd);
        return hMainWnd;
    }

    bool OnCreate(HWND, LPCREATESTRUCT)
    {
	    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();

        m_mainUserControl = winrt::MyApp::MainUserControl();
        m_hWndXamlIsland = wil::unique_hwnd(CreateDesktopWindowsXamlSource(0 /*| WS_THICKFRAME*/, m_mainUserControl));
        m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &MyWindow::OnXamlLayoutUpdated });

        return true;
    }

    void OnResize(HWND, UINT state, int cx, int cy)
    {
        if (m_parentLayout)
        {
            // We can't change while we are also changing the parent window
            return;
        }

        m_layoutUpdatedToken.revoke();
        auto reenableLayoutUpdatedHandler = wil::scope_exit_log(WI_DIAGNOSTICS_INFO, [&]()
            {
                m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &MyWindow::OnXamlLayoutUpdated });
            });

        // Change layout properties of the Xaml Island here
        // Do not call ::SetWindowPos, it will be called as a response to UpdateLayout
    }

    void OnXamlLayoutUpdated(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
    {
        if (m_parentLayout)
        {
            return;
        }

        const auto dispatcher = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
        m_parentLayout = dispatcher.TryEnqueue(winrt::Windows::System::DispatcherQueuePriority::Low, { this, &MyWindow::OnSizeXamlContentToParent });
    }


    // If you prefer no mergin, then:
    //const winrt::Windows::UI::Xaml::Thickness IslandMargin = { }
    const winrt::Windows::UI::Xaml::Thickness IslandMargin = {
        40.0f /*Left*/,
        10.0f /*Top*/,
        40.0f /*Right*/,
        0.0f /*Bottom*/
    };

    void OnSizeXamlContentToParent()
    {
        //ASSERT(m_parentLayout);

        m_layoutUpdatedToken.revoke();
        auto reenableLayoutUpdatedHandler = wil::scope_exit_log(WI_DIAGNOSTICS_INFO, [&]()
            {
                m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &MyWindow::OnXamlLayoutUpdated });
                m_parentLayout = false;
            });

        winrt::Windows::Foundation::Size currentSize(5000, 5000);
        m_mainUserControl.Measure(currentSize);
        auto size = m_mainUserControl.DesiredSize();

        // Ok, a bit of an explanation before the next line. The call to Measure will turn on Xaml's "dirty" flag,
        // which means the LayoutUpdated event will be triggered on the next tick. We don't want that.
        // By calling UpdateLayout while the token is nullified, it will clear the flag and we won't get another call here
        // because of the Measure.
        m_mainUserControl.UpdateLayout();

        DEVICE_SCALE_FACTOR scaleFactor = {};
        winrt::check_hresult(GetScaleFactorForMonitor(MonitorFromWindow(GetHandle(), 0), &scaleFactor));
        const auto dpi = static_cast<int>(scaleFactor) / 100.0f;
        const int physicalWidth = static_cast<int>((size.Width * dpi) + 0.5f);
        const int physicalHeight = static_cast<int>((size.Height * dpi) + 0.5f);

        const int marginLeft = static_cast<int>((IslandMargin.Left * dpi) + 0.5f);
        const int marginRight = static_cast<int>((IslandMargin.Right * dpi) + 0.5f);
        const int marginTop = static_cast<int>((IslandMargin.Top * dpi) + 0.5f);
        const int marginBottom = static_cast<int>((IslandMargin.Bottom * dpi) + 0.5f);

        RECT windowRect = {};
        THROW_LAST_ERROR_IF(!::GetWindowRect(GetHandle(), &windowRect));
        const auto windowHeight = windowRect.bottom - windowRect.top;
        const auto windowWidth = windowRect.right - windowRect.left;

        RECT clientRect = {};
        THROW_LAST_ERROR_IF(!::GetClientRect(GetHandle(), &clientRect));
        const auto clientHeight = clientRect.bottom - clientRect.top;
        const auto clientWidth = clientRect.right - clientRect.left;

        const auto nonClientHeight = windowHeight - clientHeight;
        const auto nonClientWidth = windowWidth - clientWidth;

        const auto parentWindowWidth = physicalWidth + marginLeft + marginRight + nonClientWidth;
        const auto parentWindowHeight = physicalHeight + marginTop + marginBottom + nonClientHeight;
        THROW_LAST_ERROR_IF(!::SetWindowPos(m_hWndXamlIsland.get(), NULL, marginLeft, marginTop, physicalWidth, physicalHeight, SWP_SHOWWINDOW));

        // Comment this line to NOT force a size on the main window
        THROW_LAST_ERROR_IF(!::SetWindowPos(GetHandle(), nullptr, 0, 0, parentWindowWidth, parentWindowHeight, SWP_NOMOVE | SWP_NOACTIVATE));
    }

    void OnCommand(HWND, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), GetHandle(), About);
            break;
        case IDM_EXIT:
            PostQuitMessage(0);
            break;
        }
    }

    void OnDestroy(HWND hwnd)
    {
        base_type::OnDestroy(hwnd);
    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    winrt::init_apartment(winrt::apartment_type::single_threaded);

    winrt::MyApp::App app;

    MyWindow myWindow(hInstance, nCmdShow);
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SAMPLECPPAPP));
    int retValue = myWindow.MessageLoop(hAccelTable);

    app.Close();
    app = nullptr;

    return retValue;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
