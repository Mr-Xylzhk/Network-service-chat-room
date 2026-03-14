#include "protocol.h"
#include "uploader.h"

#include <QFile>
#include <QThread>


void Uploader::uploadFile()
{
    QFile file(m_strUploadPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorSignal("上传文件失败");
        emit finished();
        return;
    }
    while (true) {
        PDU* pdu = mkPDU(4096);
        pdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST;
        qint64 ret = file.read(pdu->caMsg, 4096);
        if (ret < 0) {
            emit errorSignal("上传文件失败");
            break;
        }
        if (ret == 0) {
            break;
        }
        pdu->uiMsgLen = ret;
        pdu->uiTotalLen = ret + sizeof(PDU);
        emit uploadPDU(pdu);
    }
    file.close();
    emit finished();
}

Uploader::Uploader(QString strFilePath)
{
    m_strUploadPath = strFilePath;
}

void Uploader::start()
{
    QThread* thread = new QThread;
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &Uploader::uploadFile, Qt::QueuedConnection);
    connect(this, &Uploader::finished, thread, &QThread::quit, Qt::QueuedConnection);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);
    thread->start();
}
