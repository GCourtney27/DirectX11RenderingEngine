echo Config: Debug
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv" %~dp0..\..\Engine_Source\Third_Party\assimp-5.0.1\build\Assimp.sln /build Debug /project %~dp0..\..\Engine_Source\Third_Party\assimp-5.0.1\build\ALL_BUILD.vcxproj /projectconfig Debug

echo Config: Release
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv" %~dp0..\..\Engine_Source\Third_Party\assimp-5.0.1\build\Assimp.sln /build Release /project %~dp0..\..\Engine_Source\Third_Party\assimp-5.0.1\build\ALL_BUILD.vcxproj /projectconfig Release

pause
