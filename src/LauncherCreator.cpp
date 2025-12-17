#include "LauncherCreator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QFileInfo>

LauncherCreator::LauncherCreator(QWidget *parent)
    : QWidget(parent)
    , m_nameLineEdit(nullptr)
    , m_executableLineEdit(nullptr)
    , m_iconLineEdit(nullptr)
    , m_browseExecutableButton(nullptr)
    , m_browseIconButton(nullptr)
    , m_createButton(nullptr)
    , m_clearButton(nullptr)
    , m_iconPreviewLabel(nullptr)
    , m_mimeTypeComboBox(nullptr)
    , m_developmentCheckBox(nullptr)
    , m_officeCheckBox(nullptr)
    , m_graphicsCheckBox(nullptr)
    , m_audioVideoCheckBox(nullptr)
    , m_systemCheckBox(nullptr)
    , m_utilityCheckBox(nullptr)
    , m_networkCheckBox(nullptr)
    , m_gameCheckBox(nullptr)
    , m_educationCheckBox(nullptr)
{
    setupUI();
    resetForm();
}

void LauncherCreator::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel *titleLabel = new QLabel("Generador de Lanzadores", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Basic Information Group
    QGroupBox *basicGroup = new QGroupBox("Información Básica", this);
    QVBoxLayout *basicLayout = new QVBoxLayout(basicGroup);
    
    // Name field
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Nombre:"));
    m_nameLineEdit = new QLineEdit(this);
    m_nameLineEdit->setPlaceholderText("Nombre del lanzador (ej: Mi Aplicación)");
    nameLayout->addWidget(m_nameLineEdit);
    basicLayout->addLayout(nameLayout);
    
    // Executable field
    QHBoxLayout *execLayout = new QHBoxLayout();
    execLayout->addWidget(new QLabel("Ejecutable:"));
    m_executableLineEdit = new QLineEdit(this);
    m_executableLineEdit->setPlaceholderText("Ruta al ejecutable...");
    m_browseExecutableButton = new QPushButton("Examinar...", this);
    execLayout->addWidget(m_executableLineEdit);
    execLayout->addWidget(m_browseExecutableButton);
    basicLayout->addLayout(execLayout);
    
    mainLayout->addWidget(basicGroup);
    
    // Icon Group
    QGroupBox *iconGroup = new QGroupBox("Icono", this);
    QHBoxLayout *iconLayout = new QHBoxLayout(iconGroup);
    
    m_iconLineEdit = new QLineEdit(this);
    m_iconLineEdit->setPlaceholderText("Ruta al icono (opcional)...");
    m_browseIconButton = new QPushButton("Examinar...", this);
    iconLayout->addWidget(m_iconLineEdit);
    iconLayout->addWidget(m_browseIconButton);
    
    m_iconPreviewLabel = new QLabel(this);
    m_iconPreviewLabel->setFixedSize(64, 64);
    m_iconPreviewLabel->setAlignment(Qt::AlignCenter);
    m_iconPreviewLabel->setStyleSheet("border: 1px solid gray; background-color: white;");
    m_iconPreviewLabel->setText("Sin icono");
    iconLayout->addWidget(m_iconPreviewLabel);
    
    mainLayout->addWidget(iconGroup);
    
    // Categories Group
    QGroupBox *categoriesGroup = new QGroupBox("Categorías", this);
    QGridLayout *categoriesLayout = new QGridLayout(categoriesGroup);
    
    m_developmentCheckBox = new QCheckBox("Desarrollo", this);
    m_officeCheckBox = new QCheckBox("Oficina", this);
    m_graphicsCheckBox = new QCheckBox("Gráficos", this);
    m_audioVideoCheckBox = new QCheckBox("Audio/Video", this);
    m_systemCheckBox = new QCheckBox("Sistema", this);
    m_utilityCheckBox = new QCheckBox("Utilidades", this);
    m_networkCheckBox = new QCheckBox("Red", this);
    m_gameCheckBox = new QCheckBox("Juegos", this);
    m_educationCheckBox = new QCheckBox("Educación", this);
    
    categoriesLayout->addWidget(m_developmentCheckBox, 0, 0);
    categoriesLayout->addWidget(m_officeCheckBox, 0, 1);
    categoriesLayout->addWidget(m_graphicsCheckBox, 0, 2);
    categoriesLayout->addWidget(m_audioVideoCheckBox, 1, 0);
    categoriesLayout->addWidget(m_systemCheckBox, 1, 1);
    categoriesLayout->addWidget(m_utilityCheckBox, 1, 2);
    categoriesLayout->addWidget(m_networkCheckBox, 2, 0);
    categoriesLayout->addWidget(m_gameCheckBox, 2, 1);
    categoriesLayout->addWidget(m_educationCheckBox, 2, 2);
    
    mainLayout->addWidget(categoriesGroup);
    
    // File Association Group
    QGroupBox *associationGroup = new QGroupBox("Asociación de Archivos", this);
    QVBoxLayout *associationLayout = new QVBoxLayout(associationGroup);
    
    QHBoxLayout *mimeTypeLayout = new QHBoxLayout();
    mimeTypeLayout->addWidget(new QLabel("Tipo MIME:"));
    m_mimeTypeComboBox = new QComboBox(this);
    m_mimeTypeComboBox->setEditable(true);
    m_mimeTypeComboBox->addItem(""); // Empty option
    m_mimeTypeComboBox->addItem("text/plain");
    m_mimeTypeComboBox->addItem("application/pdf");
    m_mimeTypeComboBox->addItem("image/png");
    m_mimeTypeComboBox->addItem("image/jpeg");
    m_mimeTypeComboBox->addItem("application/zip");
    m_mimeTypeComboBox->addItem("application/x-tar");
    mimeTypeLayout->addWidget(m_mimeTypeComboBox);
    associationLayout->addLayout(mimeTypeLayout);
    
    mainLayout->addWidget(associationGroup);
    
    // Action Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_createButton = new QPushButton("Crear Lanzador", this);
    m_createButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px 16px; }");
    m_clearButton = new QPushButton("Limpiar", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_createButton);
    buttonLayout->addWidget(m_clearButton);
    mainLayout->addLayout(buttonLayout);
    
    mainLayout->addStretch();
    
    // Connect signals
    connect(m_browseExecutableButton, &QPushButton::clicked, this, &LauncherCreator::onBrowseExecutableButtonClicked);
    connect(m_browseIconButton, &QPushButton::clicked, this, &LauncherCreator::onBrowseIconButtonClicked);
    connect(m_createButton, &QPushButton::clicked, this, &LauncherCreator::onCreateLauncherButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &LauncherCreator::onClearButtonClicked);
    
    // Icon preview update
    connect(m_iconLineEdit, &QLineEdit::textChanged, [this](const QString &iconPath) {
        if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
            QPixmap pixmap(iconPath);
            if (!pixmap.isNull()) {
                m_iconPreviewLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_iconPreviewLabel->setText("");
            } else {
                m_iconPreviewLabel->setText("Icono inválido");
            }
        } else {
            m_iconPreviewLabel->setText("Sin icono");
            m_iconPreviewLabel->setPixmap(QPixmap());
        }
    });
}

void LauncherCreator::onBrowseExecutableButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Seleccionar Ejecutable",
        QDir::homePath(),
        "Archivos ejecutables (*)"
    );
    
    if (!fileName.isEmpty()) {
        m_executableLineEdit->setText(fileName);
        
        // Auto-fill name if empty
        if (m_nameLineEdit->text().isEmpty()) {
            QFileInfo fileInfo(fileName);
            m_nameLineEdit->setText(fileInfo.baseName());
        }
    }
}

void LauncherCreator::onBrowseIconButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Seleccionar Icono",
        QDir::homePath(),
        "Archivos de imagen (*.png *.jpg *.jpeg *.svg *.xpm);;Todos los archivos (*)"
    );
    
    if (!fileName.isEmpty()) {
        m_iconLineEdit->setText(fileName);
    }
}

void LauncherCreator::onCreateLauncherButtonClicked()
{
    QString name = m_nameLineEdit->text().trimmed();
    QString executable = m_executableLineEdit->text().trimmed();
    QString icon = m_iconLineEdit->text().trimmed();
    
    // Validation
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Por favor ingrese un nombre para el lanzador.");
        return;
    }
    
    if (executable.isEmpty()) {
        QMessageBox::warning(this, "Error", "Por favor seleccione un ejecutable.");
        return;
    }
    
    if (!QFile::exists(executable)) {
        QMessageBox::warning(this, "Error", "El ejecutable seleccionado no existe.");
        return;
    }
    
    if (!icon.isEmpty() && !QFile::exists(icon)) {
        QMessageBox::warning(this, "Error", "El archivo de icono no existe.");
        return;
    }
    
    // Get selected categories
    QStringList categories;
    if (m_developmentCheckBox->isChecked()) categories << "Development";
    if (m_officeCheckBox->isChecked()) categories << "Office";
    if (m_graphicsCheckBox->isChecked()) categories << "Graphics";
    if (m_audioVideoCheckBox->isChecked()) categories << "AudioVideo";
    if (m_systemCheckBox->isChecked()) categories << "System";
    if (m_utilityCheckBox->isChecked()) categories << "Utility";
    if (m_networkCheckBox->isChecked()) categories << "Network";
    if (m_gameCheckBox->isChecked()) categories << "Game";
    if (m_educationCheckBox->isChecked()) categories << "Education";
    
    if (categories.isEmpty()) {
        categories << "Utility"; // Default category
    }
    
    // Get MIME type
    QString mimeType = getMimeType();
    
    // Create desktop file
    if (createDesktopFile(name, executable, icon, categories, mimeType)) {
        QMessageBox::information(this, "Éxito", 
            QString("Lanzador '%1' creado exitosamente.").arg(name));
        resetForm();
    }
}

void LauncherCreator::onClearButtonClicked()
{
    resetForm();
}

void LauncherCreator::resetForm()
{
    m_nameLineEdit->clear();
    m_executableLineEdit->clear();
    m_iconLineEdit->clear();
    m_iconPreviewLabel->setText("Sin icono");
    m_iconPreviewLabel->setPixmap(QPixmap());
    m_mimeTypeComboBox->setCurrentIndex(0);
    
    // Uncheck all categories
    m_developmentCheckBox->setChecked(false);
    m_officeCheckBox->setChecked(false);
    m_graphicsCheckBox->setChecked(false);
    m_audioVideoCheckBox->setChecked(false);
    m_systemCheckBox->setChecked(false);
    m_utilityCheckBox->setChecked(false);
    m_networkCheckBox->setChecked(false);
    m_gameCheckBox->setChecked(false);
    m_educationCheckBox->setChecked(false);
}

bool LauncherCreator::createDesktopFile(const QString &name, const QString &exec, const QString &icon, 
                                       const QStringList &categories, const QString &mimeType)
{
    QString desktopFilePath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/" + name.toLower().replace(" ", "-") + ".desktop";
    
    QFile desktopFile(desktopFilePath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "No se pudo crear el archivo .desktop.");
        return false;
    }
    
    QTextStream out(&desktopFile);
    out << "[Desktop Entry]\n";
    out << "Version=1.0\n";
    out << "Type=Application\n";
    out << "Name=" << name << "\n";
    out << "Exec=" << exec << "\n";
    
    if (!icon.isEmpty()) {
        out << "Icon=" << icon << "\n";
    }
    
    out << "Categories=" << categories.join(";") << ";\n";
    out << "Terminal=false\n";
    out << "StartupNotify=true\n";
    
    if (!mimeType.isEmpty()) {
        out << "MimeType=" << mimeType << "\n";
    }
    
    desktopFile.close();
    return true;
}

QString LauncherCreator::getSelectedCategories() const
{
    QStringList categories;
    if (m_developmentCheckBox->isChecked()) categories << "Development";
    if (m_officeCheckBox->isChecked()) categories << "Office";
    if (m_graphicsCheckBox->isChecked()) categories << "Graphics";
    if (m_audioVideoCheckBox->isChecked()) categories << "AudioVideo";
    if (m_systemCheckBox->isChecked()) categories << "System";
    if (m_utilityCheckBox->isChecked()) categories << "Utility";
    if (m_networkCheckBox->isChecked()) categories << "Network";
    if (m_gameCheckBox->isChecked()) categories << "Game";
    if (m_educationCheckBox->isChecked()) categories << "Education";
    
    return categories.join(";");
}

QString LauncherCreator::getMimeType() const
{
    return m_mimeTypeComboBox->currentText().trimmed();
}