
#pragma once

#include "ViewTree.h"
#include "xamlbridge.h"
using namespace winrt;
using namespace Windows::UI;
using namespace Windows::Foundation;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CFileView : public CDockablePane, DesktopWindowT<CFileView>
{
// Construction
public:
	CFileView() noexcept;
	void OnXamlLayoutUpdated(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);
	void AdjustLayout();
	void OnChangeVisualStyle();
	void OnSizeXamlContentToParent();
// Attributes
protected:
	bool m_parentLayout = false;
	CViewTree m_wndFileView;
	CImageList m_FileViewImages;
	CFileViewToolBar m_wndToolBar;
	HWND hWndXamlIsland;
	wil::unique_hwnd m_hWndXamlIsland = nullptr;
	winrt::MyApp::MainUserControl m_mainUserControl = nullptr;
	winrt::Windows::UI::Xaml::FrameworkElement::LayoutUpdated_revoker m_layoutUpdatedToken{};
	winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource desktopSource;
protected:
	void FillFileView();
	int CreateXamlHost();
	void CreateXamlHostV2();
// Implementation
public:
	virtual ~CFileView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
};

