#include "pch.h"

#include "XamlBridge.h"
#include "Resource.h"

bool DesktopWindow::FilterMessage(const MSG* msg)
{
    // When multiple child windows are present it is needed to pre dispatch messages to all 
    // DesktopWindowXamlSource instances so keyboard accelerators and 
    // keyboard focus work correctly.
    BOOL xamlSourceProcessedMessage = FALSE;
    {
        for (auto xamlSource : m_xamlSources)
        {
            auto xamlSourceNative2 = xamlSource.as<IDesktopWindowXamlSourceNative2>();
            const auto hr = xamlSourceNative2->PreTranslateMessage(msg, &xamlSourceProcessedMessage);
            winrt::check_hresult(hr);
            if (xamlSourceProcessedMessage)
            {
                break;
            }
        }
    }

    return !!xamlSourceProcessedMessage;
}

const auto static invalidReason = static_cast<winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason>(-1);

winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason GetReasonFromKey(WPARAM key)
{
    auto reason = invalidReason;
    if (key == VK_TAB)
    {
        byte keyboardState[256] = {};
        WINRT_VERIFY(::GetKeyboardState(keyboardState));
        reason = (keyboardState[VK_SHIFT] & 0x80) ?
            winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Last :
            winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First;
    }
    else if (key == VK_LEFT)
    {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Left;
    }
    else if (key == VK_RIGHT)
    {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right;
    }
    else if (key == VK_UP)
    {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Up;
    }
    else if (key == VK_DOWN)
    {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down;
    }
    return reason;
}

winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource DesktopWindow::GetNextFocusedIsland(MSG* msg)
{
    if (msg->message == WM_KEYDOWN)
    {
        const auto key = msg->wParam;
        auto reason = GetReasonFromKey(key);
        if (reason != invalidReason)
        {
            const BOOL previous =
                (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First ||
                    reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down ||
                    reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right) ? false : true;

            const auto currentFocusedWindow = ::GetFocus();
            const auto nextElement = ::GetNextDlgTabItem(m_hMainWnd.get(), currentFocusedWindow, previous);
            for (auto xamlSource : m_xamlSources)
            {
                const auto nativeIsland = xamlSource.as<IDesktopWindowXamlSourceNative>();
                HWND islandWnd = nullptr;
                winrt::check_hresult(nativeIsland->get_WindowHandle(&islandWnd));
                if (nextElement == islandWnd)
                {
                    return xamlSource;
                }
            }
        }
    }

    return nullptr;
}

winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource DesktopWindow::GetFocusedIsland()
{
    for (auto xamlSource : m_xamlSources)
    {
        if (xamlSource.HasFocus())
        {
            return xamlSource;
        }
    }
    return nullptr;
}

bool DesktopWindow::NavigateFocus(MSG* msg)
{
    if (const auto nextFocusedIsland = GetNextFocusedIsland(msg))
    {
        const auto previousFocusedWindow = ::GetFocus();
        RECT rect = {};
        WINRT_VERIFY(::GetWindowRect(previousFocusedWindow, &rect));
        const auto nativeIsland = nextFocusedIsland.as<IDesktopWindowXamlSourceNative>();
        HWND islandWnd = nullptr;
        winrt::check_hresult(nativeIsland->get_WindowHandle(&islandWnd));
        POINT pt = { rect.left, rect.top };
        SIZE size = { rect.right - rect.left, rect.bottom - rect.top };
        ::ScreenToClient(islandWnd, &pt);
        const auto hintRect = winrt::Windows::Foundation::Rect({ static_cast<float>(pt.x), static_cast<float>(pt.y), static_cast<float>(size.cx), static_cast<float>(size.cy) });
        const auto reason = GetReasonFromKey(msg->wParam);
        const auto request = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationRequest(reason, hintRect);
        lastFocusRequestId = request.CorrelationId();
        const auto result = nextFocusedIsland.NavigateFocus(request);
        return result.WasFocusMoved();
    }
    else
    {
        const bool islandIsFocused = GetFocusedIsland() != nullptr;
        byte keyboardState[256] = {};
        WINRT_VERIFY(::GetKeyboardState(keyboardState));
        const bool isMenuModifier = keyboardState[VK_MENU] & 0x80;
        if (islandIsFocused && !isMenuModifier)
        {
            return false;
        }
        const bool isDialogMessage = !!IsDialogMessage(m_hMainWnd.get(), msg);
        return isDialogMessage;
    }
}

int DesktopWindow::MessageLoop(HACCEL hAccelTable)
{
    MSG msg = {};
    HRESULT hr = S_OK;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        const bool xamlSourceProcessedMessage = FilterMessage(&msg);
        if (!xamlSourceProcessedMessage && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            if (!NavigateFocus(&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return (int)msg.wParam;
}

static const WPARAM invalidKey = (WPARAM)-1;

WPARAM GetKeyFromReason(winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason reason)
{
    auto key = invalidKey;
    if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Last || reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First)
    {
        key = VK_TAB;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Left)
    {
        key = VK_LEFT;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right)
    {
        key = VK_RIGHT;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Up)
    {
        key = VK_UP;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down)
    {
        key = VK_DOWN;
    }
    return key;
}

void DesktopWindow::OnTakeFocusRequested(winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource const& sender, winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const& args)
{
    if (args.Request().CorrelationId() != lastFocusRequestId)
    {
        const auto reason = args.Request().Reason();
        const BOOL previous =
            (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First ||
                reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down ||
                reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right) ? false : true;

        const auto nativeXamlSource = sender.as<IDesktopWindowXamlSourceNative>();
        HWND senderHwnd = nullptr;
        winrt::check_hresult(nativeXamlSource->get_WindowHandle(&senderHwnd));

        MSG msg = {};
        msg.hwnd = senderHwnd;
        msg.message = WM_KEYDOWN;
        msg.wParam = GetKeyFromReason(reason);
        if (!NavigateFocus(&msg))
        {
            const auto nextElement = ::GetNextDlgTabItem(m_hMainWnd.get(), senderHwnd, previous);
            ::SetFocus(nextElement);
        }
    }
    else
    {
        const auto request = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationRequest(winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Restore);
        lastFocusRequestId = request.CorrelationId();
        sender.NavigateFocus(request);
    }
}

HWND DesktopWindow::CreateDesktopWindowsXamlSource(DWORD dwStyle, winrt::Windows::UI::Xaml::UIElement content)
{
    HRESULT hr = S_OK;

    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource desktopSource;

    auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();
    // Parent the DesktopWindowXamlSource object to current window
    hr = interop->AttachToWindow(m_hMainWnd.get());
    winrt::check_hresult(hr);

    // Get the new child window's hwnd 
    HWND hWndXamlIsland = nullptr;
    hr = interop->get_WindowHandle(&hWndXamlIsland);
    winrt::check_hresult(hr);
    DWORD dwNewStyle = GetWindowLong(hWndXamlIsland, GWL_STYLE);
    dwNewStyle |= dwStyle;
    SetWindowLong(hWndXamlIsland, GWL_STYLE, dwNewStyle);

    desktopSource.Content(content);

    m_takeFocusEventRevokers.push_back(desktopSource.TakeFocusRequested(winrt::auto_revoke, { this, &DesktopWindow::OnTakeFocusRequested }));

    m_xamlSources.push_back(desktopSource);

    return hWndXamlIsland;
}

void DesktopWindow::ClearXamlIslands()
{
    for (auto& takeFocusRevoker : m_takeFocusEventRevokers)
    {
        takeFocusRevoker.revoke();
    }
    m_takeFocusEventRevokers.clear();

    for (auto xamlSource : m_xamlSources)
    {
        xamlSource.Close();
    }
    m_xamlSources.clear();
}

winrt::Windows::UI::Xaml::UIElement LoadXamlControl(uint32_t id)
{
    auto rc = ::FindResource(nullptr, MAKEINTRESOURCE(id), MAKEINTRESOURCE(XAMLRESOURCE));
    if (!rc)
    {
        winrt::check_hresult(HRESULT_FROM_WIN32(GetLastError()));
    }
    HGLOBAL rcData = ::LoadResource(nullptr, rc);
    if (!rcData)
    {
        winrt::check_hresult(HRESULT_FROM_WIN32(GetLastError()));
    }
    auto pData = static_cast<wchar_t*>(::LockResource(rcData));
    auto content = winrt::Windows::UI::Xaml::Markup::XamlReader::Load(winrt::get_abi(pData));
    auto uiElement = content.as<winrt::Windows::UI::Xaml::UIElement>();
    return uiElement;
}
