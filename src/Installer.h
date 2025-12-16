#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QSqlDatabase>
#include <QProgressBar>
#include <QTextEdit>

class Installer : public QObject
{
    Q_OBJECT

public:
    explicit Installer(QObject *parent = nullptr);
    ~Installer();

    void setProgressBar(QProgressBar *bar);
    void setLogTextEdit(QTextEdit *textEdit);

    bool installFromLocalFile(const QString &filePath, const QString &installPath,
                             bool createDesktop, bool createSymlink);
    bool installFromUrl(const QUrl &url, const QString &installPath,
                       bool createDesktop, bool createSymlink);
    
    bool updateExistingApp(const QString &appName, const QString &newSource,
                          const QString &installPath, bool isUrl);
    
    QStringList getInstalledApps() const;
    bool removeApp(const QString &appName);
    
    bool checkAdminPrivileges() const;
    bool restartWithAdminPrivileges(const QStringList &args);
    bool checkDependencies();

signals:
    void progressUpdated(int value);
    void logMessage(const QString &message);
    void installationCompleted(bool success, const QString &message);
    void adminPrivilegesRequired();

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    bool extractTarball(const QString &tarballPath, const QString &destPath);
    bool createDesktopEntry(const QString &appName, const QString &execPath, const QString &iconPath);
    bool createSymlink(const QString &targetPath, const QString &linkName);
    bool registerApp(const QString &appName, const QString &version, const QString &installPath,
                     const QString &sourceUrl, const QString &execPath);
    bool downloadFile(const QUrl &url, const QString &destPath);
    QString findExecutableInDirectory(const QString &dirPath);
    QString getAppNameFromPath(const QString &path);
    QString getVersionFromExecutable(const QString &execPath);
    
    bool initializeDatabase();
    void log(const QString &message);
    void updateProgress(int value);
    bool needsAdminPrivileges(const QString &installPath, bool createSymlink) const;
    bool copyDirectoryRecursively(const QString &sourcePath, const QString &destPath);
    mutable QString m_tempLogBuffer;

    QSqlDatabase m_db;
    QProgressBar *m_progressBar;
    QTextEdit *m_logTextEdit;
    QString m_currentDownloadPath;
};

#endif // INSTALLER_H
