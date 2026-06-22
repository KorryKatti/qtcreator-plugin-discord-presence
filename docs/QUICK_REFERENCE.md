# Discord RPC Plugin - Quick Reference

## Pre-Build Checklist

- [ ] Qt Creator 19.0.2 installed with development files
- [ ] Qt 6.11.1 SDK installed
- [ ] CMake 3.21+ installed
- [ ] C++ compiler ready (MSVC/GCC/Clang)
- [ ] Git installed
- [ ] discord-rpc library downloaded and built

## Key Paths to Verify

```
Windows:
  QTCREATOR_PATH:     C:\Qt\Tools\QtCreator
  QT_PATH:            C:\Qt\6.11.1\msvc2019_64
  DISCORD_RPC_PATH:   C:\discord-rpc-install
  PLUGIN_OUTPUT:      %APPDATA%\QtProject\QtCreator\plugins\19.0.2\

Linux/macOS:
  QTCREATOR_ROOT:     $(which qtcreator | xargs dirname | xargs dirname)
  QT_PATH:            /opt/Qt/6.11.1 (or use pkg-config Qt6Core)
  DISCORD_RPC_PATH:   $HOME/.local/opt/discord-rpc
  PLUGIN_OUTPUT:      ~/.local/share/QtProject/QtCreator/plugins/19.0.2/
```

## Build Command Reference

### Windows (MSVC)
```batch
cd build
cmake .. -G "Visual Studio 17 2022" ^
  -DCMAKE_PREFIX_PATH=C:\Qt\Tools\QtCreator ^
  -DQt6_DIR=C:\Qt\6.11.1\msvc2019_64\lib\cmake\Qt6 ^
  -DDISCORD_RPC_DIR=C:\discord-rpc-install
cmake --build . --config Release
```

### Linux/macOS (GCC/Clang)
```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$QTCREATOR_ROOT \
  -DQt6_DIR=$QT_PATH/lib/cmake/Qt6 \
  -DDISCORD_RPC_DIR=$DISCORD_RPC_PATH
cmake --build . -j$(nproc)
```

## File List

**Source Files to Create:**
- [ ] src/discordrpcplugin.h
- [ ] src/discordrpcplugin.cpp
- [ ] src/discordrpcmanager.h
- [ ] src/discordrpcmanager.cpp
- [ ] src/discordrpcconstants.h
- [ ] src/DiscordRPCPlugin.json
- [ ] CMakeLists.txt

**Configuration Files:**
- [ ] CMakeLists.txt (in root)

**Documentation:**
- [ ] DISCORD_RPC_PLUGIN_GUIDE.md
- [ ] BUILD_GUIDE_WINDOWS.md (or Linux)
- [ ] README.md

## Installation Locations

### Windows
```
%APPDATA%\QtProject\QtCreator\plugins\19.0.2\DiscordRPCPlugin.dll
```

### Linux
```
~/.local/share/QtProject/QtCreator/plugins/19.0.2/DiscordRPCPlugin.so
```

### macOS
```
~/Library/Application Support/QtProject/QtCreator/plugins/19.0.2/DiscordRPCPlugin.dylib
```

## Discord Setup

1. Go to: https://discord.com/developers/applications
2. Click: **New Application**
3. Name: "Qt Creator"
4. Copy: **Client ID**
5. In code: Replace `DISCORD_CLIENT_ID` value
6. Rebuild and restart Qt Creator

## Verification Steps

**After Build:**
```bash
# Check plugin file exists and has reasonable size
ls -lh build/DiscordRPCPlugin.so    # Linux
ls -lh build\Release\DiscordRPCPlugin.dll  # Windows
```

**After Installation:**
1. Open Qt Creator
2. Go to: **Help** → **About Plugins**
3. Search: "DiscordRPC"
4. Verify: Checkbox is checked (enabled)

**After Startup:**
1. Start Discord desktop app
2. Open a Qt project in Qt Creator
3. Check your Discord profile
4. Should see: Project name + current file

## Common Issues & Fixes

| Issue | Solution |
|-------|----------|
| Plugin won't load | Check plugin path, verify .json metadata |
| Discord not updating | Ensure Client ID is correct, Discord is running |
| CMake can't find Qt6 | Add `-DQt6_DIR=path/to/Qt6/lib/cmake/Qt6` |
| Missing discord-rpc | Build discord-rpc first, set `DISCORD_RPC_DIR` |
| Library not found at runtime | Add discord-rpc lib path to `LD_LIBRARY_PATH` (Linux) |

## Quick Troubleshooting

### Linux: Plugin loads but discord-rpc symbols unresolved
```bash
export LD_LIBRARY_PATH=$HOME/.local/opt/discord-rpc/lib:$LD_LIBRARY_PATH
qtcreator
```

### Windows: "Cannot find discord-rpc.dll"
```batch
copy C:\discord-rpc-install\bin\discord-rpc.dll ^
  %APPDATA%\QtProject\QtCreator\plugins\19.0.2\
```

### Plugin not appearing in list
```bash
# Run with debug output
qtcreator -logging.rules=qtc.extensionsystem=debug
```

## Development Commands

### Clean Build
```bash
rm -rf build
mkdir build && cd build
# ... run cmake and build again
```

### Fast Rebuild (after code changes)
```bash
cd build
cmake --build . --config Release
# Copy plugin to install location
# Restart Qt Creator
```

### Enable Debug Logging
Edit `src/discordrpcmanager.cpp` and add:
```cpp
qDebug() << "Discord RPC: [message]";
```

## Useful Qt Creator Debug Info

```bash
# Show all loaded plugins
qtcreator -help | grep -i plugin

# Run with extension system debug output
qtcreator -logging.rules=qtc.extensionsystem=debug

# Show all loaded libraries (Linux)
ldd ~/.local/share/QtProject/QtCreator/plugins/19.0.2/DiscordRPCPlugin.so
```

## Next Steps After Building

1. ✅ Build and install plugin
2. ⬜ Configure Discord application (get Client ID)
3. ⬜ Update Client ID in source
4. ⬜ Rebuild and test
5. ⬜ Customize presence messages (optional)
6. ⬜ Add settings dialog (optional)
7. ⬜ Publish/share with others

## Links

- Qt Creator Documentation: https://doc.qt.io/qtcreator-extending/
- Discord RPC API: https://discord.com/developers/docs/activities/rpc
- discord-rpc GitHub: https://github.com/discordapp/discord-rpc
- Qt Documentation: https://doc.qt.io/qt-6/

---

**Start with:** Read the appropriate BUILD_GUIDE for your platform, then follow the steps above.
