#include "clienttask.h"
#include "mytcpserver.h"
#include "mytcpsocket.h"

MyTcpServer::MyTcpServer()
{
    threadPool.setMaxThreadCount(8);
}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "新客户端连接";
    MyTcpSocket* pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(handle);
    m_tcpSocketList.append(pTcpSocket);
    ClientTask* task = new ClientTask(pTcpSocket);
    threadPool.start(task);
}

void MyTcpServer::removeSocket(MyTcpSocket *mysocket)
{
    if (mysocket == NULL) {
        return;
    }
    m_tcpSocketList.removeOne(mysocket);
    mysocket->deleteLater();
    mysocket = NULL;
    for (int i = 0; i < m_tcpSocketList.size(); i++) {
        qDebug() << m_tcpSocketList[i]->m_strLoginName;
    }
}

void MyTcpServer::resend(char *tarName, PDU *pdu)
{
    if (tarName == NULL || pdu == NULL) {
        return;
    }
    for (int i=0; i<m_tcpSocketList.size(); i++) {
        if (tarName == m_tcpSocketList[i]->m_strLoginName) {
            m_tcpSocketList[i]->write((char*)pdu, pdu->uiTotalLen);
            qDebug() << "resend msg pdu->uiTotalLen" << pdu->uiTotalLen
                     << "pdu->uiMsgLen" << pdu->uiMsgLen
                     << "pdu->uiType" << pdu->uiType
                     << "pdu->caData" << pdu->caData
                     << "pdu->caData+32" << pdu->caData+32
                     << "pdu->caMsg" << pdu->caMsg;
        }
    }
}
