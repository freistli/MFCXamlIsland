# MFCXamlIsland

### MFCXamlIsland01 

Is the MFC app with XamlIsland. It loaded Windows::UI::Controls.

It follows Oscar's [guide](https://github.com/marb2000/XamlIslands/tree/master/1903_Samples/CppWinRT_Win32_SingleIsland)  to copy xbf, dll, pri files from MyApp and Winows Runtime Components. And also can be built successfully with WinUI components.

Please remove app.xbf and resource.pri in the output bin directory to let it run at the moment. 

This will cause SampleCPPApp fail because they are in the same output directly and SampleCPPApp uses pp.xbf and resource.pri.

### BaseApp
Dummpy App for MFCXamlIsland01.Package. Without it, MFCXamlIsland01 cannot run with MFCXamlIsland01.Package.

### MFCXamlIsland01.Package 
can launched directly without changing things.
### SampleCPPApp 
Can run with app.xbf and resource.pri without changing things by default. it can load Microsoft::UI::Controls and Windows::UI::Controls
### MyApp
The dummy app for SampleCPPApp
### SampleUserControl
Windows Runtime Component, will be finally used by SampleCPPApp. The component contains Microsoft::UI::Controls and Windows::UI::Controls. MFCXamlIsland01 still cannot use it at the moment.
