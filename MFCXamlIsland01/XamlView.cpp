// XamlView.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "XamlView.h"
#include "MFCXamlIsland01.h"
#include <ShellScalingApi.h>

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::Foundation;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;



const winrt::Windows::UI::Xaml::Thickness IslandMargin = {
	   40.0f /*Left*/,
	   10.0f /*Top*/,
	   40.0f /*Right*/,
	   0.0f /*Bottom*/
};
// XamlView

//IMPLEMENT_DYNAMIC(XamlView, CDockablePane)

XamlView::XamlView() noexcept
{

}

XamlView::~XamlView()
{
}
BEGIN_MESSAGE_MAP(XamlView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int XamlView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	
	
	AdjustLayout();

	return 0;
}


void XamlView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	CreateXamlHost();
	AdjustLayout();
} 
BOOL XamlView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void XamlView::OnPaint()
{	 
	CPaintDC dc(this); // device context for painting
	CRect rect;
	this->GetWindowRect(rect);
	ScreenToClient(rect);
	dc.Draw3dRect(rect, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));	 
}

void XamlView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	 
}
void XamlView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}
}
int XamlView::CreateXamlHost()
{
	//XAML Island section

	// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
	// to host UWP controls in any UI element that is associated with a window handle (HWND).

	// Get handle to corewindow
	auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();
	// Parent the DesktopWindowXamlSource object to current window
	check_hresult(interop->AttachToWindow(this->m_hWnd));

	// This Hwnd will be the window handler for the Xaml Island: A child window that contains Xaml.  
	hWndXamlIsland = nullptr;
	// Get the new child window's hwnd 
	interop->get_WindowHandle(&hWndXamlIsland);

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	::SetWindowPos(hWndXamlIsland, 0, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_SHOWWINDOW);
	// Update the xaml island window size becuase initially is 0,0
	//::SetWindowPos(hWndXamlIsland, 0, 0, 0, 200,800, SWP_SHOWWINDOW);

	//Creating the Xaml content

	Windows::UI::Xaml::Controls::StackPanel xamlContainer;
	//xamlContainer.Background(Windows::UI::Xaml::Media::SolidColorBrush{ Windows::UI::Colors::LightGray() });

	Windows::UI::Xaml::Controls::TreeView treeView;
	Windows::UI::Xaml::Controls::TreeViewNode rootNode;
	rootNode.Content(winrt::box_value(L"This is a node"));

	for (int i = 0; i < 15; i++)
	{
		Windows::UI::Xaml::Controls::TreeViewNode childNode;
		childNode.Content(winrt::box_value(L"Children"));
		rootNode.Children().Append(childNode);
	} 

	treeView.RootNodes().Append(rootNode);
	treeView.Expand(rootNode);

	xamlContainer.Children().Append(treeView);
	xamlContainer.UpdateLayout();
	desktopSource.Content(xamlContainer);

	//End XAML Island section

	return true;
}

void XamlView::CreateXamlHostV2()
{

	m_mainUserControl = winrt::MyApp::MainUserControl();
	m_hWndXamlIsland = wil::unique_hwnd(CreateDesktopWindowsXamlSource(0 /*| WS_THICKFRAME*/, m_mainUserControl));
	m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &XamlView::OnXamlLayoutUpdated });

}

void XamlView::OnXamlLayoutUpdated(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
{
	if (m_parentLayout)
	{
		return;
	}

	const auto dispatcher = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
	m_parentLayout = dispatcher.TryEnqueue(winrt::Windows::System::DispatcherQueuePriority::Low, { this, &XamlView::OnSizeXamlContentToParent });
}

void XamlView::OnSizeXamlContentToParent()
{
	//ASSERT(m_parentLayout);

	m_layoutUpdatedToken.revoke();
	auto reenableLayoutUpdatedHandler = wil::scope_exit_log(WI_DIAGNOSTICS_INFO, [&]()
	{
		m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &XamlView::OnXamlLayoutUpdated });
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
// XamlView message handlers


