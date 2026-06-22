#include "discordrpcmanager.h"
#include "discordrpcconstants.h"

#include <QDateTime>
#include <QFileInfo>
#include <QDebug>

namespace DiscordRPC::Internal {

DiscordRPCManager *DiscordRPCManager::s_instance = nullptr;

// --- QDiscordRichPresence ---

QDiscordRichPresence::QDiscordRichPresence()
    : StartTimestamp{0}, EndTimestamp{0}, PartySize{0}, PartyMax{0}, Instance{0} {}

void QDiscordRichPresence::update() const
{
    DiscordRichPresence presence{};
    memset(&presence, 0, sizeof(presence));

    const QByteArray state = State.toLocal8Bit();
    const QByteArray details = Details.toLocal8Bit();
    const QByteArray largeImageKey = LargeImageKey.toLocal8Bit();
    const QByteArray largeImageText = LargeImageText.toLocal8Bit();
    const QByteArray smallImageKey = SmallImageKey.toLocal8Bit();
    const QByteArray smallImageText = SmallImageText.toLocal8Bit();

    presence.state = state.isEmpty() ? nullptr : state.data();
    presence.details = details.isEmpty() ? nullptr : details.data();
    presence.largeImageKey = largeImageKey.isEmpty() ? nullptr : largeImageKey.data();
    presence.largeImageText = largeImageText.isEmpty() ? nullptr : largeImageText.data();
    presence.smallImageKey = smallImageKey.isEmpty() ? nullptr : smallImageKey.data();
    presence.smallImageText = smallImageText.isEmpty() ? nullptr : smallImageText.data();
    presence.startTimestamp = StartTimestamp;
    presence.endTimestamp = EndTimestamp;
    presence.partySize = PartySize;
    presence.partyMax = PartyMax;
    presence.instance = Instance;

    Discord_UpdatePresence(&presence);
}

// --- MIME-to-descriptor map ---

const QMap<QString, RichPresenceFileDescriptor> DiscordRPCManager::s_mimeToDescriptor = {
    {"text/x-c++src",              {"cxx",  "C++ Source File",       "Editing"}},
    {"text/x-c++hdr",              {"cxx",  "C++ Header File",       "Editing"}},
    {"text/x-csrc",                {"c",    "C Source File",         "Editing"}},
    {"text/x-chdr",                {"c",    "C Header File",         "Editing"}},
    {"text/x-csharp",              {"csharp", "C# Source File",      "Editing"}},
    {"text/x-python",              {"python", "Python Script",       "Editing"}},
    {"application/x-ruby",         {"ruby",  "Ruby Script",          "Editing"}},
    {"text/rust",                  {"rust",  "Rust Source File",     "Editing"}},
    {"text/x-lua",                 {"lua",   "Lua Script",           "Editing"}},
    {"text/css",                   {"css",   "CSS Stylesheet",       "Editing"}},
    {"text/qss",                   {"qss",   "QSS Stylesheet",       "Editing"}},
    {"application/javascript",     {"js",    "JavaScript File",      "Editing"}},
    {"text/x-java",                {"java",  "Java Class",           "Editing"}},
    {"text/x-qml",                 {"qml",   "QtQuick QML File",     "Editing"}},
    {"text/x-qt.ui+qml",           {"qml",   "QtQuick QML UI File",  "Designing"}},
    {"application/octet-stream",   {"binary", "Binary Data",         "Inspecting"}},
    {"text/x-asm",                 {"asm",   "Assembly Source File", "Editing"}},
    {"text/x-asminfo",             {"asminfo", "Preprocessed C/C++ File", "Editing"}},
    {"application/vnd.qt.qmakeprofile",       {"qmake",    "QMake Project File",       "Configuring"}},
    {"application/vnd.qt.qmakeproincludefile", {"qmake_pri", "QMake Include File",     "Editing"}},
    {"application/vnd.qt.qmakeprostashfile",   {"qt",       "QMake Stash File",         "Editing"}},
    {"text/x-cmake-project",       {"cmake", "CMake Project File",   "Editing"}},
    {"text/x-makefile",            {"gnu",   "GNU Makefile",         "Editing"}},
    {"text/gitignore",             {"git",   "Gitignore File",       "Configuring"}},
    {"application/x-designer",     {"ui",    "Qt User Interface File", "Designing"}},
    {"application/vnd.qt.xml.resource", {"qrc", "Qt Resource File",  "Editing"}},
    {"application/json",           {"json",  "JSON File",            "Editing"}},
    {"text/plain",                 {"txt",   "Plain Text File",      "Editing"}},
    {"text/markdown",              {"markdown", "Markdown Document", "Writing"}},
    {"text/html",                  {"html",  "HTML File",            "Editing"}},
    {"application/xml",            {"xml",   "XML File",             "Editing"}},
    {"text/vnd.qtcreator.git.submit", {"git", "Git Commit File",    "Editing"}},
    {"image/png",                  {"image", "PNG Image",            "Viewing"}},
};

// --- MIME overrides ---

QMap<QString, QList<DiscordRPCManager::MimeOverride>> DiscordRPCManager::s_mimeOverrideMap = {
    {"text/x-c++hdr", {{{".h"}, "text/x-chdr"}}},
    {"text/plain",    {{{".qss"}, "text/qss"},
                       {{".gitignore"}, "text/gitignore"},
                       {{".i"}, "text/x-asminfo"}}},
};

QString DiscordRPCManager::overrideMime(const QString &mime, const Utils::FilePath &filePath)
{
    if (!s_mimeOverrideMap.contains(mime))
        return mime;

    for (const auto &o : s_mimeOverrideMap.value(mime)) {
        for (const QString &ext : o.Extensions) {
            if (filePath.endsWith(ext))
                return o.TargetMime;
        }
    }
    return mime;
}

// --- DiscordRPCManager ---

DiscordRPCManager::DiscordRPCManager(QObject *parent)
    : QObject(parent)
    , m_activatedTimestamp(0)
    , m_timeOnCurrentEditor(0)
{
    s_instance = this;

    connect(&m_reconnectTimer, &QTimer::timeout, this, &DiscordRPCManager::attemptReconnect);
    connect(&m_idleTimer, &QTimer::timeout, this, [this]() {
        if (!m_idle) {
            m_idle = true;
            setIdleState();
            qDebug() << "Discord RPC: Idle";
        }
    });

    initializeDiscord();
    setupControlMenu();
    loadSettings();
    activate();
}

DiscordRPCManager::~DiscordRPCManager()
{
    deactivate();
    Discord_Shutdown();
    s_instance = nullptr;
}

void DiscordRPCManager::loadSettings()
{
    QSettings settings;

    m_idleMessage = settings.value("DiscordRPC/IdleMessage",
                                   QStringLiteral("Not Currently Editing Anything")).toString();

    m_verbOverrides.clear();
    settings.beginGroup("DiscordRPC/VerbOverrides");
    for (const auto &key : settings.childKeys())
        m_verbOverrides[key] = settings.value(key).toString();
    settings.endGroup();

    qDebug() << "Discord RPC: Settings loaded"
             << "(idle:" << m_idleMessage
             << ", verb overrides:" << m_verbOverrides.size() << ")";
}

void DiscordRPCManager::initializeDiscord()
{
    DiscordEventHandlers handlers{};
    memset(&handlers, 0, sizeof(handlers));

    handlers.disconnected = [](int errorCode, const char *message) {
        Q_UNUSED(errorCode)
        qDebug() << "Discord RPC: Disconnected -" << message;
        if (s_instance) {
            s_instance->m_connected = false;
            s_instance->m_reconnectAttempts = 0;
            s_instance->m_reconnectTimer.start(kBaseReconnectDelayMs);
        }
    };

    Discord_Initialize(Constants::DISCORD_CLIENT_ID, &handlers, 1, nullptr);
    m_connected = true;
    qDebug() << "Discord RPC: Initialized";
}

void DiscordRPCManager::setupControlMenu()
{
    auto *actionManager = Core::ActionManager::actionContainer(Core::Constants::M_TOOLS);

    auto *menu = Core::ActionManager::createMenu(Utils::Id(Constants::MENU_ID));
    menu->menu()->setTitle("Discord RPC");

    auto *startAction = new QAction("Start Discord RPC", this);
    auto *stopAction = new QAction("Stop Discord RPC", this);

    auto *startCmd = Core::ActionManager::registerAction(
        startAction, Utils::Id(Constants::ACTION_START_ID),
        Core::Context{Core::Constants::C_GLOBAL});

    auto *stopCmd = Core::ActionManager::registerAction(
        stopAction, Utils::Id(Constants::ACTION_STOP_ID),
        Core::Context{Core::Constants::C_GLOBAL});

    connect(startAction, &QAction::triggered, this, &DiscordRPCManager::activate);
    connect(stopAction, &QAction::triggered, this, &DiscordRPCManager::deactivate);

    menu->addAction(startCmd);
    menu->addAction(stopCmd);
    actionManager->addMenu(menu);
}

void DiscordRPCManager::activate()
{
    deactivate();
    m_idle = false;
    setIdleState();

    m_syncConnections = {
        connect(Core::EditorManager::instance(),
                &Core::EditorManager::currentEditorChanged,
                this, [this](Core::IEditor *) {
                    m_timeOnCurrentEditor = 0;
                    m_idle = false;
                    m_idleTimer.start(kIdleTimeoutMs);
                    syncToEditor();
                }),

        connect(ProjectExplorer::ProjectTree::instance(),
                &ProjectExplorer::ProjectTree::currentProjectChanged,
                this, [this](ProjectExplorer::Project *) {
                    m_idle = false;
                    m_idleTimer.start(kIdleTimeoutMs);
                    syncToEditor();
                }),

        connect(Core::EditorManager::instance(),
                &Core::EditorManager::currentDocumentStateChanged,
                this, [this]() {
                    m_idle = false;
                    m_idleTimer.start(kIdleTimeoutMs);
                    syncToEditor();
                }),
    };

    m_syncTimer.start(1000);
    m_idleTimer.start(kIdleTimeoutMs);
    m_activatedTimestamp = std::time(nullptr);

    qDebug() << "Discord RPC: Activated";
}

void DiscordRPCManager::deactivate()
{
    m_reconnectTimer.stop();
    m_reconnectAttempts = 0;
    m_idleTimer.stop();
    m_idle = false;

    for (auto &conn : m_syncConnections)
        disconnect(conn);
    m_syncConnections.clear();

    m_syncTimer.stop();
    Discord_ClearPresence();

    qDebug() << "Discord RPC: Deactivated";
}

void DiscordRPCManager::setIdleState()
{
    QDiscordRichPresence presence;
    presence.LargeImageKey = "qt";
    presence.LargeImageText = "Qt Creator";
    presence.Details = m_idleMessage;
    presence.StartTimestamp = m_activatedTimestamp;
    presence.update();
}

void DiscordRPCManager::attemptReconnect()
{
    if (m_connected || m_reconnectAttempts >= kMaxReconnectAttempts) {
        m_reconnectTimer.stop();
        if (m_reconnectAttempts >= kMaxReconnectAttempts)
            qDebug() << "Discord RPC: Gave up reconnecting after" << kMaxReconnectAttempts << "attempts";
        return;
    }

    ++m_reconnectAttempts;
    const int delay = kBaseReconnectDelayMs * (1 << (m_reconnectAttempts - 1));
    qDebug() << "Discord RPC: Reconnect attempt" << m_reconnectAttempts
             << "in" << delay << "ms";

    m_reconnectTimer.stop();

    Discord_Shutdown();
    DiscordEventHandlers handlers{};
    memset(&handlers, 0, sizeof(handlers));
    handlers.disconnected = [](int errorCode, const char *message) {
        Q_UNUSED(errorCode)
        qDebug() << "Discord RPC: Disconnected -" << message;
        if (s_instance) {
            s_instance->m_connected = false;
            s_instance->m_reconnectAttempts = 0;
            s_instance->m_reconnectTimer.start(kBaseReconnectDelayMs);
        }
    };

    Discord_Initialize(Constants::DISCORD_CLIENT_ID, &handlers, 1, nullptr);
    m_connected = true;

    if (m_syncTimer.isActive())
        syncToEditor();

    m_reconnectTimer.start(delay);
}

void DiscordRPCManager::syncToEditor()
{
    auto *editor = Core::EditorManager::instance()->currentEditor();
    if (!editor || !editor->document()) {
        setIdleState();
        return;
    }

    auto *activeProject = ProjectExplorer::ProjectTree::currentProject();
    const QString projectName = activeProject ? activeProject->displayName() : "No Project";

    const Utils::FilePath filePath = editor->document()->filePath();
    const QString fileName = filePath.fileName();
    const QString rawMime = editor->document()->mimeType();
    const QString mime = overrideMime(rawMime, filePath);

    RichPresenceFileDescriptor desc;
    if (s_mimeToDescriptor.contains(mime))
        desc = s_mimeToDescriptor.value(mime);
    else
        desc = {"unknown", "Unknown File (" + mime + ")", "Editing"};

    if (m_verbOverrides.contains(mime))
        desc.WorkingVerb = m_verbOverrides.value(mime);

    QDiscordRichPresence presence;
    presence.Details = desc.WorkingVerb + " " + desc.Description;
    presence.State = fileName + "/" + projectName;
    presence.LargeImageKey = desc.ImageKey;
    presence.LargeImageText = desc.WorkingVerb + " " + fileName
                              + " since " + QString::number(m_timeOnCurrentEditor) + "s"
                              + " (" + mime + ")";
    presence.SmallImageKey = "qtcircle";
    presence.SmallImageText = projectName;
    presence.StartTimestamp = m_activatedTimestamp;
    presence.update();

    ++m_timeOnCurrentEditor;
}

} // namespace DiscordRPC::Internal
