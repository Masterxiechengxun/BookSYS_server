#include "bookmanager.h"

bookManager::bookManager(QObject *parent, QString dir) : QObject(parent)
{
    if(dir == "")
        path = "C:\\pdfsys\\serverFiles";
    else
        path = dir;
    homeDir = new QDir(path);
    conn = connDB::getDB();
    QString result = conn->userList("admin");
    list = result.split('#');
    list.removeLast();
    watcher = new QFileSystemWatcher();
    watcher->addPath(path);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(changeBooks()));
}
bookManager::~bookManager()
{
    delete homeDir;
    delete watcher;
}

QFileInfoList bookManager::getFileList(QString path)
{
    QDir dir(path);
    QStringList filter;
    filter << "*.pdf";
    dir.setNameFilters(filter);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folderList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for(int i = 0; i != folderList.size(); i++)
    {
        QString name = folderList.at(i).absolutePath();
        QFileInfoList childFileList = getFileList(name);
        fileList.append(childFileList);
    }
    return fileList;
}
void bookManager::changeBooks()
{
    QFileInfoList booklist = getFileList(path);
    QStringList curList;
    for(int i = 0; i < booklist.size(); i++)
    {
        curList.append(booklist.at(i).fileName());
    }
    QStringList remChange;
    for(int i = 0; i < list.size(); i++)
    {
        if(!curList.contains(list.at(i)))
        {
            remChange.append(list.at(i)); //数据库需要删除的图书记录
        }
        else
        {
            curList.removeOne(list.at(i)); //数据库不需要改变的图书记录
        }
    }
    if(curList.size() > 0)
        addBooks(curList);                  //数据库需要添加的图书记录
    if(remChange.size() > 0)
        removeBooks(remChange);
    QString result = conn->userList("admin"); //更新关于目录下拥有的图书的记录
    list = result.split('#');
    list.removeLast();
}
void bookManager::addBooks(QStringList list)
{
    conn->addBooks(list, path);
    qDebug() << "add books:" << list;
}
void bookManager::removeBooks(QStringList list)
{
    conn->removeBooks(list);
    qDebug() << "remove books:" << list;
}
