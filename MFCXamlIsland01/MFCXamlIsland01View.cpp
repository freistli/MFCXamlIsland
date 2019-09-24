
// MFCXamlIsland01View.cpp : implementation of the CMFCXamlIsland01View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCXamlIsland01.h"
#endif

#include "MFCXamlIsland01Doc.h"
#include "MFCXamlIsland01View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCXamlIsland01View

IMPLEMENT_DYNCREATE(CMFCXamlIsland01View, CView)

BEGIN_MESSAGE_MAP(CMFCXamlIsland01View, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCXamlIsland01View::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CMFCXamlIsland01View construction/destruction

CMFCXamlIsland01View::CMFCXamlIsland01View() noexcept
{
	// TODO: add construction code here

}

CMFCXamlIsland01View::~CMFCXamlIsland01View()
{
}

BOOL CMFCXamlIsland01View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCXamlIsland01View drawing

void CMFCXamlIsland01View::OnDraw(CDC* /*pDC*/)
{
	CMFCXamlIsland01Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMFCXamlIsland01View printing


void CMFCXamlIsland01View::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCXamlIsland01View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMFCXamlIsland01View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMFCXamlIsland01View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMFCXamlIsland01View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCXamlIsland01View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCXamlIsland01View diagnostics

#ifdef _DEBUG
void CMFCXamlIsland01View::AssertValid() const
{
	CView::AssertValid();
}

void CMFCXamlIsland01View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCXamlIsland01Doc* CMFCXamlIsland01View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCXamlIsland01Doc)));
	return (CMFCXamlIsland01Doc*)m_pDocument;
}
#endif //_DEBUG


// CMFCXamlIsland01View message handlers


int CMFCXamlIsland01View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	//XAML Island section
	HWND hWndXamlIsland;

	// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
	// to host UWP controls in any UI element that is associated with a window handle (HWND).
	desktopSource = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource{ };

	// Get handle to corewindow
	auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();
	// Parent the DesktopWindowXamlSource object to current window
	check_hresult(interop->AttachToWindow(this->GetSafeHwnd()));

	// This Hwnd will be the window handler for the Xaml Island: A child window that contains Xaml.  
	hWndXamlIsland = nullptr;
	// Get the new child window's hwnd 
	interop->get_WindowHandle(&hWndXamlIsland);

	 

	// Update the xaml island window size becuase initially is 0,0
	::SetWindowPos(hWndXamlIsland, NULL, 0, 0, 1000, 1000, SWP_SHOWWINDOW);

	//Creating the Xaml content
	Windows::UI::Xaml::Controls::StackPanel xamlContainer;

	xamlContainer.Background(Windows::UI::Xaml::Media::SolidColorBrush{ Windows::UI::Colors::LightGray() });

	Windows::UI::Xaml::Controls::TextBlock tb;
	tb.Text(L"Hello World from Xaml Islands!");
	tb.VerticalAlignment(Windows::UI::Xaml::VerticalAlignment::Center);
	tb.HorizontalAlignment(Windows::UI::Xaml::HorizontalAlignment::Center);
	tb.FontSize(48);

	Windows::UI::Xaml::Controls::InkCanvas ic;
	ic.Width(300);
	ic.Height(300);
	ic.HorizontalAlignment(Windows::UI::Xaml::HorizontalAlignment::Center);

	ic.InkPresenter().InputDeviceTypes(winrt::Windows::UI::Core::CoreInputDeviceTypes::Touch| winrt::Windows::UI::Core::CoreInputDeviceTypes::Mouse);
	
	Windows::UI::Xaml::Controls::InkToolbar it;
	it.TargetInkCanvas(ic);
	it.HorizontalAlignment(Windows::UI::Xaml::HorizontalAlignment::Center);

	winrt::Windows::UI::Xaml::Controls::ColorPicker cp;
	cp.HorizontalAlignment(Windows::UI::Xaml::HorizontalAlignment::Center);

	xamlContainer.Children().Append(tb);
	xamlContainer.Children().Append(ic);
	xamlContainer.Children().Append(it);
	xamlContainer.Children().Append(cp);
	xamlContainer.UpdateLayout();
	desktopSource.Content(xamlContainer);

	return 0;
}
