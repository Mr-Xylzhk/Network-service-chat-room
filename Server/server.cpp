#include "mytcpserver.h"
#include "server.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>

Server::Server(QWidget *parent)
    : QWidget(parent)
{
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
}

Server::~Server()
{
}

void Server::loadConfig()
{
    QFile file(":/connect.config");    //配置文件固定冒号开头
    if (file.open(QIODevice::ReadOnly)) {
        QString strData = QString(file.readAll());
        QStringList strList = strData.split("\r\n");
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        m_strRootPath = strList[2];
        qDebug() << "loadConfig strIP" << m_strIP << "usPort" << m_usPort << "strPath" << m_strRootPath;
        file.close();
    } else {
        QMessageBox::critical(this, "提示", "打开文件失败");
        qDebug() << "loadConfig 打开文件失败";
    }
}

Server &Server::getInstance()
{
    static Server instance;
    return instance;
}

