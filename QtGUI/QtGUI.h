#pragma once

#include <QtWidgets/QWidget>
#include<QtWidgets/qmenu.h>
#include "ui_QtGUI.h"
#include <QMenu>
#include <QDialog>
#include<QLineEdit>
#include"sfm_reconstruction.h"
#include<QLabel>
#include<qtextedit.h>
#include<QTextEdit>
#include <QComboBox>


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
    QString m_cameraModel;
    QString m_imageFolderPath;
    QString m_algorithm;
    QString m_save;
    void executeSFM();
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

signals:
    void cameraModelSelected(const QString& model);

private slots:
    void onConfirmClicked();

private:
    QLineEdit* modelLineEdit;
    QPushButton* confirmButton;
    QLabel* resultLabel;
    QString searchCamera(const QString& model);
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