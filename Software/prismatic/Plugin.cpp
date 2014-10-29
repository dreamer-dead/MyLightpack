#include "Plugin.hpp"

#include <QSettings>
#include <QIcon>
#include <QDir>
#include <QFile>

#include "Settings.hpp"
#include "common/DebugOut.hpp"
#include "common/defs.h"

using namespace SettingsScope;

namespace {
#if defined(Q_OS_WIN)
static const QString kOsSpecificExecuteKey = "ExecuteOnWindows";
#elif defined(MAC_OS)
static const QString kOsSpecificExecuteKey = "ExecuteOnOSX";
#elif defined(Q_OS_UNIX)
static const QString kOsSpecificExecuteKey = "ExecuteOnNix";
#endif

static const QString kPriorityKey("Priority");
static const QString kEnableKey("Enable");
static const QString kDefaultIconPath(":/icons/plugins.png");
static const QString kDefaultInfoValue("");
}

// static
void Plugin::readPluginInfo(const QSettings& settings, PluginInfo* info) {
    info->name = settings.value("Name", "Error").toString();
    if (settings.contains(kOsSpecificExecuteKey)) {
        info->exec = settings.value( kOsSpecificExecuteKey, kDefaultInfoValue).toString();
    } else {
        info->exec = settings.value("Execute", kDefaultInfoValue).toString();
    }
    info->guid = settings.value("Guid", kDefaultInfoValue).toString();
    info->author = settings.value("Author", kDefaultInfoValue).toString();
    info->description = settings.value("Description", kDefaultInfoValue).toString();
    info->version = settings.value("Version", kDefaultInfoValue).toString();
    info->icon = settings.value("Icon", kDefaultInfoValue).toString();
}

Plugin::Plugin(const QString& name, const QString& path, QObject *parent)
    : QObject(parent)
    , m_pathPlugin(path)
    , m_processState(QProcess::NotRunning) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << name << path;
    const QDir pluginPath(m_pathPlugin);
    const QString fileName = pluginPath.absoluteFilePath(name+".ini");
    Q_ASSERT(QFile(fileName).exists());
    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Main");
    readPluginInfo(settings, &m_info);
    m_info.icon = pluginPath.absoluteFilePath(m_info.icon);
    settings.endGroup();

    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
}

Plugin::~Plugin() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    Stop();
}

QString Plugin::Name() const {
    return m_info.name;
}

QString Plugin::Guid() const {
    return m_info.guid;
}

QString Plugin::Author() const  {
    return m_info.author;
}

QString Plugin::Description() const {
    return m_info.description;
}

QString Plugin::Version() const {
    return m_info.version;
}

QIcon Plugin::Icon() const {
   if (QFile(m_info.icon).exists())
       return QIcon(m_info.icon);
   return QIcon(kDefaultIconPath);
}

int Plugin::getPriority() const {
    return Settings::instance()->pluginValue(m_info.name, kPriorityKey).toInt();
}

void Plugin::setPriority(int priority) {
    Settings::instance()->setPluginValue(m_info.name, kPriorityKey, priority);
}

bool Plugin::isEnabled() const {
    return Settings::instance()->pluginValue(m_info.name, kEnableKey).toBool();
}

void Plugin::setEnabled(bool enable) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << enable;
    Settings::instance()->setPluginValue(m_info.name, kEnableKey, enable);
    if (!enable) this->Stop();
    if (enable) this->Start();
}

void Plugin::Start() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << m_info.exec;

    if (m_process) {
        DEBUG_LOW_LEVEL << "Starting plugin process while it's still running!";
        Stop();
    }

    if (!m_info.exec.isEmpty()) {
        m_process.reset(new QProcess(this));
        connect(m_process.data(), &QProcess::stateChanged,
                this, &Plugin::processStateChanged);
        connect(m_process.data(), SIGNAL(finished(int )),
                this, SLOT(processFinished(int )));
        m_process->setWorkingDirectory(m_pathPlugin);
        m_process->setEnvironment(QProcess::systemEnvironment());
        // process->setProcessChannelMode(QProcess::ForwardedChannels);
        m_process->start(m_info.exec, NULL);
    }
}

void Plugin::Stop() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if (m_process) {
        m_process->disconnect();
        m_process->kill();
        processFinished(1);
    }
}

void Plugin::processFinished(int) {
    Q_ASSERT(m_process);
    m_process.take()->deleteLater();
    m_processState = QProcess::NotRunning;
    emit stateChanged(m_processState);
}

void Plugin::processStateChanged(QProcess::ProcessState newState) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << newState << this << Name();
    m_processState = newState;
    emit stateChanged(m_processState);
}

QProcess::ProcessState Plugin::state() const {
    if (m_process) {
        Q_ASSERT(m_processState == m_process->state());
    } else {
        Q_ASSERT(m_processState == QProcess::NotRunning);
    }
    return m_processState;
}

