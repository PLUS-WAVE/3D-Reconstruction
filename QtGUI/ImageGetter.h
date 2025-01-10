#pragma once
#include <functional>
#include <iostream>
#include <filesystem>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <Windows.h>

#include <QMessageBox>
#include <QString>
#include <QObject>
#include <QThread>
#include <QTextCodec>
#include <QTextDecoder>

class ImageWorker : public QObject {
    Q_OBJECT

public:
    ImageWorker(QObject* parent = nullptr) : QObject(parent) {}

    void setParameters(const QString& imageFolderPath, const QString& ssh_host, const QString& ssh_user, const QString& ssh_password) {
        this->imageFolderPath = imageFolderPath;
		this->ssh_host = ssh_host;
		this->ssh_user = ssh_user;
		this->ssh_password = ssh_password;
    }

public slots:
    void process() {
		bool success = performImageGet(imageFolderPath, ssh_host, ssh_user, ssh_password,
            [this](const QString& message) {
                emit logMessage(message);
            });

        if (success) {
            emit finished();
        }
        else {
            emit error("Í¼Ïñ»ñÈ¡Ê§°Ü");
        }
    }

signals:
    void logMessage(const QString& message);
    void finished();
    void error(const QString& errorMessage);

private:
    QString imageFolderPath;
    QString ssh_host;
    QString ssh_user;
	QString ssh_password;

    bool performImageGet(
        const QString& imageFolderPath,
        const QString& ssh_host,
		const QString& ssh_user,
		const QString& ssh_password,
        const std::function<void(const QString&)>& logCallback);
};

