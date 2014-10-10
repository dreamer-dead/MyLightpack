#include "PluginsManager.hpp"

#include <QDir>

#include "Plugin.hpp"
#include "common/DebugOut.hpp"

// static
QString PluginsManager::defaultPluginsDir() {
    return QString("Plugins");
}

PluginsManager::PluginsManager(const QString& pluginsDir, QObject *parent)
    : QObject(parent)
    , m_pluginsDir(pluginsDir) {
    Q_ASSERT(QDir(m_pluginsDir).exists());
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
        QString name = p->Name();
        p->Stop();
        delete p;
    }
    _plugins.clear();
}

void PluginsManager::reloadPlugins(){
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    dropPlugins();
    LoadPlugins(m_pluginsDir);
    StartPlugins();
}

void PluginsManager::LoadPlugins(QString path)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << path;

    //QStringList pluginPaths = QApplication::libraryPaths();
   // path = QString(Settings::getApplicationDirPath() + "Plugins");
    QDir dir(path);
    //QStringList files = dir.entryList(QStringList("*.py"), QDir::Files);

    QStringList lstDirs = dir.entryList(QDir::Dirs |
                                        QDir::AllDirs |
                                        QDir::NoDotAndDotDot); //Get directories

    foreach(QString pluginDir, lstDirs){
           QString plugin = QFileInfo (pluginDir).baseName ();

           QMap<QString, Plugin*>::iterator found = _plugins.find(plugin);
           if(found != _plugins.end()){
               DEBUG_LOW_LEVEL << "Already loaded a plugin named " << plugin << " !";
               continue;
           }

           Plugin* p = new Plugin(plugin,path+"/"+pluginDir,this);
           //DEBUG_LOW_LEVEL <<p->getName()<<  p->getAuthor() << p->getDescription() << p->getVersion();
           //connect(p, SIGNAL(executed()), this, SIGNAL(pluginExecuted()));
           _plugins[plugin] = p;
   }

    emit updatePlugin(_plugins.values());

}

Plugin* PluginsManager::getPlugin(const QString& name_){
    QMap<QString, Plugin*>::iterator found = _plugins.find(name_);
    if(found != _plugins.end())
        return found.value();
    else
        return 0;
}

QList<Plugin*> PluginsManager::getPluginList(){
    return _plugins.values();
}



void PluginsManager::StartPlugins()
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

void PluginsManager::StopPlugins()
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
