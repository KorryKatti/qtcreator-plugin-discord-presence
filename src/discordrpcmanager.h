#pragma once

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QMenu>
#include <QElapsedTimer>

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>

#include <projectexplorer/project.h>
#include <projectexplorer/projecttree.h>

#include <discord_rpc.h>

#include <ctime>
#include <functional>
#include <string>

namespace DiscordRPC::Internal {

struct RichPresenceFileDescriptor {
    QString ImageKey;
    QString Description;
    QString WorkingVerb;
};

class QDiscordRichPresence {
public:
    QString State;
    QString Details;
    quint64 StartTimestamp;
    quint64 EndTimestamp;
    QString LargeImageKey;
    QString LargeImageText;
    QString SmallImageKey;
    QString SmallImageText;
    QString PartyId;
    qint32 PartySize;
    qint32 PartyMax;
    QString MatchSecret;
    QString SpectateSecret;
    QString JoinSecret;
    int8_t Instance;

    void update() const;
    QDiscordRichPresence();
};

class DiscordRPCManager : public QObject
{
    Q_OBJECT

public:
    explicit DiscordRPCManager(QObject *parent = nullptr);
    ~DiscordRPCManager() override;

    void activate();
    void deactivate();

private:
    static const QMap<QString, RichPresenceFileDescriptor> s_mimeToDescriptor;

    struct MimeOverride {
        QList<QString> Extensions;
        QString TargetMime;
    };
    static QMap<QString, QList<MimeOverride>> s_mimeOverrideMap;
    static QString overrideMime(const QString &mime, const Utils::FilePath &filePath);

    std::time_t m_activatedTimestamp;
    std::time_t m_timeOnCurrentEditor;
    QTimer m_syncTimer;
    QList<QMetaObject::Connection> m_syncConnections;

    void initializeDiscord();
    void setupControlMenu();
    void syncToEditor();
    void setIdleState();
    void attemptReconnect();

    bool m_connected = false;
    int m_reconnectAttempts = 0;
    static constexpr int kMaxReconnectAttempts = 5;
    static constexpr int kBaseReconnectDelayMs = 1000;
    QTimer m_reconnectTimer;
    static DiscordRPCManager *s_instance;
};

} // namespace DiscordRPC::Internal
