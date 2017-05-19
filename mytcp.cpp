#include "mytcp.h"

myTcp::myTcp(QObject *parent):
    QObject(parent)
{
    tcpServer = new QTcpServer(this);
    peerList = QMap<QTcpSocket*, QString>();
    outBuf.resize(0);
    sumPeer = 0;
    serverPort = 6666;
    totalSize = 0;
    sizeToWrite = 0;
    isSendFile = false;
    loadSize = 64*1024; //每次文件传输的包大小上限64KB
    file = NULL;
    cryptoFile = NULL;
    if(!tcpServer->listen(QHostAddress::Any, serverPort))
    {
        qDebug() <<"service start listener failed!";
        tcpServer->close();
    } else {
        qDebug() << "service startup listener  succeed!";
        conn = connDB::getDB();
    }
    connect(tcpServer, &QTcpServer::newConnection, this, &myTcp::onNewConnection);
}
myTcp::~myTcp()
{
    if(!tcpServer)
        delete tcpServer;
}
void myTcp::onNewConnection()
{
    QTcpSocket *newClient = tcpServer->nextPendingConnection();
    if(NULL == newClient)
    {
        qDebug() <<"socket is error!";
        return;
    }
    sumPeer += 1;
    QString clientIp = newClient->peerAddress().toString();
    qDebug() << "current user count: " << sumPeer
             << ", and new clientIp :" << clientIp;
    peerList.insert(newClient,"");
    connect(newClient, SIGNAL(readyRead()), this, SLOT(slot_readData()));
    connect(newClient, SIGNAL(disconnected()), this, SLOT(onDisConnection()));
    connect(newClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorHandle(QAbstractSocket::SocketError)));
    connect(newClient, SIGNAL(bytesWritten(qint64)), this, SLOT(slotSendFile(qint64)));
    //updatePeerList();
}
void myTcp::onDisConnection()
{
    sumPeer -= 1;
    QTcpSocket *disClient = static_cast<QTcpSocket *>(this->sender());
    QString disClientIp = disClient->peerAddress().toString();
    qDebug() << "current user count: " << sumPeer
             << ", and leave clientIp :" << disClientIp;
    //peerList.remove(disClientIp);
    disClient->close();
}
void myTcp::slot_readData()
{
    QTcpSocket *makerSocket = static_cast<QTcpSocket *>(this->sender());
    QByteArray byteData;
    while(makerSocket->bytesAvailable() > 0)
    {
        byteData = makerSocket->readAll();
    }
    QString clientIp = makerSocket->peerAddress().toString();
    QString info = static_cast<QString>(byteData);
    //qDebug() << info;
    QString result;
    qDebug() << "service recieve user: " << clientIp << " sended text: " << info;
    QStringList list = info.split(' ');
    if(list.at(0) == "register") {
        result = conn->userRegister(info);
        sendDate(makerSocket, result);
    }
    else if(list.at(0) == "login") {
        result = conn->userLogin(info);
        sendDate(makerSocket, result);
        peerList[makerSocket] = list.at(2);
    }
    else if(list.at(0) == "booklist")
    {
        result = conn->userList(list.at(1));
        sendDate(makerSocket, result.toUtf8());
    }
    else if(list.at(0) == "read")
    {
        result = conn->userRead(info);
        file = new QFile(result);
        //通过计算用户密码的Hash值来获得AES秘钥
        QString pwd = peerList[makerSocket];
        QByteArray key;
        QByteArray p;
        key = QCryptographicHash::hash(p.append(pwd), QCryptographicHash::Md5);
        unsigned char *aesKey = (unsigned char *)key.data();
        //使用AES算法加密读者希望阅读的图书
        AES aes(Bits128, aesKey);
        QString curFileName = result.right(result.size() - result.lastIndexOf('\\') - 1);
        QString cryptoFileName = result.left(result.lastIndexOf("\\") + 1);
        QDir *workdir = new QDir(cryptoFileName + "EncryptedBooks");
        if(!workdir->exists())  //创建用于加密图书的工作目录
            workdir->mkdir(cryptoFileName + "EncryptedBooks");
        cryptoFileName = cryptoFileName + "EncryptedBooks\\" + curFileName;
        cryptoFile = new QFile(cryptoFileName);
        if(cryptoFile->exists())
            cryptoFile->remove();
        //开始AES文件加密
        aes.FileCipher(file, cryptoFile);
        //开始传输文件描述信息
        if(cryptoFile->open(QFile::ReadOnly))
        {
            isSendFile = true;
            totalSize = cryptoFile->size();
            sizeToWrite = totalSize;  
            sendDate(makerSocket, tr("file#%1#%2").arg(totalSize).arg(curFileName));
        } else {
            qDebug() << "open file failed!";
        }
    }
}
void myTcp::sendDate(QTcpSocket *targetSocket, const QString &str)
{
    int len = targetSocket->write(str.toUtf8(), str.length() + 1);
    if(len > 0)
    {
        QString clientIp = targetSocket->peerAddress().toString();
        qDebug() << "service send user: " << clientIp << " text: " << str;
    }
}
void myTcp::errorHandle(QAbstractSocket::SocketError errors)
{
    QTcpSocket * errorSocket = static_cast<QTcpSocket *>(this->sender());
    //peerList.remove(errorSocket->peerAddress().toString());
    //sumPeer -= 1;
    qDebug() << "warning! user: "<< errorSocket->peerAddress().toString() << " is left.";
}
//void myTcp::updatePeerList()
//{
//    QMap<QString, QTcpSocket*>::Iterator iter;
//    for(iter = peerList.begin(); iter != peerList.end(); )
//    {
//        QAbstractSocket::SocketState curState = (*iter)->state();
//        if(QAbstractSocket::UnconnectedState == curState ||
//                QAbstractSocket::ClosingState == curState)
//        {
//            qDebug() << "erase closed socket user : " << iter.key();
//            //sumPeer -= 1;
//            iter = peerList.erase(iter);
//        } else
//            ++iter;
//    }
//}
void myTcp::slotSendFile(qint64 numBytes)
{
    if(!isSendFile)
        return;
    QTcpSocket *makerSocket = static_cast<QTcpSocket *>(this->sender());
    outBuf.resize(0);
    if(sizeToWrite > 0 ) {    //服务端还要写的字节数
        outBuf = cryptoFile->read(qMin(sizeToWrite,loadSize));  //最大发送64KB的数据包
        sizeToWrite -= (static_cast<int>(makerSocket->write(outBuf)));
        outBuf.resize(0);
    } else {
        cryptoFile->close();
        file = NULL;
        cryptoFile = NULL;
        totalSize = 0;
        sizeToWrite = 0;
        isSendFile = false;
        outBuf.resize(0);
        qDebug() << "file send succeed.";
    }
}
