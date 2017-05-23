#include "mytcp.h"

myTcp::myTcp(QObject *parent):
    QObject(parent)
{
    tcpServer = new QTcpServer(this);
    peerList = QMap<QString, QTcpSocket*>();
    outBuf.resize(0);
    sumPeer = 0;
    serverPort = 6666;
    totalSize = 0;
    sizeToWrite = 0;
    sizeWritten = 0;
    isSendFile = false;
    loadSize = 64*1024; //每次文件传输的包大小上限64KB
    //loadSize = 1024*1024; //每次文件传输的包大小上限1024KB
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
    newClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    if(NULL == newClient)
    {
        qDebug() <<"socket is error!";
        return;
    }
    sumPeer += 1;
    QString clientIp = newClient->peerAddress().toString();
    peerList.insert(clientIp, newClient);
    connect(newClient, SIGNAL(readyRead()), this, SLOT(slot_readData()));
    connect(newClient, SIGNAL(disconnected()), this, SLOT(onDisConnection()));
    connect(newClient, SIGNAL(bytesWritten(qint64)), this, SLOT(slotSendFile(qint64)));
    updatePeerList();
}
void myTcp::onDisConnection()
{
    sumPeer -= 1;
    QTcpSocket *disClient = static_cast<QTcpSocket *>(this->sender());
    QString disClientIp = disClient->peerAddress().toString();
    peerList.remove(disClientIp);
    disClient->close();
}
void myTcp::updatePeerList()
{
    QMap<QString, QTcpSocket*>::Iterator iter;
    for(iter = peerList.begin(); iter != peerList.end(); )
    {
        QAbstractSocket::SocketState curState = (*iter)->state();
        if(QAbstractSocket::UnconnectedState == curState ||
                QAbstractSocket::ClosingState == curState)
        {
            qDebug() << "erase closed socket user : " << iter.key();
            iter = peerList.erase(iter);
        } else
            ++iter;
    }
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
    QString result;
    qDebug() << "service recieve user: " << clientIp << " sended text";
    QStringList list = info.split(' ');
    if(list.at(0) == "register") {
        result = conn->userRegister(info);
        sendDate(makerSocket, result);
        qDebug()<< clientIp<<"register succeed.";
    }
    else if(list.at(0) == "login") {
        result = conn->userLogin(info);
        sendDate(makerSocket, result);
        if(result == "true")
            qDebug()<< clientIp<<"login succeed.";
        else
            qDebug()<< clientIp<<"register failed.";
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
        QString pwd = (info.split('#')).at(2);
        qDebug() << "pwd:" << pwd;
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
    else
    {
        return;//非服务器能接受处理的数据
    }
}
void myTcp::sendDate(QTcpSocket *targetSocket, const QString &str)
{
    int len = targetSocket->write(str.toUtf8(), str.length() + 1);
    targetSocket->flush();
    if(len > 0)
    {
        QString clientIp = targetSocket->peerAddress().toString();
    }
}
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
    }
    if(sizeToWrite == 0){
        cryptoFile->close();
        cryptoFile->remove();
        file = NULL;
        cryptoFile = NULL;
        totalSize = 0;
        sizeToWrite = 0;
        isSendFile = false;
        outBuf.resize(0);
        QString clientIp = makerSocket->peerAddress().toString();
        qDebug() << "send file to " << clientIp << " succeed.";
        makerSocket->close();
    }
}
