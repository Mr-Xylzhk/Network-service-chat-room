#include "mytcpserver.h"
#include "mytcpsocket.h"
#include "operatedb.h"
#include "protocol.h"

MyTcpSocket::MyTcpSocket()
{
    connect(this, &QTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);
    connect(this, &QTcpSocket::disconnected, this, &MyTcpSocket::clientOffline);
    m_pmh = new MsgHandler;
}

MyTcpSocket::~MyTcpSocket()
{
    delete m_pmh;
}

void MyTcpSocket::sendMsg(PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }
    this->write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "send msg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData+32" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    free(pdu);
    pdu = NULL;
}

PDU *MyTcpSocket::readMsg()
{
    qDebug() << "readMsg 接收消息长度" << this->bytesAvailable();
    //先读取pdu总长度这个成变量
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));

    //根据总长度构建pdu，读取结构体中剩余的成员变量
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU* pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
    qDebug() << "recvMsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData+32" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    return pdu;
}

PDU *MyTcpSocket::handleMsg(PDU *pdu)
{
    qDebug() << "handleMsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData+32" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    PDU* respdu = NULL;
    m_pmh->pdu = pdu;
    //处理消息
    switch (pdu->uiType) {
    case ENUM_MSG_TYPE_REGIST_REQUEST: {
        respdu = m_pmh->regist();
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST: {
        respdu = m_pmh->login(m_strLoginName);
        break;
    }
    case ENUM_MSG_TYPE_FIND_USER_REQUEST: {
        respdu = m_pmh->findUser();
        break;
    }
    case ENUM_MSG_TYPE_ONLINE_USER_REQUEST: {
        respdu = m_pmh->onlineUser();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
        respdu = m_pmh->addFriend();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE_REQUEST: {
        respdu = m_pmh->addFriendAgree();
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST: {
        respdu = m_pmh->flushFriend();
        break;
    }
    case ENUM_MSG_TYPE_CHAT_REQUEST: {
        respdu = m_pmh->chat();
        break;
    }
    case ENUM_MSG_TYPE_MKDIR_REQUEST: {
        respdu = m_pmh->mkdir();
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST: {
        respdu = m_pmh->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_DEL_FILE_REQUEST: {
        respdu = m_pmh->delFile();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST: {
        respdu = m_pmh->uploadFileInit();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST: {
        respdu = m_pmh->uploadFileData();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
        respdu = m_pmh->shareFile();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_AGREE_REQUEST: {
        respdu = m_pmh->shareFileAgree();
        break;
    }
    default:
        break;
    }
    return respdu;
}

void MyTcpSocket::recvMsg()
{
    qDebug() << "recvMsg 接收消息长度" << this->bytesAvailable();
    //先读取pdu总长度这个成变量
    QByteArray data = this->readAll();
    buffer.append(data);
    while (buffer.size() >= int(sizeof(PDU))) {
        PDU* pdu = (PDU*)buffer.data();
        if (buffer.size() < int(pdu->uiTotalLen)) {
            break;
        }
        PDU* respdu = handleMsg(pdu);
        sendMsg(respdu);
        buffer.remove(0, pdu->uiTotalLen);
    }
}

void MyTcpSocket::clientOffline()
{
    OperateDB::getInstance().handleOffline(m_strLoginName.toStdString().c_str());
    MyTcpServer::getInstance().removeSocket(this);
}
