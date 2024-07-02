#include "QtGUI.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include<QFileDialog>
#include<QVariant>
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
    connect(dialog, &CameraDialog::cameraModelSelected, this, [this](const QString& model) {
        m_cameraModel = model; // 将相机型号保存到成员变量
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

//相机
CameraDialog::CameraDialog(QWidget* parent)
{
    setWindowTitle("选择摄像机");
    QVBoxLayout* layout = new QVBoxLayout(this);

    modelLineEdit = new QLineEdit(this);
    confirmButton = new QPushButton("确认", this);
    resultLabel = new QLabel(this);

    layout->addWidget(new QLabel("输入相机型号：", this));
    layout->addWidget(modelLineEdit);
    layout->addWidget(confirmButton);
    layout->addWidget(resultLabel);

    setLayout(layout);

    connect(confirmButton, &QPushButton::clicked, this, &CameraDialog::onConfirmClicked);
}

void CameraDialog::onConfirmClicked()//点击寻找功能
{
    QString model = modelLineEdit->text();
    QString result = searchCamera(model);
    resultLabel->setText(result);
    emit cameraModelSelected(model);
}


QString CameraDialog::searchCamera(const QString& model)
{
    QFile file("cameras.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return "Error: Cannot open database file.";
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 2 && parts[0].trimmed() == model.trimmed())
        {
            file.close();
            return parts[1].trimmed();
            
        }
    }

    file.close();
    return "Camera model not found.";
}
//相机
void QtGUI::executeSFM()
{
    if (m_cameraModel.isEmpty() || m_imageFolderPath.isEmpty() || m_algorithm.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先选择相机型号、图片文件夹和匹配算法");
        return;
    }

    // 调用 SFM 重建函数
    bool success = performSFMReconstruction(m_cameraModel, m_imageFolderPath, m_algorithm,m_save,
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
//算法选择
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
    // 可以根据需要添加更多算法

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
//算法选择

SaveDialog::SaveDialog(QWidget* parent)
{
    setWindowTitle("选择保存格式");
    QVBoxLayout* layout = new QVBoxLayout(this);

    SaveComboBox = new QComboBox(this);
    confirmButton = new QPushButton("确认", this);

    // 添加保存选项
    SaveComboBox->addItem("ply");
    SaveComboBox->addItem("obj");
    // 可以根据需要添加格式

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