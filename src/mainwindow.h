#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "databasemanager.h"
#include "networkmanager.h"
#include "contact.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Main application window
 * 
 * Demonstrates Qt signals/slots architecture for UI interaction
 * with database and network operations
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Database connection slots
    void onConnectClicked();
    void onDatabaseConnected();
    void onDatabaseDisconnected();
    void onDatabaseError(const QString &error);

    // Contact management slots
    void onAddContactClicked();
    void onEditContactClicked();
    void onDeleteContactClicked();
    void onRefreshClicked();
    void onSearchTextChanged(const QString &text);
    void onTableSelectionChanged();

    // Network slots
    void onFetchFromApiClicked();
    void onContactFetched(const Contact &contact);
    void onNetworkFetchStarted();
    void onNetworkFetchFinished();
    void onNetworkError(const QString &error);

private:
    Ui::MainWindow *ui;
    DatabaseManager *m_dbManager;
    NetworkManager *m_networkManager;
    
    void setupConnections();
    void loadContacts();
    void displayContacts(const QVector<Contact> &contacts);
    void updateButtonStates();
    void showStatusMessage(const QString &message, int timeout = 3000);
    
    // Dialog helpers
    Contact showContactDialog(const QString &title, const Contact &contact = Contact());
};

/**
 * @brief Simple dialog for adding/editing contacts
 */
class ContactDialog : public QDialog
{
    Q_OBJECT

public:
    ContactDialog(const QString &title, const Contact &contact, QWidget *parent = nullptr);
    Contact getContact() const;

private:
    QLineEdit *m_firstNameEdit;
    QLineEdit *m_lastNameEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_cityEdit;
    QLineEdit *m_countryEdit;
    Contact m_contact;
};

#endif // MAINWINDOW_H
