#include "PluginsManager.hpp"

#include <QDir>
#include <QProcess>

#include "Plugin.hpp"
#include "common/DebugOut.hpp"

// static
QString PluginsManager::defaultPluginsDir() {
    return QString("Plugins");
}

PluginsManager::PluginsManager(const QString& pluginsDir, QObject *parent)
    : QObject(parent)
    , m_pluginsDir(pluginsDir) {
}


PluginsManager::~PluginsManager()
{
    dropPlugins();
}

void PluginsManager::dropPlugins(){
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    //cleanAll();
    for(QMap<QString, Plugin*>::iterator it = _plugins.begin(); it != _plugins.end(); ++it){
        Plugin* p = it.value();
        p->Stop();
        delete p;
    }
    _plugins.clear();
}

void PluginsManager::reloadPlugins(){
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    dropPlugins();
    const QDir pluginsDir(m_pluginsDir);
    if (pluginsDir.exists()) {
        loadPlugins(m_pluginsDir);
        startPlugins();
    }
}

void PluginsManager::loadPlugins(const QDir& pluginsDir)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << pluginsDir.absolutePath();

    QStringList lstDirs = pluginsDir.entryList(
        QDir::Dirs |
        QDir::AllDirs |
        QDir::NoDotAndDotDot); //Get directories

    foreach(QString pluginDir, lstDirs){
       const QString plugin = QFileInfo (pluginDir).baseName();
       if (_plugins.contains(plugin)) {
           DEBUG_LOW_LEVEL << "Already loaded a plugin named " << plugin << " !";
           continue;
       }

       Plugin* p = new Plugin(plugin, pluginsDir.absoluteFilePath(pluginDir), this);
       //DEBUG_LOW_LEVEL <<p->getName()<<  p->getAuthor() << p->getDescription() << p->getVersion();
       //connect(p, SIGNAL(executed()), this, SIGNAL(pluginExecuted()));
       _plugins[plugin] = p;
   }

    emit updatePlugin(_plugins.values());
}

Plugin* PluginsManager::getPlugin(const QString& name_) const {
    const QMap<QString, Plugin*>::const_iterator found = _plugins.find(name_);
    if(found != _plugins.end())
        return found.value();
    else
        return 0;
}

QList<Plugin*> PluginsManager::getPluginList() const {
    return _plugins.values();
}

void PluginsManager::startPlugins()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    for(QMap<QString, Plugin*>::iterator it = _plugins.begin(); it != _plugins.end(); ++it){
        Plugin* p = it.value();
        p->disconnect();
        if (p->isEnabled())
            p->Start();
        connect(p, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(onPluginStateChangedHandler()));
    }
}

void PluginsManager::stopPlugins()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    for(QMap<QString, Plugin*>::iterator it = _plugins.begin(); it != _plugins.end(); ++it){
        Plugin* p = it.value();
        p->Stop();
    }
}

void PluginsManager::onPluginStateChangedHandler()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    emit updatePlugin(_plugins.values());
}
