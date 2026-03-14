#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase>

class OperateDB : public QObject
{
    Q_OBJECT
public:

    QSqlDatabase m_db;
    ~OperateDB();
    void connect();
    static OperateDB& getInstance();
    bool handleRegist(char* caName, char* caPwd);
    bool handleLogin(char* caName, char* caPwd);
    void handleOffline(const char* caName);
    int handleFindUser(const char* caName);
    QStringList handleOnlineUser();
    int handleAddFriend(char* caCurName, char* caTarName);
    bool handleAddFriendAgree(char* caCurName, char* caTarName);
    QStringList handleFlushFriend(char* caName);
private:
    explicit OperateDB(QObject *parent = nullptr);
    OperateDB(const OperateDB& instance) = delete;
    OperateDB& operator=(const OperateDB&) = delete;

signals:

};

#endif // OPERATEDB_H
