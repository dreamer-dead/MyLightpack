#pragma once

#include <QDir>
#include <QObject>
#include <QMap>

class Plugin;

class PluginsManager : public QObject
{
    Q_OBJECT
public:
    static QString defaultPluginsDir();

    PluginsManager(const QString& pluginsDir, QObject *parent = 0);
    virtual ~PluginsManager();

    QList<Plugin*> getPluginList() const;
    Plugin* getPlugin(const QString& name_) const;

signals:
    void updatePlugin(QList<Plugin*>);
    
public slots:
    void reloadPlugins();
    void startPlugins();
    void stopPlugins();

private slots:
    void onPluginStateChangedHandler();

private:
    void dropPlugins();
    void loadPlugins(const QDir& pluginsDir);

    QMap<QString, Plugin*> _plugins;
    const QString m_pluginsDir;
};

