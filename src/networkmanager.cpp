#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), m_busy(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &NetworkManager::onReplyFinished);
}

NetworkManager::~NetworkManager()
{
    // QNetworkAccessManager is deleted automatically as it's a child object
}

void NetworkManager::fetchRandomContact()
{
    if (m_busy) {
        qWarning() << "Network request already in progress";
        return;
    }

    m_busy = true;
    emit fetchStarted();

    // Using RandomUser.me API - a free API for generating random user data
    QUrl url("https://randomuser.me/api/?results=1");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    qDebug() << "Fetching random contact from API...";
    m_networkManager->get(request);
}

void NetworkManager::onReplyFinished(QNetworkReply *reply)
{
    m_busy = false;
    emit fetchFinished();

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = "Network error: " + reply->errorString();
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }

    // Read response data
    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    // Parse JSON and extract contact
    Contact contact = parseJsonResponse(responseData);
    
    if (contact.isValid()) {
        qDebug() << "Successfully fetched contact:" << contact.fullName();
        emit contactFetched(contact);
    } else {
        QString errorMsg = "Failed to parse contact data from API response";
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
    }
}

Contact NetworkManager::parseJsonResponse(const QByteArray &data)
{
    Contact contact;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON response";
        return contact;
    }

    QJsonObject root = doc.object();
    
    // Check if results array exists
    if (!root.contains("results") || !root["results"].isArray()) {
        qWarning() << "No results array in response";
        return contact;
    }

    QJsonArray results = root["results"].toArray();
    if (results.isEmpty()) {
        qWarning() << "Empty results array";
        return contact;
    }

    // Get first result
    QJsonObject user = results[0].toObject();

    // Extract name
    if (user.contains("name") && user["name"].isObject()) {
        QJsonObject name = user["name"].toObject();
        contact.firstName = name["first"].toString();
        contact.lastName = name["last"].toString();
    }

    // Extract email
    if (user.contains("email")) {
        contact.email = user["email"].toString();
    }

    // Extract phone
    if (user.contains("phone")) {
        contact.phone = user["phone"].toString();
    }

    // Extract location
    if (user.contains("location") && user["location"].isObject()) {
        QJsonObject location = user["location"].toObject();
        
        if (location.contains("city")) {
            contact.city = location["city"].toString();
        }
        
        if (location.contains("country")) {
            contact.country = location["country"].toString();
        }
    }

    return contact;
}
