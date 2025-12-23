#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVector>
#include "contact.h"


class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Database connection
    bool connectToDatabase(const QString &host, const QString &database,
                          const QString &user, const QString &password,
                          int port = 3306);
    void disconnectFromDatabase();
    bool isConnected() const;
    QString lastError() const { return m_lastError; }

    // CRUD Operations
    bool createTable();
    bool addContact(const Contact &contact);
    bool updateContact(const Contact &contact);
    bool deleteContact(int id);
    Contact getContact(int id);
    QVector<Contact> getAllContacts();
    QVector<Contact> searchContacts(const QString &searchTerm);

signals:
    void databaseConnected();
    void databaseDisconnected();
    void contactAdded(int id);
    void contactUpdated(int id);
    void contactDeleted(int id);
    void errorOccurred(const QString &error);

private:
    QSqlDatabase m_database;
    QString m_lastError;
    
    void setLastError(const QString &error);
};

#endif // DATABASEMANAGER_H
