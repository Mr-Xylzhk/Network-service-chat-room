#ifndef CLIENT_H
#define CLIENT_H

#include "protocol.h"
#include "reshandler.h"

#include <QTcpSocket>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

public:
    QString m_strIP;
    quint16 m_usPort;
    QString m_strRootPath;
    QString m_strLoginName;
    ResHandler* m_prh;
    QByteArray buffer;
    QTcpSocket socket;
    PDU* readMsg();
    void handleMsg(PDU* pdu);
    ~Client();
    void loadConfig();
    static Client& getInstance(); 

public slots:
    void showConnect();
    void recvMsg();
    void sendMsg(PDU* pdu);

private slots:

    void on_regist_PB_clicked();

    void on_login_PB_clicked();

private:
    Ui::Client *ui;
    Client(QWidget *parent = nullptr);
    Client(const Client& instance) = delete;
    Client& operator=(const Client&) = delete;
};
#endif // CLIENT_H
