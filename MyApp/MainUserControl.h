//
// Declaration of the MainUserControl class.
//

#pragma once

#include "pch.h"

#include "MainUserControl.g.h"

using namespace winrt;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Shapes;

namespace winrt::MyApp::implementation
{
    struct MainUserControl : MainUserControlT<MainUserControl>
    {
	
		Compositor _compositor = Compositor{ nullptr };
		ContainerVisual _hostVisual = ContainerVisual{nullptr};
		Scenes::SceneVisual _sceneVisual = Scenes::SceneVisual{ nullptr };
		
        MainUserControl();

        hstring MyProperty();
        void MyProperty(hstring value);
	public:
		Rectangle mh{ nullptr };
		void Show3D_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::MyApp::factory_implementation
{
    struct MainUserControl : MainUserControlT<MainUserControl, implementation::MainUserControl>
    {
    };
}
