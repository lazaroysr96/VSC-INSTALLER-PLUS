#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QTextEdit>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include "Installer.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onBrowseButtonClicked();
    void onBrowseInstallButtonClicked();
    void onInstallButtonClicked();
    void onUpdateButtonClicked();
    void onClearButtonClicked();
    void onLocalFileRadioToggled(bool checked);
    void onUrlRadioToggled(bool checked);
    void onActionSalirTriggered();
    void onActionVerInstaladosTriggered();
    void onActionLimpiarRegistrosTriggered();
    void onInstallationCompleted(bool success, const QString &message);
    void onProgressUpdated(int value);
    void onLogMessage(const QString &message);
    void onAdminPrivilegesRequired();
    
    // Public methods for auto-install
    void setLocalFile(const QString &filePath);
    void setUrl(const QString &url);
    void setInstallPath(const QString &path);
    void setCreateDesktop(bool create);
    void setCreateSymlink(bool create);
    void startAutoInstall();

private:
    void setupConnections();
    void resetForm();
    void enableControls(bool enabled);
    QString getSelectedSource() const;
    QString getInstallPath() const;
    bool shouldCreateDesktop() const;
    bool shouldCreateSymlink() const;

    Ui::MainWindow *ui;
    Installer *m_installer;
};

#endif // MAINWINDOW_H
