#pragma once

#include <QObject>
#include <QMap>
#include <QProcess>

class Plugin;

class PluginsManager : public QObject
{
    Q_OBJECT
public:
    static QString defaultPluginsDir();

    PluginsManager(const QString& pluginsDir, QObject *parent = 0);
    virtual ~PluginsManager();

    void LoadPlugins(QString path);
    QList<Plugin*> getPluginList();
    Plugin* getPlugin(const QString& name_);

signals:
    void updatePlugin(QList<Plugin*>);
    
public slots:
    void reloadPlugins();
    void StartPlugins();
    void StopPlugins();

private slots:
    void onPluginStateChangedHandler();

private:
    void dropPlugins();

    QMap<QString, Plugin*> _plugins;
    const QString m_pluginsDir;
};

