#include "Installer.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileInfo>
#include <QCoreApplication>
#include <unistd.h>

Installer::Installer(QObject *parent)
    : QObject(parent)
    , m_progressBar(nullptr)
    , m_logTextEdit(nullptr)
{
    initializeDatabase();
}

Installer::~Installer()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void Installer::setProgressBar(QProgressBar *bar)
{
    m_progressBar = bar;
}

void Installer::setLogTextEdit(QTextEdit *textEdit)
{
    m_logTextEdit = textEdit;
}

bool Installer::installFromLocalFile(const QString &filePath, const QString &installPath,
                                     bool createDesktop, bool createSymlink)
{
    log("Iniciando instalación desde archivo local: " + filePath);
    
    // Check dependencies first
    if (!checkDependencies()) {
        log("ERROR: Dependencias del sistema no cumplidas");
        emit installationCompleted(false, "Dependencias del sistema no cumplidas. Por favor instale 'tar' y otras herramientas necesarias.");
        return false;
    }
    
    // Check if admin privileges are needed
    if (needsAdminPrivileges(installPath, createSymlink)) {
        if (!checkAdminPrivileges()) {
            log("Se requieren privilegios de administrador para esta instalación");
            emit adminPrivilegesRequired();
            return false;
        }
    }
    
    if (!QFile::exists(filePath)) {
        log("ERROR: El archivo no existe: " + filePath);
        emit installationCompleted(false, "El archivo no existe");
        return false;
    }

    // Extract to a temporary directory first
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/vsc_installer_temp_" + timestamp + "_" + QString::number(QCoreApplication::applicationPid());
    
    log("Creando directorio temporal: " + tempDir);
    
    // Ensure the directory exists and is writable
    if (!QDir().mkpath(tempDir)) {
        log("ERROR: No se pudo crear el directorio temporal: " + tempDir);
        emit installationCompleted(false, "No se pudo crear directorio temporal");
        return false;
    }
    
    // Verify directory is writable
    QFileInfo tempDirInfo(tempDir);
    if (!tempDirInfo.exists() || !tempDirInfo.isWritable()) {
        log("ERROR: El directorio temporal no es escribible: " + tempDir);
        QDir(tempDir).removeRecursively();
        emit installationCompleted(false, "Directorio temporal no es escribible");
        return false;
    }
    
    log("Extrayendo temporalmente a: " + tempDir);
    updateProgress(20);

    if (!extractTarball(filePath, tempDir)) {
        log("ERROR: Falló la extracción del tarball");
        emit installationCompleted(false, "Falló la extracción del tarball");
        QDir(tempDir).removeRecursively();
        return false;
    }

    updateProgress(40);

    // Find the actual application directory and executable
    QString execPath = findExecutableInDirectory(tempDir);
    if (execPath.isEmpty()) {
        log("ERROR: No se encontró ejecutable en el directorio extraído");
        emit installationCompleted(false, "No se encontró ejecutable");
        QDir(tempDir).removeRecursively();
        return false;
    }

    // Determine the real application directory (parent of executable)
    QFileInfo execInfo(execPath);
    QString realAppDir = execInfo.absolutePath();
    QString appName = getAppNameFromPath(execPath);
    
    log("Ejecutable encontrado: " + execPath);
    log("Directorio real de la aplicación: " + realAppDir);
    log("Nombre de la aplicación: " + appName);

    // Final installation directory
    QString finalInstallDir = installPath + "/" + appName;
    
    // Move the application directory to final location
    log("Moviendo aplicación a: " + finalInstallDir);
    QDir().mkpath(installPath);
    
    // Remove existing installation if it exists
    if (QDir(finalInstallDir).exists()) {
        log("Eliminando instalación previa en: " + finalInstallDir);
        QDir(finalInstallDir).removeRecursively();
    }
    
    // Rename/move the actual application directory
    log("Copiando aplicación de " + realAppDir + " a " + finalInstallDir);
    
    // Try rename first (works on same filesystem)
    if (!QDir().rename(realAppDir, finalInstallDir)) {
        log("Rename falló, intentando copia recursiva...");
        
        // If rename fails, use recursive copy (works across filesystems)
        if (!copyDirectoryRecursively(realAppDir, finalInstallDir)) {
            log("ERROR: No se pudo copiar la aplicación al destino final");
            emit installationCompleted(false, "No se pudo copiar la aplicación al destino final");
            QDir(tempDir).removeRecursively();
            return false;
        }
        
        // Remove source directory after successful copy
        log("Eliminando directorio temporal: " + realAppDir);
        QDir(realAppDir).removeRecursively();
    } else {
        log("Aplicación movida exitosamente con rename");
    }

    // Update executable path to final location
    QString finalExecPath = finalInstallDir + "/" + execInfo.fileName();
    log("Ruta final del ejecutable: " + finalExecPath);

    updateProgress(60);

    if (createSymlink) {
        QString symlinkName = "/usr/local/bin/" + appName;
        if (this->createSymlink(finalExecPath, symlinkName)) {
            log("Enlace simbólico creado: " + symlinkName);
        } else {
            log("ADVERTENCIA: No se pudo crear el enlace simbólico");
        }
    }

    updateProgress(80);

    if (createDesktop) {
        QString iconPath = finalInstallDir + "/icon.png";
        if (!QFile::exists(iconPath)) {
            iconPath = "";
        }
        
        if (createDesktopEntry(appName, finalExecPath, iconPath)) {
            log("Entrada de escritorio creada correctamente");
        } else {
            log("ADVERTENCIA: No se pudo crear la entrada de escritorio");
        }
    }

    updateProgress(90);

    QString version = getVersionFromExecutable(finalExecPath);
    if (registerApp(appName, version, finalInstallDir, "", finalExecPath)) {
        log("Aplicación registrada en la base de datos");
    } else {
        log("ADVERTENCIA: No se pudo registrar la aplicación");
    }

    // Clean up temporary directory
    QDir(tempDir).removeRecursively();

    updateProgress(100);
    log("Instalación completada exitosamente");
    emit installationCompleted(true, "Instalación completada exitosamente");
    
    return true;
}

bool Installer::installFromUrl(const QUrl &url, const QString &installPath,
                              bool createDesktop, bool createSymlink)
{
    log("Iniciando instalación desde URL: " + url.toString());
    
    // Check if admin privileges are needed
    if (needsAdminPrivileges(installPath, createSymlink)) {
        if (!checkAdminPrivileges()) {
            log("Se requieren privilegios de administrador para esta instalación");
            emit adminPrivilegesRequired();
            return false;
        }
    }
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileName = url.fileName();
    if (fileName.isEmpty()) {
        fileName = "download.tar.gz";
    }
    
    QString downloadPath = tempDir + "/" + fileName;
    m_currentDownloadPath = downloadPath;

    log("Descargando archivo a: " + downloadPath);
    
    if (!downloadFile(url, downloadPath)) {
        log("ERROR: Falló la descarga del archivo");
        emit installationCompleted(false, "Falló la descarga del archivo");
        return false;
    }

    log("Descarga completada, iniciando instalación");
    
    bool result = installFromLocalFile(downloadPath, installPath, createDesktop, createSymlink);
    
    QFile::remove(downloadPath);
    
    return result;
}

bool Installer::updateExistingApp(const QString &appName, const QString &newSource,
                                  const QString &installPath, bool isUrl)
{
    log("Iniciando actualización de aplicación: " + appName);
    
    QSqlQuery query(m_db);
    query.prepare("SELECT install_path FROM installed_apps WHERE app_name = ?");
    query.addBindValue(appName);
    
    if (!query.exec() || !query.next()) {
        log("ERROR: Aplicación no encontrada en los registros");
        emit installationCompleted(false, "Aplicación no encontrada");
        return false;
    }
    
    QString currentInstallPath = query.value(0).toString();
    
    bool result;
    if (isUrl) {
        result = installFromUrl(QUrl(newSource), currentInstallPath, true, true);
    } else {
        result = installFromLocalFile(newSource, currentInstallPath, true, true);
    }
    
    if (result) {
        log("Actualización completada exitosamente");
    }
    
    return result;
}

QStringList Installer::getInstalledApps() const
{
    QStringList apps;
    QSqlQuery query("SELECT app_name FROM installed_apps", m_db);
    
    while (query.next()) {
        apps << query.value(0).toString();
    }
    
    return apps;
}

bool Installer::removeApp(const QString &appName)
{
    log("Eliminando aplicación: " + appName);
    
    QSqlQuery query(m_db);
    query.prepare("SELECT install_path, exec_path FROM installed_apps WHERE app_name = ?");
    query.addBindValue(appName);
    
    if (!query.exec() || !query.next()) {
        log("ERROR: Aplicación no encontrada en los registros");
        return false;
    }
    
    QString installPath = query.value(0).toString();
    QString execPath = query.value(1).toString();
    
    QDir dir(installPath);
    if (dir.exists()) {
        if (!dir.removeRecursively()) {
            log("ADVERTENCIA: No se pudo eliminar completamente el directorio de instalación");
        }
    }
    
    QString symlinkName = "/usr/local/bin/" + appName;
    QFile::remove(symlinkName);
    
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/" + appName + ".desktop";
    QFile::remove(desktopPath);
    
    query.prepare("DELETE FROM installed_apps WHERE app_name = ?");
    query.addBindValue(appName);
    
    if (!query.exec()) {
        log("ERROR: No se pudo eliminar el registro de la base de datos");
        return false;
    }
    
    log("Aplicación eliminada exitosamente");
    return true;
}

void Installer::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 50) / bytesTotal);
        updateProgress(progress);
        log(QString("Descargado: %1/%2 bytes").arg(bytesReceived).arg(bytesTotal));
    }
}

bool Installer::extractTarball(const QString &tarballPath, const QString &destPath)
{
    log("Extrayendo tarball...");
    
    // First, verify the tarball exists and is readable
    QFileInfo tarballInfo(tarballPath);
    if (!tarballInfo.exists() || !tarballInfo.isReadable()) {
        log("ERROR: El archivo tarball no existe o no se puede leer: " + tarballPath);
        return false;
    }
    
    // Verify destination directory exists and is writable
    QFileInfo destInfo(destPath);
    if (!destInfo.exists() || !destInfo.isWritable()) {
        log("ERROR: El directorio de destino no existe o no se puede escribir: " + destPath);
        return false;
    }
    
    // List contents before extraction for debugging
    QDir destDir(destPath);
    QStringList beforeContents = destDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    log("Contenido antes de extracción: " + QString::number(beforeContents.size()) + " elementos");
    
    QProcess process;
    QStringList arguments;
    
    // Build command based on file extension
    if (tarballPath.endsWith(".tar.gz") || tarballPath.endsWith(".tgz")) {
        arguments << "-xzf" << tarballPath << "-C" << destPath;
    } else if (tarballPath.endsWith(".tar.bz2") || tarballPath.endsWith(".tbz2")) {
        arguments << "-xjf" << tarballPath << "-C" << destPath;
    } else if (tarballPath.endsWith(".tar.xz")) {
        arguments << "-xJf" << tarballPath << "-C" << destPath;
    } else if (tarballPath.endsWith(".tar")) {
        arguments << "-xf" << tarballPath << "-C" << destPath;
    } else {
        log("ERROR: Formato de tarball no soportado: " + tarballPath);
        return false;
    }
    
    log("Ejecutando: tar " + arguments.join(" "));
    
    // Set up process environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    process.setProcessEnvironment(env);
    
    // Start the process
    process.start("tar", arguments);
    
    // Wait for completion with timeout
    if (!process.waitForFinished(300000)) { // 5 minutes timeout
        log("ERROR: Timeout extrayendo tarball");
        process.kill();
        process.waitForFinished();
        return false;
    }
    
    // Check exit code
    int exitCode = process.exitCode();
    if (exitCode != 0) {
        QString errorOutput = process.readAllStandardError();
        log(QString("ERROR: Falló la extracción - Código de salida: %1").arg(exitCode));
        log("ERROR Output: " + errorOutput);
        return false;
    }
    
    // List contents after extraction
    QStringList afterContents = destDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    log("Contenido después de extracción: " + QString::number(afterContents.size()) + " elementos");
    
    if (afterContents.isEmpty()) {
        log("ERROR: No se extrajo ningún contenido del tarball");
        return false;
    }
    
    // Log what was extracted
    foreach (const QString &item, afterContents) {
        QFileInfo itemInfo(destDir.absoluteFilePath(item));
        if (itemInfo.isDir()) {
            log("Directorio extraído: " + item);
        } else {
            log("Archivo extraído: " + item);
        }
    }
    
    log("Extracción completada exitosamente");
    return true;
}

bool Installer::createDesktopEntry(const QString &appName, const QString &execPath, const QString &iconPath)
{
    // Determine desktop path based on user privileges
    QString desktopPath;
    if (checkAdminPrivileges()) {
        // Running as root - use global system directory
        desktopPath = "/usr/share/applications";
        log("Creando entrada de escritorio global en: " + desktopPath);
    } else {
        // Running as normal user - use user directory
        desktopPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        log("Creando entrada de escritorio de usuario en: " + desktopPath);
    }
    
    QDir().mkpath(desktopPath);
    
    QString desktopFile = desktopPath + "/" + appName + ".desktop";
    
    // Determine appropriate name and icon based on the application type
    QString displayName = appName;
    QString iconResource = iconPath;
    QString categories = "Development;";
    QString comment = "Code Editor";
    
    // Customize for different editors
    if (appName.contains("windsurf", Qt::CaseInsensitive)) {
        displayName = "Windsurf";
        comment = "AI-powered code editor";
        if (iconResource.isEmpty()) {
            iconResource = "windsurf";
        }
    } else if (appName.contains("cursor", Qt::CaseInsensitive)) {
        displayName = "Cursor";
        comment = "AI-powered IDE";
        if (iconResource.isEmpty()) {
            iconResource = "cursor";
        }
    } else if (appName.contains("code", Qt::CaseInsensitive)) {
        displayName = "Visual Studio Code";
        comment = "Code editing. Redefined.";
        if (iconResource.isEmpty()) {
            iconResource = "code";
        }
    }
    
    QString content = QString(
        "[Desktop Entry]\n"
        "Name=%1\n"
        "Comment=%2\n"
        "Exec=%3\n"
        "Icon=%4\n"
        "Type=Application\n"
        "Categories=%5\n"
        "Terminal=false\n"
        "StartupWMClass=%6\n"
    ).arg(displayName).arg(comment).arg(execPath).arg(iconResource).arg(categories).arg(appName);
    
    QFile file(desktopFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        log("ERROR: No se pudo crear el archivo .desktop: " + desktopFile);
        return false;
    }
    
    file.write(content.toUtf8());
    file.close();
    
    log("Entrada de escritorio creada: " + desktopFile);
    return true;
}

bool Installer::createSymlink(const QString &targetPath, const QString &linkName)
{
    QFile::remove(linkName);
    
    return QFile::link(targetPath, linkName);
}

bool Installer::registerApp(const QString &appName, const QString &version, const QString &installPath,
                           const QString &sourceUrl, const QString &execPath)
{
    QSqlQuery query(m_db);
    
    query.prepare("INSERT OR REPLACE INTO installed_apps "
                  "(app_name, version, install_path, source_url, exec_path, install_date) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(appName);
    query.addBindValue(version);
    query.addBindValue(installPath);
    query.addBindValue(sourceUrl);
    query.addBindValue(execPath);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    
    return query.exec();
}

bool Installer::downloadFile(const QUrl &url, const QString &destPath)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::downloadProgress, this, &Installer::onDownloadProgress);
    
    loop.exec();
    
    if (reply->error() != QNetworkReply::NoError) {
        log("ERROR de descarga: " + reply->errorString());
        reply->deleteLater();
        return false;
    }
    
    QFile file(destPath);
    if (!file.open(QIODevice::WriteOnly)) {
        log("ERROR: No se pudo crear el archivo de destino");
        reply->deleteLater();
        return false;
    }
    
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    
    return true;
}

QString Installer::findExecutableInDirectory(const QString &dirPath)
{
    return findExecutableInDirectoryRecursive(dirPath, 0);
}

QString Installer::findExecutableInDirectoryRecursive(const QString &dirPath, int depth)
{
    const int MAX_DEPTH = 3;
    
    log("Buscando ejecutables en: " + dirPath + " (profundidad: " + QString::number(depth) + ")");
    
    QDir dir(dirPath);
    if (!dir.exists()) {
        log("ERROR: El directorio no existe: " + dirPath);
        return "";
    }
    
    // Check for executables in current directory
    QStringList files = dir.entryList(QDir::Files | QDir::Executable);
    log("Archivos ejecutables en directorio: " + QString::number(files.size()));
    
    foreach (const QString &file, files) {
        QString filePath = dirPath + "/" + file;
        QFileInfo fileInfo(filePath);
        
        // Skip some common non-application executables
        if (file.toLower() == "sh" || file.toLower() == "bash" || 
            file.toLower() == "chmod" || file.toLower() == "ln") {
            continue;
        }
        
        log("Archivo ejecutable encontrado: " + filePath);
        
        // Check if it's a binary file (not a script)
        if (fileInfo.isFile() && fileInfo.isExecutable()) {
            // Additional check: see if it's a binary (ELF) or a script
            QFile execFile(filePath);
            if (execFile.open(QIODevice::ReadOnly)) {
                QByteArray header = execFile.read(4);
                execFile.close();
                
                // ELF files start with 0x7F 'ELF'
                QByteArray elfHeader = QByteArray::fromHex("7F454C46");
                if (header.startsWith(elfHeader) || header.startsWith("#!")) {
                    log("Ejecutable válido encontrado: " + filePath);
                    return filePath;
                }
            }
        }
    }
    
    // If no executables found and we haven't reached max depth, check subdirectories
    if (files.isEmpty() && depth < MAX_DEPTH) {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        log("Subdirectorios encontrados: " + QString::number(subdirs.size()));
        
        // If exactly one subdirectory, descend automatically
        if (subdirs.size() == 1) {
            QString subPath = dirPath + "/" + subdirs.first();
            log("Descendiendo automáticamente al único subdirectorio: " + subPath);
            return findExecutableInDirectoryRecursive(subPath, depth + 1);
        }
        
        // If multiple subdirectories, try each one
        foreach (const QString &subdir, subdirs) {
            QString subPath = dirPath + "/" + subdir;
            log("Probando subdirectorio: " + subPath);
            QString result = findExecutableInDirectoryRecursive(subPath, depth + 1);
            if (!result.isEmpty()) {
                return result;
            }
        }
    }
    
    log("No se encontró ejecutable en: " + dirPath);
    return "";
}

QString Installer::getAppNameFromPath(const QString &execPath)
{
    QFileInfo execInfo(execPath);
    QString appName = execInfo.baseName();
    
    // Special handling for common executable names
    if (appName.toLower() == "code") {
        return "VSCode";
    } else if (appName.toLower() == "windsurf") {
        return "Windsurf";
    } else if (appName.toLower() == "cursor") {
        return "Cursor";
    } else if (appName.toLower() == "vscode") {
        return "VSCode";
    }
    
    return appName;
}

QString Installer::getVersionFromExecutable(const QString &execPath)
{
    QProcess process;
    
    // Try different version commands for different editors
    QStringList versionCommands;
    
    // Check if it's Windsurf
    if (execPath.contains("windsurf", Qt::CaseInsensitive)) {
        versionCommands << "--version" << "-v" << "version";
    }
    // Check if it's Cursor
    else if (execPath.contains("cursor", Qt::CaseInsensitive)) {
        versionCommands << "--version" << "-v" << "version";
    }
    // Default for VS Code variants
    else {
        versionCommands << "--version" << "-v" << "version";
    }
    
    foreach (const QString &cmd, versionCommands) {
        process.start(execPath, QStringList() << cmd);
        process.waitForFinished(3000);
        
        if (process.exitCode() == 0) {
            QString output = process.readAllStandardOutput();
            QRegularExpression re(R"(\d+\.\d+\.\d+)");
            QRegularExpressionMatch match = re.match(output);
            
            if (match.hasMatch()) {
                return match.captured(0);
            }
        }
    }
    
    return "Desconocida";
}

bool Installer::initializeDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    m_db.setDatabaseName(dbPath + "/apps.db");
    
    if (!m_db.open()) {
        log("ERROR: No se pudo abrir la base de datos");
        return false;
    }
    
    QSqlQuery query(m_db);
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS installed_apps (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            app_name TEXT UNIQUE NOT NULL,
            version TEXT,
            install_path TEXT NOT NULL,
            source_url TEXT,
            exec_path TEXT,
            install_date TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query.exec(createTable);
}

void Installer::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    
    emit logMessage(logEntry);
}

void Installer::updateProgress(int value)
{
    if (m_progressBar) {
        m_progressBar->setValue(value);
    }
    emit progressUpdated(value);
}

bool Installer::checkAdminPrivileges() const
{
    // Check if we're running as root
    return geteuid() == 0;
}

bool Installer::restartWithAdminPrivileges(const QStringList &args)
{
    QString program = QCoreApplication::applicationFilePath();
    
    // Use pkexec for privilege escalation (recommended for Linux)
    QStringList pkexecArgs;
    pkexecArgs << program;
    pkexecArgs.append(args);
    
    QProcess::startDetached("pkexec", pkexecArgs);
    
    // Exit current instance
    QCoreApplication::exit(0);
    
    return true;
}

bool Installer::checkDependencies()
{
    log("Verificando dependencias del sistema...");
    
    // Check for tar command
    QProcess process;
    process.start("tar", QStringList() << "--version");
    process.waitForFinished(3000);
    
    if (process.exitCode() != 0) {
        log("ERROR: El comando 'tar' no está disponible o no funciona correctamente");
        log("Por favor instale el paquete 'tar' usando el gestor de paquetes de su sistema:");
        log("  Ubuntu/Debian: sudo apt install tar");
        log("  Fedora/CentOS: sudo dnf install tar");
        log("  Arch Linux: sudo pacman -S tar");
        return false;
    }
    
    log("Dependencias verificadas correctamente");
    return true;
}

bool Installer::copyDirectoryRecursively(const QString &sourcePath, const QString &destPath)
{
    log("Iniciando copia recursiva de " + sourcePath + " a " + destPath);
    
    QDir sourceDir(sourcePath);
    QDir destDir(destPath);
    
    if (!sourceDir.exists()) {
        log("ERROR: Directorio fuente no existe: " + sourcePath);
        return false;
    }
    
    // Create destination directory if it doesn't exist
    if (!destDir.exists()) {
        if (!destDir.mkpath(destPath)) {
            log("ERROR: No se pudo crear directorio destino: " + destPath);
            return false;
        }
    }
    
    // Copy all files and subdirectories
    QStringList entries = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    
    int totalFiles = entries.size();
    int copiedFiles = 0;
    
    foreach (const QString &entry, entries) {
        QString sourceEntry = sourcePath + "/" + entry;
        QString destEntry = destPath + "/" + entry;
        
        QFileInfo fileInfo(sourceEntry);
        
        if (fileInfo.isDir()) {
            // Recursively copy subdirectory
            if (!copyDirectoryRecursively(sourceEntry, destEntry)) {
                log("ERROR: Falló copia de subdirectorio: " + sourceEntry);
                return false;
            }
        } else {
            // Copy file
            log("Copiando archivo: " + sourceEntry + " -> " + destEntry);
            
            // Remove destination file if it exists
            if (QFile::exists(destEntry)) {
                QFile::remove(destEntry);
            }
            
            if (!QFile::copy(sourceEntry, destEntry)) {
                log("ERROR: Falló copia de archivo: " + sourceEntry);
                return false;
            }
            
            // Preserve executable permissions
            if (fileInfo.isExecutable()) {
                QFile destFile(destEntry);
                destFile.setPermissions(fileInfo.permissions());
            }
        }
        
        // Update progress and process UI events to prevent freezing
        copiedFiles++;
        if (copiedFiles % 10 == 0) { // Update every 10 files
            int progressPercent = 60 + (int)((float)copiedFiles / totalFiles * 30); // 60-90% range
            updateProgress(progressPercent);
            
            // Process UI events to keep interface responsive
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }
    
    log("Copia recursiva completada exitosamente");
    return true;
}

bool Installer::needsAdminPrivileges(const QString &installPath, bool createSymlink) const
{
    // Check if install path requires admin privileges
    QFileInfo installDir(installPath);
    if (!installDir.exists()) {
        // For common system directories that require admin
        if (installPath.startsWith("/opt") || 
            installPath.startsWith("/usr") || 
            installPath.startsWith("/etc") ||
            installPath.startsWith("/var")) {
            return true;
        }
    } else {
        // Check if we have write permissions to the install directory
        if (!installDir.isWritable()) {
            return true;
        }
    }
    
    // Check if symlink creation requires admin privileges
    if (createSymlink) {
        QFileInfo binDir("/usr/local/bin");
        if (!binDir.exists() || !binDir.isWritable()) {
            return true;
        }
    }
    
    return false;
}
