#include "client.h"
#include "file.h"
#include "ui_file.h"
#include "uploader.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

File::File(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::File)
{
    ui->setupUi(this);
    m_strUserPath = QString("%1/%2").arg(Client::getInstance().m_strRootPath).arg(Client::getInstance().m_strLoginName);
    m_strCurPath = m_strUserPath;
    m_pShareFile = new ShareFile;
    flushFile();
}

File::~File()
{
    delete ui;
    delete m_pShareFile;
}

void File::updateFileList(QList<FileInfo *> pFileList)
{
    foreach (FileInfo *pFileInfo, m_pFileInfoList) {
        delete pFileInfo;
    }
    m_pFileInfoList = pFileList;
    ui->listWidget->clear();
    foreach (FileInfo *pFileInfo, pFileList) {
        QListWidgetItem* pItem = new QListWidgetItem;
        if (pFileInfo->uiType == 0) {
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        } else if (pFileInfo->uiType == 1) {
            pItem->setIcon(QIcon(QPixmap(":/file.png")));
        }
        pItem->setText(pFileInfo->caName);
        qDebug() << "pFileInfo->caName" << pFileInfo->caName;
        ui->listWidget->addItem(pItem);
    }
}

void File::flushFile()
{
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::uploadFile()
{
    Uploader* uploader = new Uploader(m_strUploadPath);
    connect(uploader, &Uploader::errorSignal, this, &File::errorBox, Qt::QueuedConnection);
    connect(uploader, &Uploader::uploadPDU, &Client::getInstance(), &Client::sendMsg, Qt::QueuedConnection);
    uploader->start();
}

void File::on_mkDir_PB_clicked()
{
    QString strDirName = QInputDialog::getText(this, "新建文件夹", "文件夹名：");
    if (strDirName.isEmpty() || strDirName.toStdString().size() > 32) {
        QMessageBox::information(this, "提示", "文件夹名长度非法");
        return;
    }
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiType = ENUM_MSG_TYPE_MKDIR_REQUEST;
    memcpy(pdu->caData, strDirName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_flush_PB_clicked()
{
    flushFile();
}

void File::on_delFile_PB_clicked()
{
    //判断是否选择了文件
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if (!pItem) {
        return;
    }
    //是否确认删除
    int ret = QMessageBox::question(this, "删除文件", QString("是否删除文件 %1").arg(pItem->text()));
    if (ret != QMessageBox::Yes) {
        return;
    }
    //构建pdu，文件完整路径放入caMsg，文件类型放入caData
    QString strPath = QString("%1/%2").arg(m_strCurPath).arg(pItem->text());
    PDU* pdu = mkPDU(strPath.toStdString().size()+1);
    pdu->uiType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
    memcpy(pdu->caMsg, strPath.toStdString().c_str(), strPath.toStdString().size());
    //遍历当前路径下的所有文件，找到要删除文件的类型
    foreach(FileInfo* pFileInfo, m_pFileInfoList) {
        if (pItem->text() == pFileInfo->caName) {
            memcpy(pdu->caData, &pFileInfo->uiType, sizeof(uint));
            qDebug() << "pFileInfo->uiType" << pFileInfo->uiType;
        }
    }
    Client::getInstance().sendMsg(pdu);
}

void File::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    foreach(FileInfo* pFileInfo, m_pFileInfoList) {
        if (item->text() == pFileInfo->caName && pFileInfo->uiType != 0) {
            return;
        }
    }
    m_strCurPath = QString("%1/%2").arg(m_strCurPath).arg(item->text());
    flushFile();
}

void File::on_return_PB_clicked()
{
    if (m_strCurPath == m_strUserPath) {
        return;
    }
    int index = m_strCurPath.lastIndexOf('/');
    m_strCurPath.remove(index, m_strCurPath.size() - index);
    flushFile();
}

void File::on_upload_PB_clicked()
{
    m_strUploadPath = QFileDialog::getOpenFileName();
    qDebug() << m_strUploadPath;
    int index = m_strUploadPath.lastIndexOf('/');
    QString strFileName = m_strUploadPath.right(m_strUploadPath.size() - index - 1);
    QFile file(m_strUploadPath);
    qint64 iFileSize = file.size();
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST;
    memcpy(pdu->caData, strFileName.toStdString().c_str(), 32);
    memcpy(pdu->caData+32, &iFileSize, sizeof(qint64));
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::errorBox(QString strMsg)
{
    QMessageBox::information(this, "提示", strMsg);
}

void File::on_share_PB_clicked()
{
    //判断是否选择了文件
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if (!pItem) {
        return;
    }
    m_pShareFile->m_strFileName = pItem->text();
    m_pShareFile->updateLW();
    if (m_pShareFile->isHidden()) {
        m_pShareFile->show();
    }
}
