#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "protocol.h"

#include <QFile>
#include <QString>



class MsgHandler
{
public:
    PDU* pdu;
    QFile m_fUploadFile;
    qint64 m_iUploadFileSize;
    qint64 m_iUploadReceivedSize;
    MsgHandler();
    PDU* regist();
    PDU* login(QString& strName);
    PDU* findUser();
    PDU* onlineUser();
    PDU* addFriend();
    PDU* addFriendAgree();
    PDU* flushFriend();
    PDU* chat();
    PDU* mkdir();
    PDU* flushFile();
    PDU* delFile();
    PDU* uploadFileInit();
    PDU* uploadFileData();
    PDU* shareFile();
    PDU* shareFileAgree();
};

#endif // MSGHANDLER_H
