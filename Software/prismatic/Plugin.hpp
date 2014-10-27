#pragma once

#include <QObject>
#include <QProcess>
#include <QScopedPointer>
#include <QString>

class QSettings;

struct PluginInfo {
    QString guid;
    QString name;
    QString description;
    QString author;
    QString version;
    QString icon;
    QString exec;
    QString arguments;
};

class Plugin : public QObject
{
    Q_OBJECT
public:
    static void readPluginInfo(const QSettings&, PluginInfo* info);

    Plugin(const QString& name, const QString& path, QObject *parent = 0);
    ~Plugin();

    void Start();
    void Stop();

    QString Name() const;
    QString Guid() const;
    QString Description() const;
    QString Author() const;
    QString Version() const;
    QIcon Icon() const;


    QProcess::ProcessState state() const;
    bool isEnabled() const;
    void setEnabled(bool enable);
    void setPriority(int priority);
    int getPriority() const;


signals:
    void stateChanged(QProcess::ProcessState);

private:
    QString m_pathPlugin;
    PluginInfo m_info;
    QScopedPointer<QProcess> m_process;
};


