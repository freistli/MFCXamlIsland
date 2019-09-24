
// MFCXamlIsland01View.h : interface of the CMFCXamlIsland01View class
//

#pragma once


class CMFCXamlIsland01View : public CView
{
protected: // create from serialization only
	CMFCXamlIsland01View() noexcept;
	DECLARE_DYNCREATE(CMFCXamlIsland01View)

// Attributes
public:
	CMFCXamlIsland01Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMFCXamlIsland01View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource desktopSource;
// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG  // debug version in MFCXamlIsland01View.cpp
inline CMFCXamlIsland01Doc* CMFCXamlIsland01View::GetDocument() const
   { return reinterpret_cast<CMFCXamlIsland01Doc*>(m_pDocument); }
#endif

