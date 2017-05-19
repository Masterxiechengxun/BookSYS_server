#ifndef BOOKMANAGER_H
#define BOOKMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QFileSystemWatcher>
#include "conndb.h"

class bookManager : public QObject
{
    Q_OBJECT
public:
    explicit bookManager(QObject *parent = 0, QString dir = 0);
    ~bookManager();
    void addBooks(QStringList list);
    void removeBooks(QStringList list);
private:
    QFileInfoList getFileList(QString path);
    connDB *conn;
    QString path;
    QDir *homeDir;
    QStringList list;
    QFileSystemWatcher *watcher;
signals:

private slots:
    void changeBooks();
};

#endif // BOOKMANAGER_H
