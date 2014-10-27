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

public slots:
    void Start();
    void Stop();

signals:
    void stateChanged(QProcess::ProcessState);

private slots:
    void processStateChanged(QProcess::ProcessState);
    void processFinished(int exitCode);

private:
    QString m_pathPlugin;
    PluginInfo m_info;
    QProcess::ProcessState m_processState;
    QScopedPointer<QProcess> m_process;
};


