#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Initialize managers
    m_dbManager = new DatabaseManager(this);
    m_networkManager = new NetworkManager(this);
    
    // Setup signal/slot connections
    setupConnections();
    
    // Initial UI state
    updateButtonStates();
    
    showStatusMessage("Welcome! Please connect to database to begin.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    // Database connection signals
    connect(ui->pushButton_connect, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(m_dbManager, &DatabaseManager::databaseConnected,
            this, &MainWindow::onDatabaseConnected);
    connect(m_dbManager, &DatabaseManager::databaseDisconnected,
            this, &MainWindow::onDatabaseDisconnected);
    connect(m_dbManager, &DatabaseManager::errorOccurred,
            this, &MainWindow::onDatabaseError);
    
    // Contact management signals
    connect(ui->pushButton_add, &QPushButton::clicked,
            this, &MainWindow::onAddContactClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked,
            this, &MainWindow::onEditContactClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked,
            this, &MainWindow::onDeleteContactClicked);
    connect(ui->pushButton_refresh, &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);
    
    // Search functionality
    connect(ui->lineEdit_search, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    
    // Table selection
    connect(ui->tableWidget_contacts, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onTableSelectionChanged);
    
    // Network signals
    connect(ui->pushButton_fetch, &QPushButton::clicked,
            this, &MainWindow::onFetchFromApiClicked);
    connect(m_networkManager, &NetworkManager::contactFetched,
            this, &MainWindow::onContactFetched);
    connect(m_networkManager, &NetworkManager::fetchStarted,
            this, &MainWindow::onNetworkFetchStarted);
    connect(m_networkManager, &NetworkManager::fetchFinished,
            this, &MainWindow::onNetworkFetchFinished);
    connect(m_networkManager, &NetworkManager::errorOccurred,
            this, &MainWindow::onNetworkError);
    
    // Database CRUD signals for UI updates
    connect(m_dbManager, &DatabaseManager::contactAdded,
            this, [this](int) { loadContacts(); });
    connect(m_dbManager, &DatabaseManager::contactUpdated,
            this, [this](int) { loadContacts(); });
    connect(m_dbManager, &DatabaseManager::contactDeleted,
            this, [this](int) { loadContacts(); });
}

// ============= Database Connection Slots =============

void MainWindow::onConnectClicked()
{
    QString host = ui->lineEdit_host->text();
    QString database = ui->lineEdit_database->text();
    QString user = ui->lineEdit_user->text();
    QString password = ui->lineEdit_password->text();
    
    if (host.isEmpty() || database.isEmpty() || user.isEmpty()) {
        QMessageBox::warning(this, "Input Error", 
                           "Please fill in all required database connection fields.");
        return;
    }
    
    showStatusMessage("Connecting to database...");
    
    if (m_dbManager->connectToDatabase(host, database, user, password)) {

        if (!m_dbManager->createTable()) {
            QMessageBox::warning(this, "Database Error",
                               "Connected but failed to create table: " + 
                               m_dbManager->lastError());
        }
    } else {
        QMessageBox::critical(this, "Connection Error",
                            "Failed to connect to database:\n" + 
                            m_dbManager->lastError());
    }
}

void MainWindow::onDatabaseConnected()
{
    ui->label_status->setText("Connected");
    ui->label_status->setStyleSheet("color: green; font-weight: bold;");
    ui->pushButton_connect->setEnabled(false);
    
    // Disable connection input fields
    ui->lineEdit_host->setEnabled(false);
    ui->lineEdit_database->setEnabled(false);
    ui->lineEdit_user->setEnabled(false);
    ui->lineEdit_password->setEnabled(false);
    
    updateButtonStates();
    loadContacts();
    
    showStatusMessage("Successfully connected to database!");
}

void MainWindow::onDatabaseDisconnected()
{
    ui->label_status->setText("Disconnected");
    ui->label_status->setStyleSheet("color: red; font-weight: bold;");
    ui->pushButton_connect->setEnabled(true);
    
    updateButtonStates();
    showStatusMessage("Disconnected from database");
}

void MainWindow::onDatabaseError(const QString &error)
{
    showStatusMessage("Database error: " + error, 5000);
}

// ============= Contact Management Slots =============

void MainWindow::onAddContactClicked()
{
    Contact newContact = showContactDialog("Add New Contact");
    
    if (newContact.isValid()) {
        if (m_dbManager->addContact(newContact)) {
            showStatusMessage("Contact added successfully!");
        } else {
            QMessageBox::warning(this, "Error", 
                               "Failed to add contact: " + m_dbManager->lastError());
        }
    }
}

void MainWindow::onEditContactClicked()
{
    int currentRow = ui->tableWidget_contacts->currentRow();
    if (currentRow < 0) return;

    // Get contact ID from UserRole data
    QTableWidgetItem *item = ui->tableWidget_contacts->item(currentRow, 0);
    if (!item) {
        QMessageBox::warning(this, "Error", "Cannot retrieve contact information");
        return;
    }

    int contactId = item->data(Qt::UserRole).toInt();

    if (contactId <= 0) {
        QMessageBox::warning(this, "Error", "Invalid contact ID");
        return;
    }

    Contact contact = m_dbManager->getContact(contactId);

    if (!contact.isValid()) {
        QMessageBox::warning(this, "Error", "Failed to load contact details");
        return;
    }

    Contact updatedContact = showContactDialog("Edit Contact", contact);

    if (updatedContact.isValid()) {
        updatedContact.id = contactId;
        if (m_dbManager->updateContact(updatedContact)) {
            showStatusMessage("Contact updated successfully!");
        } else {
            QMessageBox::warning(this, "Error",
                                 "Failed to update contact: " + m_dbManager->lastError());
        }
    }
}

void MainWindow::onDeleteContactClicked()
{
    int currentRow = ui->tableWidget_contacts->currentRow();
    if (currentRow < 0) return;

    // Get contact ID from UserRole data
    QTableWidgetItem *item = ui->tableWidget_contacts->item(currentRow, 0);
    if (!item) {
        QMessageBox::warning(this, "Error", "Cannot retrieve contact information");
        return;
    }

    int contactId = item->data(Qt::UserRole).toInt();

    if (contactId <= 0) {
        QMessageBox::warning(this, "Error", "Invalid contact ID");
        return;
    }

    QString contactName = ui->tableWidget_contacts->item(currentRow, 0)->text() + " " +
                          ui->tableWidget_contacts->item(currentRow, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Deletion",
        "Are you sure you want to delete contact:\n" + contactName + "?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (m_dbManager->deleteContact(contactId)) {
            showStatusMessage("Contact deleted successfully!");
        } else {
            QMessageBox::warning(this, "Error",
                                 "Failed to delete contact: " + m_dbManager->lastError());
        }
    }
}

void MainWindow::onRefreshClicked()
{
    QString host = ui->lineEdit_host->text();
    QString database = ui->lineEdit_database->text();
    QString user = ui->lineEdit_user->text();
    QString password = ui->lineEdit_password->text();
    m_dbManager->disconnectFromDatabase();
    m_dbManager->connectToDatabase(host, database, user, password);   // or whatever method you use
    loadContacts();
    showStatusMessage("Contact list refreshed");
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    if (!m_dbManager->isConnected()) return;
    
    QVector<Contact> contacts = m_dbManager->searchContacts(text);
    displayContacts(contacts);
}

void MainWindow::onTableSelectionChanged()
{
    updateButtonStates();
}

// ============= Network Slots =============

void MainWindow::onFetchFromApiClicked()
{
    if (m_networkManager->isBusy()) {
        showStatusMessage("Network request already in progress...");
        return;
    }
    
    m_networkManager->fetchRandomContact();
}

void MainWindow::onContactFetched(const Contact &contact)
{
    // Show dialog to confirm adding the fetched contact
    QString message = QString("Fetched contact from API:\n\n"
                             "Name: %1 %2\n"
                             "Email: %3\n"
                             "Phone: %4\n"
                             "Location: %5, %6\n\n"
                             "Would you like to add this contact to the database?")
                        .arg(contact.firstName, contact.lastName, contact.email,
                             contact.phone, contact.city, contact.country);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Add Fetched Contact", message,
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (m_dbManager->addContact(contact)) {
            showStatusMessage("Fetched contact added to database!");
        } else {
            QMessageBox::warning(this, "Error",
                               "Failed to add contact: " + m_dbManager->lastError());
        }
    }
}

void MainWindow::onNetworkFetchStarted()
{
    ui->pushButton_fetch->setEnabled(false);
    showStatusMessage("Fetching contact from API...");
}

void MainWindow::onNetworkFetchFinished()
{
    ui->pushButton_fetch->setEnabled(true);
}

void MainWindow::onNetworkError(const QString &error)
{
    QMessageBox::warning(this, "Network Error", error);
    showStatusMessage("Network error occurred", 5000);
}

// ============= Helper Methods =============

void MainWindow::loadContacts()
{
    if (!m_dbManager->isConnected()) return;
    
    QVector<Contact> contacts = m_dbManager->getAllContacts();
    displayContacts(contacts);
}

void MainWindow::displayContacts(const QVector<Contact> &contacts)
{
    ui->tableWidget_contacts->setRowCount(0);

    for (const Contact &contact : contacts) {
        int row = ui->tableWidget_contacts->rowCount();
        ui->tableWidget_contacts->insertRow(row);

        // Create items for each column
        QTableWidgetItem *firstNameItem = new QTableWidgetItem(contact.firstName);
        QTableWidgetItem *lastNameItem = new QTableWidgetItem(contact.lastName);
        QTableWidgetItem *emailItem = new QTableWidgetItem(contact.email);
        QTableWidgetItem *phoneItem = new QTableWidgetItem(contact.phone);
        QTableWidgetItem *cityItem = new QTableWidgetItem(contact.city);
        QTableWidgetItem *countryItem = new QTableWidgetItem(contact.country);

        // Store ID in UserRole of EVERY item for safety
        firstNameItem->setData(Qt::UserRole, contact.id);
        lastNameItem->setData(Qt::UserRole, contact.id);
        emailItem->setData(Qt::UserRole, contact.id);
        phoneItem->setData(Qt::UserRole, contact.id);
        cityItem->setData(Qt::UserRole, contact.id);
        countryItem->setData(Qt::UserRole, contact.id);

        ui->tableWidget_contacts->setItem(row, 0, firstNameItem);
        ui->tableWidget_contacts->setItem(row, 1, lastNameItem);
        ui->tableWidget_contacts->setItem(row, 2, emailItem);
        ui->tableWidget_contacts->setItem(row, 3, phoneItem);
        ui->tableWidget_contacts->setItem(row, 4, cityItem);
        ui->tableWidget_contacts->setItem(row, 5, countryItem);
    }

    ui->tableWidget_contacts->resizeColumnsToContents();
}

void MainWindow::updateButtonStates()
{
    bool connected = m_dbManager->isConnected();
    bool hasSelection = ui->tableWidget_contacts->currentRow() >= 0;
    
    ui->pushButton_add->setEnabled(connected);
    ui->pushButton_edit->setEnabled(connected && hasSelection);
    ui->pushButton_delete->setEnabled(connected && hasSelection);
    ui->pushButton_refresh->setEnabled(connected);
}

void MainWindow::showStatusMessage(const QString &message, int timeout)
{
    ui->statusbar->showMessage(message, timeout);
}

Contact MainWindow::showContactDialog(const QString &title, const Contact &contact)
{
    ContactDialog dialog(title, contact, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getContact();
    }
    
    return Contact(); // Return empty contact if cancelled
}

// ============= ContactDialog Implementation =============

ContactDialog::ContactDialog(const QString &title, const Contact &contact, QWidget *parent)
    : QDialog(parent), m_contact(contact)
{
    setWindowTitle(title);
    setModal(true);
    
    QFormLayout *formLayout = new QFormLayout();
    
    m_firstNameEdit = new QLineEdit(contact.firstName, this);
    m_lastNameEdit = new QLineEdit(contact.lastName, this);
    m_emailEdit = new QLineEdit(contact.email, this);
    m_phoneEdit = new QLineEdit(contact.phone, this);
    m_cityEdit = new QLineEdit(contact.city, this);
    m_countryEdit = new QLineEdit(contact.country, this);
    
    formLayout->addRow("First Name *:", m_firstNameEdit);
    formLayout->addRow("Last Name *:", m_lastNameEdit);
    formLayout->addRow("Email:", m_emailEdit);
    formLayout->addRow("Phone:", m_phoneEdit);
    formLayout->addRow("City:", m_cityEdit);
    formLayout->addRow("Country:", m_countryEdit);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
    
    // Set minimum width for better appearance
    setMinimumWidth(400);
}

Contact ContactDialog::getContact() const
{
    Contact contact;
    contact.firstName = m_firstNameEdit->text().trimmed();
    contact.lastName = m_lastNameEdit->text().trimmed();
    contact.email = m_emailEdit->text().trimmed();
    contact.phone = m_phoneEdit->text().trimmed();
    contact.city = m_cityEdit->text().trimmed();
    contact.country = m_countryEdit->text().trimmed();
    
    return contact;
}
