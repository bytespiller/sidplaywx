rm -rf ./build/AppDir
mkdir -p build/AppDir

mkdir -p build/AppDir/usr/share/metainfo
cp ./dev/org.bytespiller.sidplaywx.appdata.xml ./build/AppDir/usr/share/metainfo

mkdir -p build/AppDir/usr/bin
#cp ./build/stil.index ./build/AppDir/usr/bin # Don't copy this on Linux, sidplaywx itself will generate new one where needed.
cp -r ./dev/theme ./build/AppDir/usr/bin
cp ./dev/default.m3u8 ./build/AppDir/usr/bin
cp ./dev/bundled-STIL.txt ./build/AppDir/usr/bin
cp ./dev/bundled-Songs.zip ./build/AppDir/usr/bin
cp ./dev/bundled-Songlengths.md5 ./build/AppDir/usr/bin

./build/linuxdeploy-x86_64.AppImage -e ./build/sidplaywx -d ./org.bytespiller.sidplaywx.desktop --icon-file ./dev/icon_src/sidplaywx_icon.png --appdir ./build/AppDir --output appimage
echo -e "\nAll done!"
