#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include "mytcpsocket.h"

#include <QObject>
#include <QTcpServer>
#include <QThreadPool>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer& getInstance();
    void incomingConnection(qintptr handle) override;
    void removeSocket(MyTcpSocket* mysocket);
    void resend(char* tarName, PDU* pdu);
private:
    MyTcpServer();
    MyTcpServer(const MyTcpServer& instance)=delete;
    MyTcpServer& operator=(const MyTcpServer&)=delete;
    QList<MyTcpSocket*> m_tcpSocketList;
    QThreadPool threadPool;
};

#endif // MYTCPSERVER_H
