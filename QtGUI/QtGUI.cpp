#include "QtGUI.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QVariant>
#include <QCoreApplication>


QtGUI::QtGUI(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

QtGUI::~QtGUI()
{}

void QtGUI::initializeUI()
{
    QPushButton* set_button = ui.set_button;
    QMenu* set_menu = new QMenu(this);
    set_menu->addAction("选择摄像机", this, &QtGUI::on1selected);
    set_menu->addAction("选择图片路径", this, &QtGUI::on2selected);
    set_menu->addAction("选择匹配算法", this, &QtGUI::on3selected);
    set_menu->addAction("选择保存格式", this, &QtGUI::on4selected);
    connect(set_button, &QPushButton::clicked, this, &QtGUI::show_set);
    connect(ui.sfm_button, &QPushButton::clicked, this, &QtGUI::executeSFM);
    this->set_menu = set_menu;
    m_textedit = ui.textEdit;
  

}

void QtGUI::show_set()
{
    QPoint pos = ui.set_button->mapToGlobal(QPoint(0, ui.set_button->height()));

    // 在按钮下方显示菜单
    set_menu->exec(pos);
}

void QtGUI::on1selected()
{
    qDebug() << "参数1被选中";
    CameraDialog* dialog = new CameraDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    connect(dialog, &CameraDialog::cameraIntrinsicsSelected, this, [this](const QString& Intrinsics) {
        m_cameraIntrinsics = Intrinsics; // 将相机型号保存到成员变量
        });
}

void QtGUI::on2selected()
{
    // 打开文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择图片文件夹", QDir::homePath());

    if (folderPath.isEmpty()) {
        qDebug() << "没有选择文件夹";
        return;
    }

    QDir directory(folderPath);
    QStringList imageFilters;
    imageFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif";
    QStringList imageFiles = directory.entryList(imageFilters, QDir::Files);

    int imageCount = imageFiles.count();

    qDebug() << "选中的文件夹：" << folderPath;
    qDebug() << "图片数量：" << imageCount;

    // 显示结果给用户
    QMessageBox::information(this, "图片统计",
    QString("选中的文件夹：%1\n图片数量：%2").arg(folderPath).arg(imageCount));
    m_imageFolderPath = folderPath;
}

void QtGUI::on3selected()
{
    AlgorithmDialog* dialog = new AlgorithmDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    connect(dialog, &AlgorithmDialog::algorithmSelected, this, &QtGUI::onAlgorithmSelected);
}

void QtGUI::on4selected()
{
    SaveDialog* dialog = new SaveDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    connect(dialog, &SaveDialog::SaveSelected, this, &QtGUI::onSaveSelected);
}

QString CameraDialog::validateKMatrix(const QString& kMatrixString)
{
    // 使用分号分割字符串
    QStringList parts = kMatrixString.split(";");
    if (parts.size() != 9) {
        return "K矩阵应包含9个数字以;分隔";
    }

    // 尝试将每个部分转换为浮点数，并验证转换是否成功
    bool ok;
    QVector<double> kMatrixValues;
    for (const QString& part : parts) {
        double value = part.toDouble(&ok);
        if (!ok) {
            return "K矩阵包含非数字字符";
        }
        kMatrixValues.append(value);
    }

    return "K矩阵验证并处理成功";
}

CameraDialog::CameraDialog(QWidget* parent)
{
    setWindowTitle("请输入相机内参K矩阵（如：2905.88;0;1416;0;2905.88;1064;0;0;1）");
    QVBoxLayout* layout = new QVBoxLayout(this);

    IntrinsicsLineEdit = new QLineEdit(this);
    confirmButton = new QPushButton("确认", this);

    layout->addWidget(new QLabel("输入K矩阵：", this));
    layout->addWidget(IntrinsicsLineEdit);
    layout->addWidget(confirmButton);

    setLayout(layout);
    resize(420, 95);

    connect(confirmButton, &QPushButton::clicked, this, &CameraDialog::onConfirmClicked);
}

void CameraDialog::onConfirmClicked()
{
    QString kMatrixString = IntrinsicsLineEdit->text();
    QString result = validateKMatrix(kMatrixString);

    if (result == "K矩阵验证并处理成功") {
        QMessageBox::information(this, "验证成功", result);
        emit cameraIntrinsicsSelected(kMatrixString);
        accept();
    }
    else {
        QMessageBox::critical(this, "验证失败", result);
        IntrinsicsLineEdit->clear();
    }
}


void QtGUI::executeSFM()
{
    if (m_cameraIntrinsics.isEmpty() || m_imageFolderPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先至少选择相机内参、图片文件夹");
        return;
    }

    // 调用 SFM 重建函数
    bool success = performSFMReconstruction(m_cameraIntrinsics, m_imageFolderPath, m_algorithm, m_save,
        [this](const QString& message) {
            appendToTextEdit(message);
        });
    if (success) {
        QMessageBox::information(this, "成功", "SFM 稀疏点云重建完成");
    }
    else {
        QMessageBox::critical(this, "错误", "SFM 重建失败");
    }
}

void QtGUI::appendToTextEdit(const QString& text)
{
    if (m_textedit) {
        m_textedit->clear();  // 清除所有现有文本
        m_textedit->setText(text);  // 设置新文本
        m_textedit->repaint();  // 强制重绘
        QCoreApplication::processEvents();
        }
}

AlgorithmDialog::AlgorithmDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("选择匹配算法");
    QVBoxLayout* layout = new QVBoxLayout(this);

    algorithmComboBox = new QComboBox(this);
    confirmButton = new QPushButton("确认", this);

    // 添加算法选项
    algorithmComboBox->addItem("SIFT_ANATOMY");
    algorithmComboBox->addItem("AKAZE_FLOAT");
    algorithmComboBox->addItem("AKAZE_MLDB");

    layout->addWidget(new QLabel("选择匹配算法：", this));
    layout->addWidget(algorithmComboBox);
    layout->addWidget(confirmButton);

    setLayout(layout);

    connect(confirmButton, &QPushButton::clicked, this, &AlgorithmDialog::onConfirmClicked);
}

void AlgorithmDialog::onConfirmClicked()
{
    QString selectedAlgorithm = algorithmComboBox->currentText();
    emit algorithmSelected(selectedAlgorithm);
    accept();
}

void QtGUI::onAlgorithmSelected(const QString& algorithm)
{
    m_algorithm = algorithm;
    QMessageBox::information(this, "算法选择", QString("选择的匹配算法：%1").arg(algorithm));
}

SaveDialog::SaveDialog(QWidget* parent)
{
    setWindowTitle("选择保存格式");
    QVBoxLayout* layout = new QVBoxLayout(this);

    SaveComboBox = new QComboBox(this);
    confirmButton = new QPushButton("确认", this);

    // 添加保存选项
    SaveComboBox->addItem("ply");
    SaveComboBox->addItem("obj");

    layout->addWidget(new QLabel("选择保存格式：", this));
    layout->addWidget(SaveComboBox);
    layout->addWidget(confirmButton);

    setLayout(layout);

    connect(confirmButton, &QPushButton::clicked, this, &SaveDialog::onConfirmClicked);
}

void SaveDialog::onConfirmClicked()
{
    QString selectedSave= SaveComboBox->currentText();
    emit SaveSelected(selectedSave);
    accept();
}

void QtGUI::onSaveSelected(const QString& Save)
{
    m_save = Save;
    QMessageBox::information(this, "保存格式", QString("选择保存格式：%1").arg(Save));
}