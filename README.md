# MFCXamlIsland

### MFCXamlIsland01 

Is the MFC MDI app with XamlIsland. It loaded Windows::UI::Controls and WinUI 2.2 (Windows::UI::Controls) through Windows Runtime Component.

![4](https://github.com/freistli/images/blob/master/MFCXamlIsland/4.png)

It follows Oscar's [guide](https://github.com/marb2000/XamlIslands/tree/master/1903_Samples/CppWinRT_Win32_SingleIsland)  to copy xbf, dll, pri files from MyApp and Winows Runtime Components. And also can be built successfully with WinUI components.

### SampleCPPApp 
Standard C++ Windows App. Can run with app.xbf and resource.pri without changing things by default. it can load Microsoft::UI::Controls and Windows::UI::Controls

![3](https://github.com/freistli/images/blob/master/MFCXamlIsland/3.png)

### MyApp
The dummy app for MFCXamlIsland01 app and SampleCPPApp

### SampleUserControl
Windows Runtime Component, will be finally used by MFCXamlIsland01 app and SampleCPPApp. The component contains Microsoft::UI::Controls and Windows::UI::Controls. MFCXamlIsland01 still cannot use it at the moment.


