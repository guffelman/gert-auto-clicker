#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <csignal>
#include <QDebug>
#include "mainwindow.h"

#ifdef QT_STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif

MainWindow* g_mainWindow = nullptr;

void signalHandler(int signal)
{
    qDebug() << "Received signal:" << signal;
    if (g_mainWindow) {
        g_mainWindow->close();
    }
    QApplication::quit();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Gert Auto Clicker");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Garrett Uffelman");
    
    // Set modern dark theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    
    // Set up signal handlers for graceful termination
    std::signal(SIGINT, signalHandler);   // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Termination request
#ifdef Q_OS_WIN
    std::signal(SIGBREAK, signalHandler); // Ctrl+Break on Windows
#endif
    
    // Create and show main window
    MainWindow window;
    g_mainWindow = &window;
    window.show();
    
    int result = app.exec();
    
    // Clean up global reference
    g_mainWindow = nullptr;
    
    return result;
} 