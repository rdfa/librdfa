echo "Setting Path..." >> build.log
set Path=C:\Program Files\Git\bin;C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE;C:\Program Files\Microsoft Visual Studio 9.0\VC\BIN;C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools;C:\WINDOWS\Microsoft.NET\Framework\v3.5;C:\WINDOWS\Microsoft.NET\Framework\v2.0.50727;C:\Program Files\Microsoft Visual Studio 9.0\VC\VCPackages > build.log

echo "Pulling from librdfa git repository..." >> build.log
git pull >> build.log

echo "Building rdfaMT.lib..." >> build.log
cd rdfa-lib
msbuild rdfa-lib.vcproj /t:Clean /p:Configuration=Release >> ..\build.log
msbuild rdfa-lib.vcproj /p:Configuration=Release >> ..\build.log

echo "Building rdfa.dll..." >> ..\build.log
cd ../librdfa
msbuild librdfa.vcproj /t:Clean /p:Configuration=Release >> ..\build.log
msbuild librdfa.vcproj /p:Configuration=Release >> ..\build.log

cd ..
