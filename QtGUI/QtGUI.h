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

#include "../MVSViewer/Common.h"
#include "../MVSViewer/Scene.h"

class QtGUI : public QWidget
{
    Q_OBJECT

public:
    QtGUI(QWidget *parent = nullptr);
    ~QtGUI();
    void appendToTextEdit(const QString& text);
    void initializeUI();
    void auto_viewer(const QString& filename);

private:
    Ui::QtWidgetsApplication1Class ui;
    QTextEdit* m_textedit;
    QMenu* set_menu;
    QMenu* pi_menu;
    QString m_cameraIntrinsics;
	QString m_imageFolderPath;
    QString m_algorithm = "AKAZE_FLOAT";
    QString m_save = "ply";
    QString m_PiImageFolderPath;
    QString m_ssh_host = "192.168.216.49";
    QString m_ssh_user = "user";
    QString m_ssh_password = "1234";

    VIEWER::Scene* MVSViewer = nullptr;
    bool ViewerAvailable = false;

    bool openViewer(QString fileName);
    void executeSFM();
    void executeMVS();

private slots:
    // void show_set_pi();
    void onStartShooting();
    void onPiSSHSelect();
    void onPiFolderSelect();
    void onExecuteSfM_pi();
    void onExecuteMVS_pi();

    void on1selected();
    void on2selected();
    void on3selected();
    void on4selected();
    void show_set();
    void onAlgorithmSelected(const QString& algorithm);
    void onSaveSelected(const QString& Save);

    void on_viewer_button_clicked();
    void on_sfm_viewer_button_clicked();
    void on_densify_viewer_button_clicked();
    void on_mesh_viewer_button_clicked();
    void on_refinemesh_viewer_button_clicked();
    void on_texture_viewer_button_clicked();
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

class SSHDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SSHDialog(QWidget* parent = nullptr);

signals:
    void sshCredentialsEntered(const QString& host, const QString& user, const QString& password);

private slots:
    void onConfirmClicked();

private:
    QLineEdit* hostLineEdit;
    QLineEdit* userLineEdit;
    QLineEdit* passwordLineEdit;
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