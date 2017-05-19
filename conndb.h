#ifndef CONNDB_H
#define CONNDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QString>
#include <QTime>
#include <QStringList>

//#include <QTextStream>
//#include <stdio.h>

class connDB : public QObject
{
    Q_OBJECT
public:
    static connDB *getDB()
    {
        static connDB instance;
        return &instance;
    }
    QString userRegister(QString str);
    QString userLogin(QString str);
    QString userRead(QString str);
    QString userList(QString str = "");
    void addBooks(QStringList list, QString path);
    void removeBooks(QStringList list);
//    void ctrlDB(QString str);
private:
    QSqlDatabase db;

    explicit connDB(QObject *parent = 0);
    ~connDB();
signals:

public slots:
    //数据库断开连接，未处理
};

#endif // CONNDB_H
