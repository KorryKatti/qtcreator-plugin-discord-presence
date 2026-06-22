# Building Discord RPC Plugin for Qt Creator 19.0.2 - Windows Guide

## Prerequisites Check

Run this to verify your setup:

```bash
# Check Qt Creator version
qtcreator --version

# Check you have CMake
cmake --version

# Check you have a C++ compiler (choose one)
cl.exe /?              # MSVC
g++ --version          # GCC
clang --version        # Clang
```

Expected output:
```
Qt Creator Version: 19.0.2
Qt Version: 6.11.1
CMake version 3.21+
```

## Step 1: Install Qt Creator Plugin Development Files

### Option A: Via Qt Installer (Easiest)

1. Open Qt Installer/Maintenance Tool
2. Click "Modify"
3. Find your Qt Creator 19.0.2 installation
4. Check "Qt Creator Plugin Development"
5. Click "Install"

This installs:
- `/lib/QtCreator/plugins/*.lib` (import libraries)
- `/src/` folder with plugin headers
- Various `.pri` files

### Option B: Build from Source

```bash
git clone https://github.com/qt-creator/qt-creator.git
cd qt-creator
git checkout v19.0.2
mkdir build && cd build

cmake .. -G "Visual Studio 17 2022" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DQt6_DIR=C:\Qt\6.11.1\msvc2019_64\lib\cmake\Qt6

cmake --build . --config Release
cmake --install . --prefix C:\Qt\Tools\QtCreator
```

## Step 2: Download discord-rpc Library

```bash
git clone https://github.com/discordapp/discord-rpc.git
cd discord-rpc
mkdir build && cd build

cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Install it
cmake --install . --prefix C:\discord-rpc-install
```

After installation, verify these files exist:
- `C:\discord-rpc-install\include\discord-rpc.h`
- `C:\discord-rpc-install\lib\discord-rpc.lib`

## Step 3: Set Up Environment Variables

Create a file `build.bat`:

```batch
@echo off
REM Configuration paths - adjust these to your system
set QT_PATH=C:\Qt\6.11.1\msvc2019_64
set QTCREATOR_PATH=C:\Qt\Tools\QtCreator
set DISCORD_RPC_PATH=C:\discord-rpc-install
set CMAKE_GENERATOR=Visual Studio 17 2022

echo.
echo Qt Path: %QT_PATH%
echo Qt Creator Path: %QTCREATOR_PATH%
echo Discord RPC Path: %DISCORD_RPC_PATH%
echo.

if not exist "%QT_PATH%" (
    echo Error: Qt path not found: %QT_PATH%
    exit /b 1
)

if not exist "%QTCREATOR_PATH%" (
    echo Error: Qt Creator path not found: %QTCREATOR_PATH%
    exit /b 1
)

if not exist "%DISCORD_RPC_PATH%" (
    echo Error: Discord RPC path not found: %DISCORD_RPC_PATH%
    exit /b 1
)

echo All paths verified. Ready to build.
```

Run it to verify all paths:

```bash
build.bat
```

## Step 4: Configure the Plugin Project

Create the plugin directory structure:

```
DiscordRPCPlugin/
├── CMakeLists.txt
├── src/
│   ├── discordrpcplugin.h
│   ├── discordrpcplugin.cpp
│   ├── discordrpcmanager.h
│   ├── discordrpcmanager.cpp
│   ├── discordrpcconstants.h
│   └── DiscordRPCPlugin.json
└── resources/
    └── resources.qrc
```

Copy the provided files into these directories.

## Step 5: Build the Plugin

```bash
cd DiscordRPCPlugin
mkdir build
cd build

cmake .. ^
  -G "Visual Studio 17 2022" ^
  -DCMAKE_PREFIX_PATH=C:\Qt\Tools\QtCreator ^
  -DQt6_DIR=C:\Qt\6.11.1\msvc2019_64\lib\cmake\Qt6 ^
  -DDISCORD_RPC_DIR=C:\discord-rpc-install

cmake --build . --config Release
```

### Common CMake Errors & Fixes

**Error: Could not find Qt6**
```bash
# Add Qt to CMAKE_PREFIX_PATH:
cmake .. -DQt6_DIR=C:\Qt\6.11.1\msvc2019_64\lib\cmake\Qt6
```

**Error: Could not find QtCreator**
```bash
# Set CMAKE_PREFIX_PATH:
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\Tools\QtCreator
```

**Error: discord-rpc not found**
```bash
# Ensure discord-rpc is installed and set:
cmake .. -DDISCORD_RPC_DIR=C:\discord-rpc-install
```

## Step 6: Locate the Built Plugin

After successful build, find the .dll:

```bash
# Check the build directory
dir build\Release\
# Look for DiscordRPCPlugin.dll
```

## Step 7: Install the Plugin

Qt Creator looks for plugins in multiple locations:

### Option A: User Plugins Directory (Recommended)

```batch
REM Create the directory if it doesn't exist
mkdir %APPDATA%\QtProject\QtCreator\plugins\19.0.2

REM Copy the plugin
copy build\Release\DiscordRPCPlugin.dll ^
  %APPDATA%\QtProject\QtCreator\plugins\19.0.2\
```

### Option B: Installation Directory

```batch
copy build\Release\DiscordRPCPlugin.dll ^
  "C:\Qt\Tools\QtCreator\lib\QtCreator\plugins\"
```

## Step 8: Verify Plugin Installation

1. Start Qt Creator
2. Go to **Help** → **About Plugins**
3. Search for "DiscordRPC"
4. Check that it's listed and **Enabled** (checkbox is checked)
5. If not listed, check **View** → check "Show disabled plugins"

### Troubleshooting Plugin Not Loading

If the plugin doesn't appear:

```bash
# Run Qt Creator with verbose output
qtcreator -logging.rules=qtc.extensionsystem=debug

# Check the output for error messages like:
# "Cannot load plugin" 
# "Missing dependency"
# "Incompatible version"
```

## Step 9: Set Up Discord Application

1. Go to https://discord.com/developers/applications
2. Click **New Application**
3. Name it "Qt Creator"
4. Go to the **General Information** tab
5. Copy your **Client ID**
6. In `discordrpcmanager.h`, replace:
   ```cpp
   static constexpr const char *DISCORD_CLIENT_ID = "1234567890123456789";
   ```
   with your actual Client ID

7. Rebuild the plugin:
   ```bash
   cd build
   cmake --build . --config Release
   ```

8. Copy the new DLL to the plugins directory
9. Restart Qt Creator

## Step 10: Test the Plugin

1. Ensure Discord is running
2. Open a Qt project in Qt Creator
3. Check your Discord profile
4. You should see a status showing:
   - Project name
   - Current file
   - Qt Creator icon

## Troubleshooting

### Plugin loads but Discord doesn't update

**Check 1:** Verify Client ID
```cpp
// In discordrpcmanager.h, ensure you set the correct Client ID
static constexpr const char *DISCORD_CLIENT_ID = "YOUR_CLIENT_ID";
```

**Check 2:** Discord desktop app must be running

**Check 3:** Check firewall/antivirus not blocking discord-rpc

**Check 4:** Enable debug output:
```bash
# Add this to discordrpcmanager.cpp to see debug messages
qDebug() << "Discord RPC: Updating presence...";
```

Then run Qt Creator and check the Application Output for debug messages.

### DLL dependency errors at runtime

**Error:** "Cannot find discord-rpc.dll" or "Missing dependency"

**Solution:** The discord-rpc DLL must be in PATH:

```batch
REM Option 1: Copy discord-rpc.dll to plugin directory
copy C:\discord-rpc-install\bin\discord-rpc.dll ^
  %APPDATA%\QtProject\QtCreator\plugins\19.0.2\

REM Option 2: Add to PATH
set PATH=%PATH%;C:\discord-rpc-install\bin
```

### Build errors with Qt 6.11.1

Ensure you're using the exact CMake commands above. Qt 6 is more strict about configuration.

## Rebuilding After Changes

When you modify the plugin code:

```bash
cd build
cmake --build . --config Release
copy Release\DiscordRPCPlugin.dll %APPDATA%\QtProject\QtCreator\plugins\19.0.2\
REM Restart Qt Creator
```

## Next Steps

Once working:
1. Add a settings dialog to enable/disable
2. Support custom status messages
3. Track more file types
4. Add activity logging

Good luck! 🎉
