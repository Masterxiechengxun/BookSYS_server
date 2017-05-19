#include "conndb.h"

connDB::connDB(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("booksys");
    db.setUserName("root");
    db.setPassword("xiechengxun");
    if(db.open())
        qDebug() << "database open succeed!";
    else
        qDebug() << "database open false!";
}
connDB::~connDB()
{
    if(db.isOpen())
        db.close();
}
QString connDB::userLogin(QString str)
{
    QStringList list = str.split(' ');
    QSqlQuery query;
    QString name = list.at(1);
    QString pwd = list.at(2);
    query.exec("SELECT name, password FROM user_info");
    while (query.next()) {
        if(name == query.value(0).toString() && pwd == query.value(1).toString())
        {
            query.clear();
            return "true";
        }
    }
    query.clear();
    return "false";
}
QString connDB::userRegister(QString str)
{
    QStringList list = str.split(' ');
    QSqlQuery query;
    QString name = list.at(1);
    QString pwd = list.at(2);
    QString mail;
    QString phone;
    if(list.at(3) == '#' && list.at(4) == '#')
    {
        query.prepare("INSERT INTO user_info(name, password) VALUES (?, ?)");
        query.addBindValue(name);
        query.addBindValue(pwd);
    } else if(list.at(3) != '#' && list.at(4) == '#') {
        mail = list.at(3);
        query.prepare("INSERT INTO user_info(name, password, mailbox) VALUES (?, ?, ?)");
        query.addBindValue(name);
        query.addBindValue(pwd);
        query.addBindValue(mail);
    } else {
        mail = list.at(3);
        phone = list.at(4);
        query.prepare("INSERT INTO user_info(name, password, mailbox, phone) VALUES (?, ?, ?, ?)");
        query.addBindValue(name);
        query.addBindValue(pwd);
        query.addBindValue(mail);
        query.addBindValue(phone);
    }
    if(query.exec())
    {
        query.clear();
        return "true";
    }else
        query.clear();
        return "false";
}
QString connDB::userRead(QString str)
{
    QStringList list = str.split('#');
    QSqlQuery query;
    QString title = list.at(1);
    QString path;
    query.exec("SELECT title, path FROM book_info");
    while (query.next()) {
        if(title == query.value(0).toString())
        {
            path = query.value(1).toString();
            path.append(tr("\\%1").arg(title));
            query.clear();
            return path;
        }
    }
    query.clear();
    return "false";
}
QString connDB::userList(QString str)
{
    //QString usrName = str;
    QSqlQuery query;
    QString booklist;
    query.exec("SELECT title FROM book_info");
    while (query.next()) {
        booklist.append(query.value(0).toString().toUtf8());
        booklist.append('#');
    }
    query.clear();
    return booklist;
}
void connDB::addBooks(QStringList list,QString path)
{
    QSqlQuery query;
    query.prepare("INSERT INTO book_info(title, path) VALUES (?, ?)");
    for(int i = 0; i < list.size(); i++)
    {
        query.addBindValue(list.at(i));
        query.addBindValue(path);
        query.exec();
    }
    query.clear();
}
void connDB::removeBooks(QStringList list)
{
    QSqlQuery query;
    query.prepare("DELETE FROM book_info WHERE title = ?");
    for(int i = 0; i < list.size(); i++)
    {
        query.addBindValue(list.at(i));
        query.exec();
    }
    query.clear();
}
