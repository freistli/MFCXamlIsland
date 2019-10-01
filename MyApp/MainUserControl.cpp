#include "pch.h"
#include "MainUserControl.h"


using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Composition;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace winrt::Windows::UI::Composition::Scenes;
using namespace winrt::SceneLoaderComponent;
using namespace Windows::Storage;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI::Xaml::Hosting;

namespace winrt::MyApp::implementation
{
    MainUserControl::MainUserControl()
    {  

        InitializeComponent();
		 
		_compositor = Window::Current().Compositor();
		_hostVisual = _compositor.CreateContainerVisual();
		_sceneVisual = SceneVisual::Create(_compositor);
		_sceneVisual.Offset(float3(0.0f, 300.0f, 0));
		 
    }

    winrt::hstring MainUserControl::MyProperty()
    {
        //return userControl().MyProperty();
		return L"usercontrol";
    }

    void MainUserControl::MyProperty(winrt::hstring value)
    {
       // userControl().MyProperty(value);
    }

	IAsyncAction  LoadGltf(Compositor &_compositor, SceneVisual &_sceneVisual, ContainerVisual &_hostVisual,Shapes::Rectangle &mc)
	{
		//winrt::init_apartment();
		Uri uri{ L"ms-appx:///Telescope.gltf" };
		auto buffer{ co_await FileIO::ReadBufferAsync(co_await StorageFile::GetFileFromApplicationUriAsync(uri)) };
		auto loader = SceneLoader{};
		auto sceneNode = loader.Load(buffer, _compositor);
		
		_hostVisual.RelativeSizeAdjustment(Numerics::float2().one());

		ElementCompositionPreview::SetElementChildVisual(mc, _hostVisual);

		_sceneVisual.Scale(float3(2.5f, 2.5f, 1.0f));
		_sceneVisual.Root(SceneNode::Create(_compositor));
		_sceneVisual.Root().Children().Clear();
		_sceneVisual.Root().Children().Append(sceneNode);

		auto rotationAnimation = _compositor.CreateScalarKeyFrameAnimation();
		rotationAnimation.InsertKeyFrame(1.0f, 360.0f, _compositor.CreateLinearEasingFunction());
		rotationAnimation.Duration(std::chrono::seconds{ 16 });
		rotationAnimation.IterationBehavior(AnimationIterationBehavior::Forever);
		_sceneVisual.Root().Transform().RotationAxis(float3(0.0f, 1.0f, 0.2f));

		_sceneVisual.Root().Transform().StartAnimation(L"RotationAngleInDegrees", rotationAnimation);

		_hostVisual.Children().InsertAtTop(_sceneVisual);
		
	}
}



void winrt::MyApp::implementation::MainUserControl::Show3D_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
	mh = ModelHost();
	LoadGltf(_compositor, _sceneVisual, _hostVisual, mh);
	
}
