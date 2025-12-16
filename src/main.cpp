#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QTimer>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("VSC-INSTALLER-PLUS");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("VSC-Installer-Plus");
    app.setOrganizationDomain("vscinstallerplus.local");
    
    app.setWindowIcon(QIcon(":/assets/icon.png"));
    
    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Gestor de instalación de tarballs para Linux");
    
    // Command line options
    QCommandLineOption localFileOption(QStringList() << "local-file", 
                                     "Archivo tarball local", "archivo");
    QCommandLineOption urlOption(QStringList() << "url", 
                               "URL de descarga", "url");
    QCommandLineOption installPathOption(QStringList() << "install-path", 
                                        "Ruta de instalación", "ruta", "/opt");
    QCommandLineOption createDesktopOption(QStringList() << "create-desktop", 
                                          "Crear entrada en el menú de aplicaciones");
    QCommandLineOption createSymlinkOption(QStringList() << "create-symlink", 
                                           "Crear enlace simbólico en /usr/local/bin");
    QCommandLineOption autoInstallOption(QStringList() << "auto-install", 
                                        "Iniciar instalación automáticamente");
    
    parser.addOption(localFileOption);
    parser.addOption(urlOption);
    parser.addOption(installPathOption);
    parser.addOption(createDesktopOption);
    parser.addOption(createSymlinkOption);
    parser.addOption(autoInstallOption);
    
    parser.process(app);
    
    MainWindow window;
    window.show();
    
    // If auto-install is requested, trigger installation after window is shown
    if (parser.isSet(autoInstallOption)) {
        QTimer::singleShot(100, [&window, &parser, &localFileOption, &urlOption, &installPathOption, &createDesktopOption, &createSymlinkOption]() {
            // Set the form values from command line arguments
            if (parser.isSet(localFileOption)) {
                window.setLocalFile(parser.value(localFileOption));
            } else if (parser.isSet(urlOption)) {
                window.setUrl(parser.value(urlOption));
            }
            
            window.setInstallPath(parser.value(installPathOption));
            window.setCreateDesktop(parser.isSet(createDesktopOption));
            window.setCreateSymlink(parser.isSet(createSymlinkOption));
            
            // Trigger installation
            window.startAutoInstall();
        });
    }
    
    return app.exec();
}
