#ifndef MYTCP_H
#define MYTCP_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QCryptographicHash>

#include "conndb.h"
#include "bookmanager.h"
#include "aes.h"

using namespace std;

class myTcp : public QObject
{
    Q_OBJECT
public:
    myTcp(QObject *parent = 0);
    ~myTcp();
public slots:
    void onNewConnection();
    void slot_readData();
    void onDisConnection();
    void slotSendFile(qint64);
private:
    QTcpServer *tcpServer;
    quint16 serverPort;
    int sumPeer;
    QMap<QString, QTcpSocket*> peerList;
    connDB *conn;
    QFile *file;
    QFile *cryptoFile;
    QByteArray outBuf;
    qint64 totalSize;
    qint64 sizeToWrite;
    qint64 sizeWritten;
    qint64 loadSize;
    bool isSendFile;
private:
    void sendDate(QTcpSocket *targetSocket, const QString &str);
    void updatePeerList();
};

#endif // MYTCP_H
