#include "client.h"
#include "index.h"
#include "reshandler.h"
#include "stdlib.h"
#include "string.h"

#include <QMessageBox>

ResHandler::ResHandler()
{

}

void ResHandler::regist()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        QMessageBox::information(&Client::getInstance(), "提示", "注册成功");
    } else {
        QMessageBox::information(&Client::getInstance(), "提示", "注册失败");
    }
}

void ResHandler::login()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        //展示首页，隐藏登录界面
        Index::getInstance().show();
        Client::getInstance().hide();
    } else {
        QMessageBox::information(&Client::getInstance(), "提示", "登录失败");
    }
}

void ResHandler::findUser()
{
    int ret;
    memcpy(&ret, pdu->caData, sizeof(int));
    if (ret == 2) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不存在");
    } else if (ret == 1) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户在线");
    } else if (ret == 0) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不在线");
    }
}

void ResHandler::onlineUser()
{
    int iSize = pdu->uiMsgLen/32;
    QStringList nameList;
    char caTmp[32] = {'\0'};
    for (int i=0; i<iSize; i++) {
        memcpy(caTmp, pdu->caMsg+i*32, 32);
        nameList.append(caTmp);
    }
    Index::getInstance().getFriend()->m_pOnlineUser->updateOnlineUser(nameList);
}

void ResHandler::addFriend()
{
    int ret;
    memcpy(&ret, pdu->caData, sizeof(int));
    if (ret == -2) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户已经是好友");
    } else if (ret == 0) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不在线");
    }
}

void ResHandler::addFriendResend()
{
    char caName[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    int ret = QMessageBox::question(&Index::getInstance(), "添加好友", QString("是否同意 %1 的添加好友请求？").arg(caName));
    if (ret != QMessageBox::Yes) {
        return;
    }
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE_REQUEST;
    memcpy(respdu->caData, pdu->caData, 64);
    Client::getInstance().sendMsg(respdu);
}

void ResHandler::addFriendAgree()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        Index::getInstance().getFriend()->flushFriend();
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "添加好友失败");
    }
}

void ResHandler::flushFriend()
{
    int iSize = pdu->uiMsgLen/32;
    QStringList nameList;
    char caTmp[32] = {'\0'};
    for (int i=0; i<iSize; i++) {
        memcpy(caTmp, pdu->caMsg+i*32, 32);
        nameList.append(caTmp);
    }
    Index::getInstance().getFriend()->flushFriend_LW(nameList);
}

void ResHandler::chat()
{
    char caChatName[32] = {'\0'};
    memcpy(caChatName, pdu->caData, 32);
    Chat* c = Index::getInstance().getFriend()->m_pChat;
    c->updateShow_TE(QString("%1: %2").arg(caChatName).arg(pdu->caMsg));
    if (c->isHidden()) {
        c->show();
    }
    c->m_strChatName = caChatName;
}

void ResHandler::mkdir()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        Index::getInstance().getFile()->flushFile();
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "创建文件夹失败");
    }
}

void ResHandler::flushFile()
{
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    QList<FileInfo*> pFileList;
    for (int i=0; i<iCount; i++) {
        FileInfo* pFileInfo = new FileInfo;
        memcpy(pFileInfo, pdu->caMsg+i*sizeof(FileInfo), sizeof(FileInfo));
        pFileList.append(pFileInfo);
    }
    Index::getInstance().getFile()->updateFileList(pFileList);
}

void ResHandler::delFile()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        Index::getInstance().getFile()->flushFile();
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "删除文件失败");
    }
}

void ResHandler::uploadFileInit()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        Index::getInstance().getFile()->uploadFile();
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "上传文件失败");
    }
}

void ResHandler::shareFileResend()
{
    QString strSharePath = QString(pdu->caMsg);
    int index = strSharePath.lastIndexOf('/');
    QString strFileName = strSharePath.right(strSharePath.size() - index - 1);
    QString strMsg = QString("%1 分享文件：%2\n是否接收？").arg(pdu->caData).arg(strFileName);
    int ret = QMessageBox::question(&Index::getInstance(), "分享文件", strMsg);
    if (ret != QMessageBox::Yes) {
        return;
    }
    PDU* respdu = mkPDU(pdu->uiMsgLen);
    respdu->uiType = ENUM_MSG_TYPE_SHARE_FILE_AGREE_REQUEST;
    memcpy(respdu->caData, Client::getInstance().m_strLoginName.toStdString().c_str(), 32);
    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
    Client::getInstance().sendMsg(respdu);
}

void ResHandler::shareFileAgree()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret) {
        QMessageBox::information(&Index::getInstance(), "提示", "分享文件完成");
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "分享文件失败");
    }
}
