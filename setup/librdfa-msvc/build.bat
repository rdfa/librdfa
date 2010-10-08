echo "Setting Path..." > build.log

set DevEnvDir=C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE
set Framework35Version=v3.5
set FrameworkDir=C:\WINDOWS\Microsoft.NET\Framework
set FrameworkVersion=v2.0.50727
set INCLUDE=C:\Program Files\Microsoft Visual Studio 9.0\VC\INCLUDE;C:\Program Files\Microsoft SDKs\Windows\v6.0A\include;C:\Program Files\Microsoft Visual Studio .NET 2003\SDK\v1.1\include\
set Path=C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE;C:\Program Files\Microsoft Visual Studio 9.0\VC\BIN;C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools;C:\WINDOWS\Microsoft.NET\Framework\v3.5;C:\WINDOWS\Microsoft.NET\Framework\v2.0.50727;C:\Program Files\Microsoft Visual Studio 9.0\VC\VCPackages;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin;C:\Program Files\Git\bin
set VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio 9.0\VC
set VS71COMNTOOLS=C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\
set VS90COMNTOOLS=C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\
set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 9.0
set WindowsSdkDir=C:\Program Files\Microsoft SDKs\Windows\v6.0A\

echo "Pulling from librdfa git repository..." >> build.log
git pull >> build.log

echo "Building rdfaMT.lib..." >> build.log
cd rdfa-lib
vcbuild rdfa-lib.vcproj /clean >> ..\build.log
vcbuild rdfa-lib.vcproj >> ..\build.log

echo "Building rdfa.dll..." >> ..\build.log
cd ../librdfa
vcbuild librdfa.vcproj /clean >> ..\build.log
vcbuild librdfa.vcproj >> ..\build.log

cd ..
