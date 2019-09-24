#pragma once
#include "FileView.h"
 

class XamlViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};
// XamlView

class XamlView : public CDockablePane, DesktopWindowT<CFileView>
{
	//DECLARE_DYNAMIC(XamlView)

public:
	XamlView() noexcept;
	virtual ~XamlView(); 
	void OnXamlLayoutUpdated(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);
	void AdjustLayout();
	void OnChangeVisualStyle();
	void OnSizeXamlContentToParent();
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	int CreateXamlHost();
	void CreateXamlHostV2();
protected:
	HWND hWndXamlIsland;
	bool m_parentLayout = false;
	wil::unique_hwnd m_hWndXamlIsland = nullptr;
	winrt::MyApp::MainUserControl m_mainUserControl = nullptr;
	winrt::Windows::UI::Xaml::FrameworkElement::LayoutUpdated_revoker m_layoutUpdatedToken{};
	winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource desktopSource;
	XamlViewToolBar m_wndToolBar;
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()
};


