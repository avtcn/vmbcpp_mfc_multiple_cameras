# mfc_multiple_cameras - Vimba CPP Multiple Cameras Example
Vimba C++ AsynchronousGrab MFC example with multiple cameras supported.

* Support Vimba 6.0 Windows 11.
* Microsoft Visual Studio Community 2022 (64 位) - Current 版本 17.9.6

## 安装目录
请装此例子代码主目录放置在`Vimba CPP Example / AsynchronousGrab`中， 此例子基于原始全程`AsynchronousGrab\MFC`：
```bash
C:\Users\Public\Documents\Allied Vision\Vimba_6.0\VimbaCPP_Examples_Play\AsynchronousGrab\vmbcpp_mfc_multiple_cameras\Build\VS2010
```

## Instruction
```bash
Remember copy `\Vimba2.1\VimbaCPP\Bin\Win[32|64]\*.dll` files to `\Build\VS2010\Win32\Debug\` folder:  
* VimbaC.dll  
* VimbaCPP.dll  
* VimbaCPPd.dll
* VimbaImageTransform.dll  
```
已经可以自动处理这些Dll文件的复制工作，无需手动处理：
```xml
<PostBuildEvent>
  <Command>
    xcopy  "$(VimbaHome)\VimbaCPP\Bin\Win$(PlatformArchitecture)\*.dll"  "$(ProjectDir)Win$(PlatformArchitecture)\$(Configuration)\" /Y 
    xcopy  "$(VimbaHome)\VimbaImageTransform\Bin\Win$(PlatformArchitecture)\*.dll" "$(ProjectDir)Win$(PlatformArchitecture)\$(Configuration)\" /Y 
  </Command>
  <Message>Copy necessary VimbaC/VimbaCPP/VimbaImageTransform dlls to executing folder.</Message>
</PostBuildEvent>
```

在`Build/VS2010/AsynchronousGrabMFC.vcxproj`文件中，定义
```xml
<PropertyGroup Label="Globals">
    <VimbaHome>C:\Program Files\Allied Vision\Vimba_6.0</VimbaHome>
    <ProjectGuid>{811FB091-5B0B-4A00-BB71-643E5CA9AE1A}</ProjectGuid>
    <RootNamespace>AsynchronousGrab</RootNamespace>
    <Keyword>MFCProj</Keyword>
    <ProjectName>AsynchronousGrabMFC</ProjectName>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
```
  
支持 Debug/Release, Win32/x64 等多种编译模式。


## 测试
* Vimba 6.0 on Windows 11.
* Alvium G1-158
* Manta G-895b  @2025年5月7日



## Screenshot
![](screenshot.png)


## Comments
如果多个相机通过交换机接入电脑的千兆网口，一定要注意此时多台相机的数据是共享一条千兆网的，为避免不稳定，需要合理分配每个相机的带宽；  
对于使用4网口PCIe独立网卡，每个相机连接其上的单独网口，则不存在此问题。
