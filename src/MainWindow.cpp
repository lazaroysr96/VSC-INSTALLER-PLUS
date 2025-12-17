#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QInputDialog>
#include <QListWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_installer(new Installer(this))
    , m_launcherCreator(new LauncherCreator(this))
    , m_tabWidget(new QTabWidget(this))
{
    ui->setupUi(this);
    
    // Create tab widget and add installer interface
    m_tabWidget->addTab(ui->centralwidget, "Instalador de Aplicaciones");
    m_tabWidget->addTab(m_launcherCreator, "Generador de Lanzadores");
    
    setCentralWidget(m_tabWidget);
    
    setupConnections();
    
    m_installer->setProgressBar(ui->progressBar);
    m_installer->setLogTextEdit(ui->logTextEdit);
    
    resetForm();
    
    setWindowTitle("VSC-INSTALLER-PLUS v1.0.0");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseButtonClicked);
    connect(ui->browseInstallButton, &QPushButton::clicked, this, &MainWindow::onBrowseInstallButtonClicked);
    connect(ui->installButton, &QPushButton::clicked, this, &MainWindow::onInstallButtonClicked);
    connect(ui->updateButton, &QPushButton::clicked, this, &MainWindow::onUpdateButtonClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearButtonClicked);
    
    connect(ui->localFileRadio, &QRadioButton::toggled, this, &MainWindow::onLocalFileRadioToggled);
    connect(ui->urlRadio, &QRadioButton::toggled, this, &MainWindow::onUrlRadioToggled);
    
    connect(ui->actionSalir, &QAction::triggered, this, &MainWindow::onActionSalirTriggered);
    connect(ui->actionVer_instalados, &QAction::triggered, this, &MainWindow::onActionVerInstaladosTriggered);
    connect(ui->actionLimpiar_registros, &QAction::triggered, this, &MainWindow::onActionLimpiarRegistrosTriggered);
    
    connect(m_installer, &Installer::installationCompleted, this, &MainWindow::onInstallationCompleted);
    connect(m_installer, &Installer::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_installer, &Installer::logMessage, this, &MainWindow::onLogMessage);
    connect(m_installer, &Installer::adminPrivilegesRequired, this, &MainWindow::onAdminPrivilegesRequired);
}

void MainWindow::onBrowseButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Seleccionar archivo tarball",
        QDir::homePath(),
        "Archivos comprimidos (*.tar.gz *.tgz *.tar.bz2 *.tbz2 *.tar.xz *.tar);;Todos los archivos (*)"
    );
    
    if (!fileName.isEmpty()) {
        ui->localFileLineEdit->setText(fileName);
    }
}

void MainWindow::onBrowseInstallButtonClicked()
{
    QString dirName = QFileDialog::getExistingDirectory(
        this,
        "Seleccionar directorio de instalación",
        ui->installPathLineEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!dirName.isEmpty()) {
        ui->installPathLineEdit->setText(dirName);
    }
}

void MainWindow::onInstallButtonClicked()
{
    QString source = getSelectedSource();
    if (source.isEmpty()) {
        QMessageBox::warning(this, "Error", "Por favor seleccione un archivo o URL válida");
        return;
    }
    
    QString installPath = getInstallPath();
    if (installPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Por favor especifique una ruta de instalación");
        return;
    }
    
    enableControls(false);
    ui->progressBar->setValue(0);
    ui->logTextEdit->clear();
    
    bool createDesktop = shouldCreateDesktop();
    bool createSymlink = shouldCreateSymlink();
    
    if (ui->localFileRadio->isChecked()) {
        m_installer->installFromLocalFile(source, installPath, createDesktop, createSymlink);
    } else {
        m_installer->installFromUrl(QUrl(source), installPath, createDesktop, createSymlink);
    }
}

void MainWindow::onUpdateButtonClicked()
{
    QStringList installedApps = m_installer->getInstalledApps();
    
    if (installedApps.isEmpty()) {
        QMessageBox::information(this, "Información", "No hay aplicaciones instaladas para actualizar");
        return;
    }
    
    bool ok;
    QString appName = QInputDialog::getItem(
        this,
        "Actualizar Aplicación",
        "Seleccione la aplicación a actualizar:",
        installedApps,
        0,
        false,
        &ok
    );
    
    if (!ok || appName.isEmpty()) {
        return;
    }
    
    bool ok2;
    QString newSource = QInputDialog::getText(
        this,
        "Nueva Fuente",
        "Ingrese la URL o ruta del nuevo paquete:",
        QLineEdit::Normal,
        "",
        &ok2
    );
    
    if (!ok2 || newSource.isEmpty()) {
        return;
    }
    
    bool isUrl = newSource.startsWith("http://") || newSource.startsWith("https://");
    
    enableControls(false);
    ui->progressBar->setValue(0);
    ui->logTextEdit->clear();
    
    m_installer->updateExistingApp(appName, newSource, "", isUrl);
}

void MainWindow::onClearButtonClicked()
{
    resetForm();
}

void MainWindow::onLocalFileRadioToggled(bool checked)
{
    ui->localFileLineEdit->setEnabled(checked);
    ui->browseButton->setEnabled(checked);
    ui->urlLineEdit->setEnabled(!checked);
}

void MainWindow::onUrlRadioToggled(bool checked)
{
    ui->urlLineEdit->setEnabled(checked);
    ui->localFileLineEdit->setEnabled(!checked);
    ui->browseButton->setEnabled(!checked);
}

void MainWindow::onActionSalirTriggered()
{
    close();
}

void MainWindow::onActionVerInstaladosTriggered()
{
    QStringList installedApps = m_installer->getInstalledApps();
    
    QDialog dialog(this);
    dialog.setWindowTitle("Aplicaciones Instaladas");
    dialog.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *label = new QLabel("Aplicaciones instaladas:");
    layout->addWidget(label);
    
    QListWidget *listWidget = new QListWidget();
    listWidget->addItems(installedApps);
    layout->addWidget(listWidget);
    
    QPushButton *closeButton = new QPushButton("Cerrar");
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    dialog.exec();
}

void MainWindow::onActionLimpiarRegistrosTriggered()
{
    int ret = QMessageBox::warning(
        this,
        "Confirmar Eliminación",
        "¿Está seguro de que desea eliminar todos los registros de aplicaciones instaladas?\n\n"
        "Esta acción no eliminará los archivos de las aplicaciones, solo los registros en la base de datos.",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        // This would require adding a method to Installer to clear all records
        QMessageBox::information(this, "Información", "Función de limpieza de registros pendiente de implementar");
    }
}

void MainWindow::onInstallationCompleted(bool success, const QString &message)
{
    enableControls(true);
    
    if (success) {
        QMessageBox::information(this, "Éxito", message);
        resetForm();
    } else {
        QMessageBox::critical(this, "Error", message);
    }
}

void MainWindow::onProgressUpdated(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::onLogMessage(const QString &message)
{
    ui->logTextEdit->append(message);
}

void MainWindow::onAdminPrivilegesRequired()
{
    enableControls(true);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Privilegios de Administrador Requeridos",
        "Esta instalación requiere privilegios de administrador para completarse.\n\n"
        "¿Desea reiniciar la aplicación con privilegios de administrador?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );
    
    if (reply == QMessageBox::Yes) {
        // Prepare arguments for restart
        QStringList args;
        
        // Save current state to arguments
        if (ui->localFileRadio->isChecked()) {
            args << "--local-file" << ui->localFileLineEdit->text();
        } else {
            args << "--url" << ui->urlLineEdit->text();
        }
        
        args << "--install-path" << ui->installPathLineEdit->text();
        
        if (ui->createDesktopCheckBox->isChecked()) {
            args << "--create-desktop";
        }
        
        if (ui->createSymlinkCheckBox->isChecked()) {
            args << "--create-symlink";
        }
        
        args << "--auto-install";
        
        // Restart with admin privileges
        m_installer->restartWithAdminPrivileges(args);
        
        // Close current window
        close();
    } else {
        ui->logTextEdit->append("Instalación cancelada por el usuario");
    }
}

void MainWindow::setLocalFile(const QString &filePath)
{
    ui->localFileRadio->setChecked(true);
    ui->localFileLineEdit->setText(filePath);
}

void MainWindow::setUrl(const QString &url)
{
    ui->urlRadio->setChecked(true);
    ui->urlLineEdit->setText(url);
}

void MainWindow::setInstallPath(const QString &path)
{
    ui->installPathLineEdit->setText(path);
}

void MainWindow::setCreateDesktop(bool create)
{
    ui->createDesktopCheckBox->setChecked(create);
}

void MainWindow::setCreateSymlink(bool create)
{
    ui->createSymlinkCheckBox->setChecked(create);
}

void MainWindow::startAutoInstall()
{
    // Simulate clicking the install button
    onInstallButtonClicked();
}

void MainWindow::resetForm()
{
    ui->localFileLineEdit->clear();
    ui->urlLineEdit->clear();
    ui->installPathLineEdit->setText("/opt");
    ui->progressBar->setValue(0);
    ui->logTextEdit->clear();
    ui->localFileRadio->setChecked(true);
    ui->createDesktopCheckBox->setChecked(true);
    ui->createSymlinkCheckBox->setChecked(true);
}

void MainWindow::enableControls(bool enabled)
{
    ui->sourceGroupBox->setEnabled(enabled);
    ui->installGroupBox->setEnabled(enabled);
    ui->installButton->setEnabled(enabled);
    ui->updateButton->setEnabled(enabled);
    ui->clearButton->setEnabled(enabled);
}

QString MainWindow::getSelectedSource() const
{
    if (ui->localFileRadio->isChecked()) {
        return ui->localFileLineEdit->text();
    } else {
        return ui->urlLineEdit->text();
    }
}

QString MainWindow::getInstallPath() const
{
    return ui->installPathLineEdit->text();
}

bool MainWindow::shouldCreateDesktop() const
{
    return ui->createDesktopCheckBox->isChecked();
}

bool MainWindow::shouldCreateSymlink() const
{
    return ui->createSymlinkCheckBox->isChecked();
}
