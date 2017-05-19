#include <QCoreApplication>
#include <QDebug>
#include "mytcp.h"
#include "conndb.h"
#include "bookmanager.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    connDB::getDB();        //开启数据库服务
    myTcp service;          //开启tcp连接服务
    bookManager manager;    //开启目录自动管理服务
    return a.exec();
}
