#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "contact.h"

/**
 * @brief Manages network operations for fetching contact data
 * 
 * This class demonstrates Qt's network capabilities by fetching
 * random user data from the RandomUser.me API
 */
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // Fetch random contact from API
    void fetchRandomContact();
    bool isBusy() const { return m_busy; }

signals:
    void contactFetched(const Contact &contact);
    void fetchStarted();
    void fetchFinished();
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    bool m_busy;
    
    Contact parseJsonResponse(const QByteArray &data);
};

#endif // NETWORKMANAGER_H
