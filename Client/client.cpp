#include "client.h"
#include "index.h"
#include "protocol.h"
#include "ui_client.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    loadConfig();
    socket.connectToHost(QHostAddress(m_strIP), m_usPort);
    connect(&socket, &QTcpSocket::connected, this, &Client::showConnect);
    connect(&socket, &QTcpSocket::readyRead, this, &Client::recvMsg);
    m_prh = new ResHandler;
}

PDU *Client::readMsg()
{
    qDebug() << "readMsg 接收消息长度" << socket.bytesAvailable();
    //先读取pdu总长度这个成变量
    uint uiPDULen = 0;
    socket.read((char*)&uiPDULen, sizeof(uint));

    //根据总长度构建pdu，读取结构体中剩余的成员变量
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU* pdu = mkPDU(uiMsgLen);
    socket.read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
    qDebug() << "readMsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    return pdu;
}

void Client::handleMsg(PDU *pdu)
{
    qDebug() << "handleMsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    //处理消息
    m_prh->pdu = pdu;
    switch(pdu->uiType) {
    case ENUM_MSG_TYPE_REGIST_RESPOND: {
        m_prh->regist();
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND: {
        m_prh->login();
        break;
    }
    case ENUM_MSG_TYPE_FIND_USER_RESPOND: {
        m_prh->findUser();
        break;
    }
    case ENUM_MSG_TYPE_ONLINE_USER_RESPOND: {
        m_prh->onlineUser();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {
        m_prh->addFriend();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
        m_prh->addFriendResend();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE_RESPOND: {
        m_prh->addFriendAgree();
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: {
        m_prh->flushFriend();
        break;
    }
    case ENUM_MSG_TYPE_CHAT_REQUEST: {
        m_prh->chat();
        break;
    }
    case ENUM_MSG_TYPE_MKDIR_RESPOND: {
        m_prh->mkdir();
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND: {
        m_prh->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_DEL_FILE_RESPOND: {
        m_prh->delFile();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND: {
        m_prh->uploadFileInit();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND: {
        Index::getInstance().getFile()->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: {
        QMessageBox::information(&Index::getInstance(), "分享文件", "分享已发送");
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
        m_prh->shareFileResend();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_AGREE_RESPOND: {
        m_prh->shareFileAgree();
        break;
    }
    default:
        break;
    }
}

Client::~Client()
{
    delete ui;
    delete m_prh;
}

void Client::loadConfig()
{
    QFile file(":/connect.config");    //配置文件固定冒号开头
    if (file.open(QIODevice::ReadOnly)) {
        QString strData = QString(file.readAll());
        QStringList strList = strData.split("\r\n");
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        m_strRootPath = strList[2];
        qDebug() << "loadConfig strIP" << m_strIP << "usPort" << m_usPort << "strRootPath" << m_strRootPath;
        file.close();
    } else {
        QMessageBox::critical(this, "提示", "打开文件失败");
        qDebug() << "loadConfig 打开文件失败";
    }
}

Client &Client::getInstance()
{
    static Client instance;
    return instance;
}

void Client::sendMsg(PDU *pdu)
{
    socket.write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "send msg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData+32" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    free(pdu);
    pdu = NULL;
}

void Client::showConnect()
{
    qDebug() << "连接服务器成功";
}

void Client::recvMsg()
{
    qDebug() << "recvMsg 接收消息长度" << socket.bytesAvailable();
    //先读取pdu总长度这个成变量
    QByteArray data = socket.readAll();
    buffer.append(data);
    while (buffer.size() >= int(sizeof(PDU))) {
        PDU* pdu = (PDU*)buffer.data();
        if (buffer.size() < int(pdu->uiTotalLen)) {
            break;
        }
        handleMsg(pdu);
        buffer.remove(0, pdu->uiTotalLen);
    }
}



void Client::on_regist_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd = ui->pwd_LE->text();
    if (strName.isEmpty() || strPwd.isEmpty() ||
            strName.toStdString().size() > 32 || strPwd.toStdString().size() > 32) {
        QMessageBox::information(this, "提示", "用户名或密码长度非法");
        return;
    }
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    memcpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
    pdu->uiType = ENUM_MSG_TYPE_REGIST_REQUEST;
    sendMsg(pdu);
}

void Client::on_login_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd = ui->pwd_LE->text();
    if (strName.isEmpty() || strPwd.isEmpty() ||
            strName.toStdString().size() > 32 || strPwd.toStdString().size() > 32) {
        QMessageBox::information(this, "提示", "用户名或密码长度非法");
        return;
    }
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    m_strLoginName = strName;
    memcpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
    pdu->uiType = ENUM_MSG_TYPE_LOGIN_REQUEST;
    sendMsg(pdu);
}
