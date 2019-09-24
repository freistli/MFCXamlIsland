
// MFCXamlIsland01.h : main header file for the MFCXamlIsland01 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "XamlBridge.h"

using namespace winrt;

// CMFCXamlIsland01App:
// See MFCXamlIsland01.cpp for the implementation of this class
//

class CMFCXamlIsland01App : public CWinAppEx
{
public:
	CMFCXamlIsland01App() noexcept;

	winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager windowsXamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager{ nullptr };

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFCXamlIsland01App theApp;
