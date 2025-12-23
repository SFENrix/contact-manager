#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>


DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();
    m_database = QSqlDatabase::addDatabase("QSQLITE");
}
DatabaseManager::~DatabaseManager()
{
    disconnectFromDatabase();
}
bool DatabaseManager::connectToDatabase(const QString &host, const QString &database,
                                        const QString &user, const QString &password, int port)
{
    // For SQLite, we only need the database name (file path)
    // The other parameters are ignored
    m_database.setDatabaseName("contacts.db");

    if (!m_database.open()) {
        setLastError("Failed to connect: " + m_database.lastError().text());
        return false;
    }

    qDebug() << "Successfully connected to SQLite database: contacts.db";
    emit databaseConnected();
    return true;
}

void DatabaseManager::disconnectFromDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        emit databaseDisconnected();
        qDebug() << "Database disconnected";
    }
}

bool DatabaseManager::isConnected() const
{
    return m_database.isOpen();
}

bool DatabaseManager::createTable()
{
    if (!isConnected()) {
        setLastError("Database not connected");
        return false;
    }

    QSqlQuery query(m_database);
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS contacts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            email TEXT,
            phone TEXT,
            city TEXT,
            country TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createTableSQL)) {
        setLastError("Failed to create table: " + query.lastError().text());
        return false;
    }

    qDebug() << "Table 'contacts' created or already exists";
    return true;
}
bool DatabaseManager::addContact(const Contact &contact)
{
    if (!isConnected()) {
        setLastError("Database not connected");
        return false;
    }

    if (!contact.isValid()) {
        setLastError("Invalid contact data");
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO contacts (first_name, last_name, email, phone, city, country) "
                 "VALUES (:firstName, :lastName, :email, :phone, :city, :country)");
    
    query.bindValue(":firstName", contact.firstName);
    query.bindValue(":lastName", contact.lastName);
    query.bindValue(":email", contact.email);
    query.bindValue(":phone", contact.phone);
    query.bindValue(":city", contact.city);
    query.bindValue(":country", contact.country);

    if (!query.exec()) {
        setLastError("Failed to add contact: " + query.lastError().text());
        return false;
    }

    int newId = query.lastInsertId().toInt();
    emit contactAdded(newId);
    qDebug() << "Contact added with ID:" << newId;
    return true;
}

bool DatabaseManager::updateContact(const Contact &contact)
{
    if (!isConnected()) {
        setLastError("Database not connected");
        return false;
    }

    if (contact.id <= 0 || !contact.isValid()) {
        setLastError("Invalid contact data");
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE contacts SET first_name=:firstName, last_name=:lastName, "
                 "email=:email, phone=:phone, city=:city, country=:country "
                 "WHERE id=:id");
    
    query.bindValue(":id", contact.id);
    query.bindValue(":firstName", contact.firstName);
    query.bindValue(":lastName", contact.lastName);
    query.bindValue(":email", contact.email);
    query.bindValue(":phone", contact.phone);
    query.bindValue(":city", contact.city);
    query.bindValue(":country", contact.country);

    if (!query.exec()) {
        setLastError("Failed to update contact: " + query.lastError().text());
        return false;
    }

    emit contactUpdated(contact.id);
    qDebug() << "Contact updated, ID:" << contact.id;
    return true;
}

bool DatabaseManager::deleteContact(int id)
{
    if (!isConnected()) {
        setLastError("Database not connected");
        return false;
    }

    if (id <= 0) {
        setLastError("Invalid contact ID");
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM contacts WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        setLastError("Failed to delete contact: " + query.lastError().text());
        return false;
    }

    emit contactDeleted(id);
    qDebug() << "Contact deleted, ID:" << id;
    return true;
}

Contact DatabaseManager::getContact(int id)
{
    Contact contact;
    
    if (!isConnected()) {
        setLastError("Database not connected");
        return contact;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contacts WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        setLastError("Contact not found");
        return contact;
    }

    contact.id = query.value("id").toInt();
    contact.firstName = query.value("first_name").toString();
    contact.lastName = query.value("last_name").toString();
    contact.email = query.value("email").toString();
    contact.phone = query.value("phone").toString();
    contact.city = query.value("city").toString();
    contact.country = query.value("country").toString();

    return contact;
}

QVector<Contact> DatabaseManager::getAllContacts()
{
    QVector<Contact> contacts;
    
    if (!isConnected()) {
        setLastError("Database not connected");
        return contacts;
    }

    QSqlQuery query(m_database);
    if (!query.exec("SELECT * FROM contacts ORDER BY first_name, last_name")) {
        setLastError("Failed to fetch contacts: " + query.lastError().text());
        return contacts;
    }

    while (query.next()) {
        Contact contact;
        contact.id = query.value("id").toInt();
        contact.firstName = query.value("first_name").toString();
        contact.lastName = query.value("last_name").toString();
        contact.email = query.value("email").toString();
        contact.phone = query.value("phone").toString();
        contact.city = query.value("city").toString();
        contact.country = query.value("country").toString();
        contacts.append(contact);
    }

    return contacts;
}

QVector<Contact> DatabaseManager::searchContacts(const QString &searchTerm)
{
    QVector<Contact> contacts;
    
    if (!isConnected()) {
        setLastError("Database not connected");
        return contacts;
    }

    if (searchTerm.isEmpty()) {
        return getAllContacts();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contacts WHERE "
                 "first_name LIKE :term OR last_name LIKE :term OR "
                 "email LIKE :term OR phone LIKE :term OR "
                 "city LIKE :term OR country LIKE :term "
                 "ORDER BY first_name, last_name");
    
    QString likePattern = "%" + searchTerm + "%";
    query.bindValue(":term", likePattern);

    if (!query.exec()) {
        setLastError("Failed to search contacts: " + query.lastError().text());
        return contacts;
    }

    while (query.next()) {
        Contact contact;
        contact.id = query.value("id").toInt();
        contact.firstName = query.value("first_name").toString();
        contact.lastName = query.value("last_name").toString();
        contact.email = query.value("email").toString();
        contact.phone = query.value("phone").toString();
        contact.city = query.value("city").toString();
        contact.country = query.value("country").toString();
        contacts.append(contact);
    }

    return contacts;
}

void DatabaseManager::setLastError(const QString &error)
{
    m_lastError = error;
    emit errorOccurred(error);
    qWarning() << "DatabaseManager Error:" << error;
}
