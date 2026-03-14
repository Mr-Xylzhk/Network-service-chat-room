#ifndef UPLOADER_H
#define UPLOADER_H

#include "protocol.h"

#include <QObject>

class Uploader : public QObject
{
    Q_OBJECT
public:

    QString m_strUploadPath;
    Uploader(QString strFilePath);
    void start();

public slots:
    void uploadFile();

signals:
    void errorSignal(const QString strMsg);
    void uploadPDU(PDU* pdu);
    void finished();
};

#endif // UPLOADER_H
