#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/qmenu.h>
#include <ui_QtGUI.h>
#include <QMenu>
#include <QDialog>
#include <QLineEdit>

#include <QLabel>
#include <qtextedit.h>
#include <QTextEdit>
#include <QComboBox>

#include "SfMWorker.h"

#include "../Viewer/Common.h"
#include "../Viewer/Scene.h"

class QtGUI : public QWidget
{
    Q_OBJECT

public:
    QtGUI(QWidget *parent = nullptr);
    ~QtGUI();
    void appendToTextEdit(const QString& text);
    void initializeUI();

private:
    Ui::QtWidgetsApplication1Class ui;
    QTextEdit* m_textedit;
    QMenu* set_menu;
    QString m_cameraIntrinsics = "2905.88;0;1416;0;2905.88;1064;0;0;1";
    QString m_imageFolderPath = "D:\\Code\\VS2022\\repos\\3D-Reconstruction\\Main\\data\\openMVG_Castle";
    QString m_algorithm = "AKAZE_FLOAT";
    QString m_save = "ply";
    void executeSFM();
    void executeMVS();
private slots:
    void on1selected();
    void on2selected();
    void on3selected();
    void on4selected();
    void show_set();
    void onAlgorithmSelected(const QString& algorithm);
    void onSaveSelected(const QString& Save);
};

class CameraDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraDialog(QWidget* parent = nullptr);
    QString validateKMatrix(const QString& kMatrix);

signals:
    void cameraIntrinsicsSelected(const QString& model);

private slots:
    void onConfirmClicked();

private:
    QLineEdit* IntrinsicsLineEdit;
    QPushButton* confirmButton;
};

class AlgorithmDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlgorithmDialog(QWidget* parent = nullptr);
signals:
    void algorithmSelected(const QString& algorithm);
private slots:
    void onConfirmClicked();
private:
    QComboBox* algorithmComboBox;
    QPushButton* confirmButton;
};

class SaveDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SaveDialog(QWidget* parent = nullptr);
signals:
    void SaveSelected(const QString& Save);
private slots:
    void onConfirmClicked();
private:
    QComboBox* SaveComboBox;
    QPushButton* confirmButton;
};