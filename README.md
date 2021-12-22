# RGB.NET

This project aims to unify the use of various RGB-devices.   
**It is currently under heavy development and will have breaking changes in the future!** Right now a lot of devices aren't working as expected and there are bugs/unfinished features. Please think about that when you consider using the library in this early stage.    
   
If you want to help with layouting/testing devices or if you need support using the library feel free to join the [RGB.NET discord-channel](https://discord.gg/9kytURv).
asdfasdfasdfasdf

## Adding prerelease packages using NuGet ##
This is the easiest and therefore preferred way to include RGB.NET in your project.  

Since there aren't any release-packages right now you'll have to use the CI-feed from [http://nuget.arge.be](http://nuget.arge.be).   
You can include it either by adding ```http://nuget.arge.be/v3/index.json``` to your Visual Studio package sources or by adding this [NuGet.Config](https://github.com/DarthAffe/RGB.NET/tree/master/Documentation/NuGet.Config) to your project (at the same level as your solution). 

### .NET 4.5 Support ###
At the end of the year with the release of .NET 5 the support for old .NET-Framwork versions will be droppped!   
It's not recommended to use RGB.NET in projects targeting .NET 4.x that aren't planned to be moved to Core/.NET 5 in the future.


### Device-Layouts
To be able to have devices with correct LED-locations and sizes they need to be layouted. Pre-created layouts can be found at https://github.com/DarthAffe/RGB.NET-Resources.   

If you plan to create layouts for your own devices check out https://github.com/DarthAffe/RGB.NET/wiki/Creating-Layouts first. There's also a layout-editor which strongly simplifies most of the work: https://github.com/SpoinkyNL/RGB.NET-Layout-Editor

### Example usage of RGB.NET
[![Example video](https://img.youtube.com/vi/JLRa0Wv4qso/0.jpg)](http://www.youtube.com/watch?v=JLRa0Wv4qso)

#### Example Projects
[https://github.com/DarthAffe/KeyboardAudioVisualizer](https://github.com/DarthAffe/KeyboardAudioVisualizer)   
[https://github.com/DarthAffe/RGBSyncPlus](https://github.com/DarthAffe/RGBSyncPlus)
