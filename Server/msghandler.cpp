#include "msghandler.h"
#include "mytcpserver.h"
#include "operatedb.h"
#include "server.h"

#include <QDebug>
#include <QDir>

MsgHandler::MsgHandler()
{
}

PDU *MsgHandler::regist()
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    memcpy(caPwd, pdu->caData+32, 32);
    bool ret = OperateDB::getInstance().handleRegist(caName, caPwd);
    qDebug() << "regist ret" << ret;
    if (ret) {
        QDir dir;
        QString strRootPath = Server::getInstance().m_strRootPath;
        QString strUserPath = QString("%1/%2").arg(strRootPath).arg(caName);
        dir.mkdir(strUserPath);
        qDebug() << "创建用户目录" << strUserPath;
    }
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_REGIST_RESPOND;
    return respdu;
}

PDU *MsgHandler::login(QString &strName)
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    memcpy(caPwd, pdu->caData+32, 32);
    bool ret = OperateDB::getInstance().handleLogin(caName, caPwd);
    qDebug() << "login ret" << ret;
    if (ret) {
        strName = caName;
    }
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_LOGIN_RESPOND;
    return respdu;
}

PDU *MsgHandler::findUser()
{
    char caName[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    int ret = OperateDB::getInstance().handleFindUser(caName);
    qDebug() << "find user ret" << ret;
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (int));
    respdu->uiType = ENUM_MSG_TYPE_FIND_USER_RESPOND;
    return respdu;
}

PDU *MsgHandler::onlineUser()
{
    QStringList ret = OperateDB::getInstance().handleOnlineUser();
    qDebug() << "ret.size" << ret.size();
    PDU* respdu = mkPDU(ret.size()*32);
    for (int i=0; i<ret.size(); i++) {
        qDebug() << "name" << ret[i];
        memcpy(respdu->caMsg+i*32, ret[i].toStdString().c_str(), 32);
    }
    respdu->uiType = ENUM_MSG_TYPE_ONLINE_USER_RESPOND;
    return respdu;
}

PDU *MsgHandler::addFriend()
{
    char caCurName[32] = {'\0'};
    char caTarName[32] = {'\0'};
    memcpy(caCurName, pdu->caData, 32);
    memcpy(caTarName, pdu->caData+32, 32);
    int ret = OperateDB::getInstance().handleAddFriend(caCurName, caTarName);
    qDebug() << "addFriend ret" << ret;
    if (ret == 1) {
        MyTcpServer::getInstance().resend(caTarName, pdu);
        return NULL;
    }
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (int));
    respdu->uiType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
    return respdu;
}

PDU *MsgHandler::addFriendAgree()
{
    char caCurName[32] = {'\0'};
    char caTarName[32] = {'\0'};
    memcpy(caCurName, pdu->caData, 32);
    memcpy(caTarName, pdu->caData+32, 32);
    bool ret = OperateDB::getInstance().handleAddFriendAgree(caCurName, caTarName);
    qDebug() << "addFriendAgree ret" << ret;
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE_RESPOND;
    MyTcpServer::getInstance().resend(caCurName, respdu);
    return respdu;
}

PDU *MsgHandler::flushFriend()
{
    char caName[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    QStringList res = OperateDB::getInstance().handleFlushFriend(caName);
    PDU* respdu = mkPDU(res.size()*32);
    respdu->uiType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
    for (int i=0; i<res.size(); i++) {
        memcpy(respdu->caMsg+i*32, res[i].toStdString().c_str(), 32);
    }
    return respdu;
}

PDU *MsgHandler::chat()
{
    char caTarName[32] = {'\0'};
    memcpy(caTarName, pdu->caData+32, 32);
    MyTcpServer::getInstance().resend(caTarName, pdu);
    return NULL;
}

PDU *MsgHandler::mkdir()
{
    char caDirName[32] = {'\0'};
    memcpy(caDirName, pdu->caData, 32);
    QString strDirPath = QString("%1/%2").arg(pdu->caMsg).arg(caDirName);
    QDir dir;
    bool ret = dir.mkdir(strDirPath);
    qDebug() << "mkdir ret:" << ret;
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_MKDIR_RESPOND;
    return respdu;
}

PDU *MsgHandler::flushFile()
{
    QDir dir(pdu->caMsg);
    QFileInfoList fileInfoList = dir.entryInfoList();
    PDU* respdu = mkPDU(sizeof(FileInfo) * (fileInfoList.size()-2));
    respdu->uiType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
    for (int i=0, j=0; i<fileInfoList.size(); i++) {
        if (fileInfoList[i].fileName() == QString(".") || fileInfoList[i].fileName() == QString("..")) {
            continue;
        }
        FileInfo* pFileInfo = (FileInfo*)respdu->caMsg+j++;
        memcpy(pFileInfo->caName, fileInfoList[i].fileName().toStdString().c_str(), 32);
        if (fileInfoList[i].isDir()) {
            pFileInfo->uiType = 0;
        } else {
            pFileInfo->uiType = 1;
        }
        qDebug() << "caName" << pFileInfo->caName << "uiType" << pFileInfo->uiType;
    }
    return respdu;
}

PDU *MsgHandler::delFile()
{
    char* pPath = pdu->caMsg;
    uint uiType = 0;
    memcpy(&uiType, pdu->caData, sizeof(uint));
    bool ret;
    if (uiType == 0) {
        QDir dir(pPath);
        ret = dir.removeRecursively();
    } else {
        QFile file(pPath);
        ret = file.remove();
    }
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
    return respdu;
}

PDU *MsgHandler::uploadFileInit()
{
    char caFileName[32] = {'\0'};
    memcpy(caFileName, pdu->caData, 32);
    memcpy(&m_iUploadFileSize, pdu->caData+32, sizeof(qint64));
    m_iUploadReceivedSize = 0;
    QString strPath = QString("%1/%2").arg(pdu->caMsg).arg(caFileName);
    m_fUploadFile.setFileName(strPath);
    bool ret = m_fUploadFile.open(QIODevice::WriteOnly);
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND;
    return respdu;
}

PDU *MsgHandler::uploadFileData()
{
    m_fUploadFile.write(pdu->caMsg, pdu->uiMsgLen);
    m_iUploadReceivedSize += pdu->uiMsgLen;
    if (m_iUploadReceivedSize < m_iUploadFileSize) {
        return NULL;
    }
    m_fUploadFile.close();
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND;
    return respdu;
}

PDU *MsgHandler::shareFile()
{
    char strCurName[32] = {'\0'};
    int iFriendSize = 0;
    memcpy(strCurName, pdu->caData, 32);
    memcpy(&iFriendSize, pdu->caData+32, sizeof(int));
    PDU* resendpdu = mkPDU(pdu->uiMsgLen-iFriendSize*32);
    resendpdu->uiType = pdu->uiType;
    memcpy(resendpdu->caData, strCurName, 32);
    memcpy(resendpdu->caMsg, pdu->caMsg+iFriendSize*32, pdu->uiMsgLen-iFriendSize*32);
    char caTmp[32] = {'\0'};
    for (int i=0; i<iFriendSize; i++) {
        memcpy(caTmp, pdu->caMsg+i*32, 32);
        MyTcpServer::getInstance().resend(caTmp, resendpdu);
    }
    free(resendpdu);
    resendpdu = NULL;

    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    return respdu;
}

PDU *MsgHandler::shareFileAgree()
{
    QString strShareFilePath = pdu->caMsg;
    int index = strShareFilePath.lastIndexOf('/');
    QString strFileName = strShareFilePath.right(strShareFilePath.size() - index - 1);
    QString strTarPath = QString("%1/%2/%3").arg(Server::getInstance().m_strRootPath).arg(pdu->caData).arg(strFileName);
    bool ret = QFile::copy(strShareFilePath, strTarPath);
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_SHARE_FILE_AGREE_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}
