#ifndef FILE_H
#define FILE_H

#include "protocol.h"
#include "sharefile.h"

#include <QListWidget>
#include <QWidget>

namespace Ui {
class File;
}

class File : public QWidget
{
    Q_OBJECT

public:
    QString m_strUserPath;
    QString m_strCurPath;
    QString m_strUploadPath;
    ShareFile* m_pShareFile;
    QList<FileInfo*> m_pFileInfoList;
    explicit File(QWidget *parent = nullptr);
    ~File();
    void updateFileList(QList<FileInfo*> pFileList);
    void flushFile();
    void uploadFile();

private slots:
    void on_mkDir_PB_clicked();

    void on_flush_PB_clicked();

    void on_delFile_PB_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_return_PB_clicked();

    void on_upload_PB_clicked();

    void errorBox(QString strMsg);

    void on_share_PB_clicked();

private:
    Ui::File *ui;
};

#endif // FILE_H
