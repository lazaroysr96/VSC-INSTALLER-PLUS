#ifndef LAUNCHERCREATOR_H
#define LAUNCHERCREATOR_H

#include <QWidget>
#include <QIcon>

class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;
class QComboBox;

class LauncherCreator : public QWidget
{
    Q_OBJECT

public:
    explicit LauncherCreator(QWidget *parent = nullptr);

private slots:
    void onBrowseExecutableButtonClicked();
    void onBrowseIconButtonClicked();
    void onCreateLauncherButtonClicked();
    void onClearButtonClicked();

private:
    void setupUI();
    void resetForm();
    bool createDesktopFile(const QString &name, const QString &exec, const QString &icon, 
                          const QStringList &categories, const QString &mimeType);
    QString getSelectedCategories() const;
    QString getMimeType() const;

    // UI elements
    QLineEdit *m_nameLineEdit;
    QLineEdit *m_executableLineEdit;
    QLineEdit *m_iconLineEdit;
    QPushButton *m_browseExecutableButton;
    QPushButton *m_browseIconButton;
    QPushButton *m_createButton;
    QPushButton *m_clearButton;
    QLabel *m_iconPreviewLabel;
    QComboBox *m_mimeTypeComboBox;
    
    // Category checkboxes
    QCheckBox *m_developmentCheckBox;
    QCheckBox *m_officeCheckBox;
    QCheckBox *m_graphicsCheckBox;
    QCheckBox *m_audioVideoCheckBox;
    QCheckBox *m_systemCheckBox;
    QCheckBox *m_utilityCheckBox;
    QCheckBox *m_networkCheckBox;
    QCheckBox *m_gameCheckBox;
    QCheckBox *m_educationCheckBox;
};

#endif // LAUNCHERCREATOR_H
