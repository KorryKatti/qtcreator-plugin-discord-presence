# Discord RPC Plugin for Qt Creator 19.0.2

## Prerequisites

You need:
- **Qt Creator 19.0.2** with source & development files
- **Qt 6.11.1** SDK with headers and libraries
- **CMake** 3.21+ (or qmake if using .pro files)
- **discord-rpc** C++ library
- **C++ Compiler** (MSVC, GCC, or Clang)

### Get Qt Creator Development Files

If you installed Qt Creator via the official installer:
1. Run the installer again
2. Select "Modify"
3. Check "Qt Creator Plugin Development"
4. This installs `/lib/qtcreator/plugins/*.lib` and the required .pri files

Alternatively, build Qt Creator from source:
```bash
git clone https://github.com/qt-creator/qt-creator.git
cd qt-creator
git checkout v19.0.2
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cmake --install build
```

### Download discord-rpc

```bash
git clone https://github.com/discordapp/discord-rpc.git
cd discord-rpc
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install . --prefix /path/to/install
```

## Project Structure

```
DiscordRPCPlugin/
├── CMakeLists.txt
├── src/
│   ├── discordrpcplugin.h
│   ├── discordrpcplugin.cpp
│   ├── discordrpcconstants.h
│   ├── discordrpcmanager.h
│   ├── discordrpcmanager.cpp
│   └── DiscordRPCPlugin.json
├── resources/
│   └── resources.qrc
└── lib/
    └── (discord-rpc headers if bundling)
```

## Key Files

### CMakeLists.txt

The plugin uses Qt Creator's plugin infrastructure. Key points:

- Must link against Qt Creator core libraries (`Core`, `ProjectExplorer`, `TextEditor`)
- Must produce a `.dll` (Windows), `.so` (Linux), or `.dylib` (macOS) in the correct plugins directory
- Must register the plugin via `DiscordRPCPlugin.json`

### DiscordRPCPlugin.json

Metadata file that Qt Creator reads to load the plugin:

```json
{
    "Name" : "DiscordRPC",
    "Version" : "1.0.0",
    "CompatVersion" : "19.0.0",
    "Vendor" : "Your Name",
    "Copyright" : "(C) 2024",
    "License" : "MIT",
    "Description" : "Discord Rich Presence integration for Qt Creator",
    "Url" : "https://github.com/yourname/qtcreator-discord-rpc",
    "Category" : "Integration",
    "Experimental" : false,
    "Enabled" : true,
    "Arguments" : "",
    "Dependencies" : [
        {"Name" : "Core", "Version" : "19.0"},
        {"Name" : "ProjectExplorer", "Version" : "19.0"},
        {"Name" : "TextEditor", "Version" : "19.0"}
    ]
}
```

### Core Plugin Code

#### discordrpcplugin.h

```cpp
#pragma once

#include <extensionsystem/iplugin.h>

namespace DiscordRPC::Internal {

class DiscordRPCPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "DiscordRPCPlugin.json")

public:
    DiscordRPCPlugin();
    ~DiscordRPCPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private:
    class DiscordRPCPluginPrivate *d = nullptr;
};

} // namespace DiscordRPC::Internal
```

#### discordrpcplugin.cpp

```cpp
#include "discordrpcplugin.h"
#include "discordrpcmanager.h"

#include <coreplugin/icore.h>
#include <projectexplorer/session.h>
#include <texteditor/texteditorsettings.h>

namespace DiscordRPC::Internal {

class DiscordRPCPluginPrivate
{
public:
    DiscordRPCManager *manager = nullptr;
};

DiscordRPCPlugin::DiscordRPCPlugin()
    : d(new DiscordRPCPluginPrivate)
{
}

DiscordRPCPlugin::~DiscordRPCPlugin()
{
    delete d;
}

bool DiscordRPCPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    d->manager = new DiscordRPCManager(this);
    return true;
}

void DiscordRPCPlugin::extensionsInitialized()
{
    // All other plugins are loaded; safe to connect signals here
}

ExtensionSystem::IPlugin::ShutdownFlag DiscordRPCPlugin::aboutToShutdown()
{
    if (d->manager)
        d->manager->shutdown();
    return SynchronousShutdown;
}

} // namespace DiscordRPC::Internal
```

#### discordrpcmanager.h

```cpp
#pragma once

#include <QObject>
#include <QTimer>

// Forward declare discord RPC types
struct discord::User;

namespace DiscordRPC::Internal {

class DiscordRPCManager : public QObject
{
    Q_OBJECT

public:
    explicit DiscordRPCManager(QObject *parent = nullptr);
    ~DiscordRPCManager();

    void shutdown();

private slots:
    void onProjectChanged();
    void onEditorChanged();
    void updatePresence();
    void onDiscordReady();
    void onDiscordDisconnected();

private:
    void initializeDiscord();
    void setupConnections();
    QString getFileIcon(const QString &filePath);
    
    QTimer *m_updateTimer = nullptr;
    QString m_currentProject;
    QString m_currentFile;
    bool m_discordConnected = false;
    const char *DISCORD_CLIENT_ID = "YOUR_CLIENT_ID_HERE";
};

} // namespace DiscordRPC::Internal
```

#### discordrpcmanager.cpp

```cpp
#include "discordrpcmanager.h"
#include "discordrpcconstants.h"

#include <coreplugin/icore.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/texteditor.h>

#include <discord_rpc.h>

#include <QDateTime>
#include <QFileInfo>

namespace DiscordRPC::Internal {

DiscordRPCManager::DiscordRPCManager(QObject *parent)
    : QObject(parent)
{
    initializeDiscord();
    setupConnections();
    
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &DiscordRPCManager::updatePresence);
    m_updateTimer->start(15000); // Update every 15 seconds
}

DiscordRPCManager::~DiscordRPCManager()
{
    shutdown();
}

void DiscordRPCManager::initializeDiscord()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    
    handlers.ready = [](const DiscordUser* request) {
        // Called when Discord is ready
    };
    handlers.disconnected = [](int errorCode, const char* message) {
        // Called when Discord disconnects
    };
    
    Discord_Initialize(DISCORD_CLIENT_ID, &handlers, 1, nullptr);
    m_discordConnected = true;
}

void DiscordRPCManager::setupConnections()
{
    auto projectManager = ProjectExplorer::ProjectExplorerPlugin::instance();
    auto sessionManager = ProjectExplorer::SessionManager::instance();
    
    // React to project changes
    connect(sessionManager, &ProjectExplorer::SessionManager::projectAdded,
            this, &DiscordRPCManager::onProjectChanged);
    connect(sessionManager, &ProjectExplorer::SessionManager::projectRemoved,
            this, &DiscordRPCManager::onProjectChanged);
    
    // React to editor changes
    connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
            this, &DiscordRPCManager::onEditorChanged);
}

void DiscordRPCManager::onProjectChanged()
{
    auto session = ProjectExplorer::SessionManager::instance();
    auto projects = session->projects();
    
    if (!projects.isEmpty()) {
        m_currentProject = projects.first()->displayName();
    } else {
        m_currentProject = "";
    }
    
    updatePresence();
}

void DiscordRPCManager::onEditorChanged()
{
    auto editor = Core::EditorManager::currentEditor();
    if (editor && editor->document()) {
        m_currentFile = QFileInfo(editor->document()->filePath().toString()).fileName();
    } else {
        m_currentFile = "";
    }
    
    updatePresence();
}

void DiscordRPCManager::updatePresence()
{
    if (!m_discordConnected)
        return;
    
    DiscordRichPresence presence;
    memset(&presence, 0, sizeof(presence));
    
    presence.state = m_currentFile.isEmpty() ? "Idle" : m_currentFile.toStdString().c_str();
    presence.details = m_currentProject.isEmpty() ? "No project" : m_currentProject.toStdString().c_str();
    presence.largeImageKey = "qt";
    presence.largeImageText = "Qt Creator";
    presence.startTimestamp = static_cast<int64_t>(QDateTime::currentSecsSinceEpoch());
    
    Discord_UpdatePresence(&presence);
}

void DiscordRPCManager::shutdown()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    if (m_discordConnected) {
        Discord_Shutdown();
        m_discordConnected = false;
    }
}

QString DiscordRPCManager::getFileIcon(const QString &filePath)
{
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    
    // Map file extensions to Discord asset names
    static const QMap<QString, QString> iconMap {
        {"cpp", "cpp"}, {"h", "cpp"}, {"c", "c"},
        {"py", "python"}, {"js", "javascript"}, {"ts", "typescript"},
        {"qml", "qml"}, {"json", "json"}, {"xml", "xml"},
        {"txt", "text"}, {"md", "markdown"},
    };
    
    return iconMap.value(suffix, "file");
}

} // namespace DiscordRPC::Internal
```

### discordrpcconstants.h

```cpp
#pragma once

namespace DiscordRPC::Constants {

const char DISCORD_CLIENT_ID[] = "YOUR_DISCORD_CLIENT_ID";

} // namespace DiscordRPC::Constants
```

## Building the Plugin

### On Windows (with MSVC)

```bash
# Ensure you have the Qt Creator development environment set up
set QT_PATH=C:\Qt\6.11.1
set QTCREATOR_PATH=C:\Qt\Tools\QtCreator
set DISCORD_RPC_PATH=C:\path\to\discord-rpc\install

mkdir build
cd build

cmake .. ^
  -G "Visual Studio 17 2022" ^
  -DCMAKE_PREFIX_PATH=%QTCREATOR_PATH% ^
  -DQt6_DIR=%QT_PATH%\msvc2019_64\lib\cmake\Qt6 ^
  -DDISCORD_RPC_DIR=%DISCORD_RPC_PATH%

cmake --build . --config Release
```

### On Linux (with GCC)

```bash
export QT_PATH=/opt/Qt/6.11.1
export QTCREATOR_PATH=/opt/qt-creator
export DISCORD_RPC_PATH=/opt/discord-rpc

mkdir build && cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$QTCREATOR_PATH \
  -DQt6_DIR=$QT_PATH/gcc_64/lib/cmake/Qt6 \
  -DDISCORD_RPC_DIR=$DISCORD_RPC_PATH

cmake --build . -j$(nproc)
```

### Locating the Plugin

After building, copy the plugin to Qt Creator's plugins directory:

- **Windows**: `%APPDATA%\QtProject\QtCreator\plugins\19.0.2\`
- **Linux**: `~/.local/share/QtProject/QtCreator/plugins/19.0.2/`
- **macOS**: `~/Library/Application Support/QtProject/QtCreator/plugins/19.0.2/`

Or directly to your Qt Creator installation:
- `<qt-creator>/lib/QtCreator/plugins/`

## Discord Application Setup

1. Go to [Discord Developer Portal](https://discord.com/developers/applications)
2. Click "New Application"
3. Name it "Qt Creator" (or your preferred name)
4. Get your **Client ID** from the General Information tab
5. Replace `YOUR_CLIENT_ID_HERE` in the code
6. Add custom assets (large image: "qt" icon)

## Debugging

Qt Creator plugins can be debugged:

```bash
# Run Qt Creator with verbose plugin output
qtcreator -logging.rules=qtc.extensionsystem=debug
```

Check the Help → About Plugins menu to verify the plugin loaded:
- Look for "DiscordRPC" in the list
- Check if it's enabled (checkbox is checked)

## Common Issues

### Plugin Won't Load

- Check the `.json` file format (use a JSON validator)
- Verify `CompatVersion` matches your Qt Creator version (19.0.0)
- Ensure all dependencies (Core, ProjectExplorer, TextEditor) are present
- Check plugin directory permissions

### Discord Not Updating

- Verify Client ID is correct
- Ensure Discord desktop app is running
- Check firewall/antivirus isn't blocking discord-rpc
- Add debug output to `updatePresence()` to trace execution

### Build Errors

- Verify Qt Creator development files are installed
- Check CMakeLists.txt paths match your system
- Ensure discord-rpc is built and installed correctly
- Verify C++ compiler version is recent enough for Qt 6

## Next Steps

1. **Add settings dialog** — Let users enable/disable the plugin
2. **Add custom status messages** — Allow users to set custom presence text
3. **Support more file types** — Expand the icon mapping
4. **Handle multiple projects** — Show which project is currently active
5. **Add activity tracking** — Log coding time, etc.

## References

- [Qt Creator Plugin Development](https://doc.qt.io/qtcreator-extending/)
- [Discord RPC Documentation](https://discord.com/developers/docs/activities/rpc)
- [ExtensionSystem Plugin API](https://doc.qt.io/qtcreator-extending/extensionsystem.html)
