#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("Contact Manager");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Qt Demo");
    
    qDebug() << "Starting Contact Manager Application...";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
