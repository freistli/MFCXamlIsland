
#include "pch.h"
#include "framework.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "MFCXamlIsland01.h"
#include <ShellScalingApi.h>



using namespace winrt;
using namespace Windows::UI;
using namespace Windows::Foundation;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const winrt::Windows::UI::Xaml::Thickness IslandMargin = {
	   40.0f /*Left*/,
	   10.0f /*Top*/,
	   40.0f /*Right*/,
	   0.0f /*Bottom*/
};
/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView() noexcept
{ 
}

CFileView::~CFileView()
{
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers
int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// Fill in some static tree view data (dummy code, nothing magic here)
	//FillFileView();
	CreateXamlHostV2();
		
	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}
/*
int CFileView::CreateXamlHost()
{
	//XAML Island section

	// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
	// to host UWP controls in any UI element that is associated with a window handle (HWND).

	// Get handle to corewindow
	auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();
	// Parent the DesktopWindowXamlSource object to current window
	check_hresult(interop->AttachToWindow(this->GetSafeHwnd()));

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
	rootNode.IsExpanded(true);
	xamlContainer.Children().Append(treeView);
	xamlContainer.UpdateLayout();
	desktopSource.Content(xamlContainer);

	//End XAML Island section

	return true;
}
*/
void CFileView::CreateXamlHostV2()
{	 

	m_mainUserControl = winrt::MyApp::MainUserControl();
	m_hWndXamlIsland = wil::unique_hwnd(CreateDesktopWindowsXamlSource(this->GetSafeHwnd(),0 /*| WS_THICKFRAME*/, m_mainUserControl));
	m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &CFileView::OnXamlLayoutUpdated });

}

void CFileView::OnXamlLayoutUpdated(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
{
	if (m_parentLayout)
	{
		return;
	}

	const auto dispatcher = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
	m_parentLayout = dispatcher.TryEnqueue(winrt::Windows::System::DispatcherQueuePriority::Low, { this, &CFileView::OnSizeXamlContentToParent });
}

void CFileView::OnSizeXamlContentToParent()
{
	//ASSERT(m_parentLayout);

	m_layoutUpdatedToken.revoke();
	auto reenableLayoutUpdatedHandler = wil::scope_exit_log(WI_DIAGNOSTICS_INFO, [&]()
	{
		m_layoutUpdatedToken = m_mainUserControl.LayoutUpdated(winrt::auto_revoke, { this, &CFileView::OnXamlLayoutUpdated });
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

void CFileView::FillFileView()
{
	HTREEITEM hRoot = m_wndFileView.InsertItem(_T("FakeApp files"), 0, 0);
	m_wndFileView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hSrc = m_wndFileView.InsertItem(_T("FakeApp Source Files"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeApp.rc"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeAppDoc.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeAppView.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("MainFrm.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("pch.cpp"), 1, 1, hSrc);

	HTREEITEM hInc = m_wndFileView.InsertItem(_T("FakeApp Header Files"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("FakeAppDoc.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("FakeAppView.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("Resource.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("MainFrm.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("pch.h"), 2, 2, hInc);

	HTREEITEM hRes = m_wndFileView.InsertItem(_T("FakeApp Resource Files"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.ico"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeApp.rc2"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeAppDoc.ico"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeToolbar.bmp"), 2, 2, hRes);

	m_wndFileView.Expand(hRoot, TVE_EXPAND);
	m_wndFileView.Expand(hSrc, TVE_EXPAND);
	m_wndFileView.Expand(hInc, TVE_EXPAND);
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFileView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	//::SetWindowPos(hWndXamlIsland, 0, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_SHOWWINDOW);
	//::UpdateWindow(hWndXamlIsland);
	//if (xamlContainer!=nullptr)
	//	xamlContainer.UpdateLayout();
}

void CFileView::OnProperties()
{
	AfxMessageBox(_T("Properties...."));

}

void CFileView::OnFileOpen()
{
	// TODO: Add your command handler code here
}

void CFileView::OnFileOpenWith()
{
	// TODO: Add your command handler code here
}

void CFileView::OnDummyCompile()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditCut()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditCopy()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditClear()
{
	// TODO: Add your command handler code here
}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree(0,0,100,100);
	//m_wndFileView.GetWindowRect(rectTree);
	
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}


