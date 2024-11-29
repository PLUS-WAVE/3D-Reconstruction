#include "ImageGetter.h"


class StreamBuffer : public std::streambuf {
public:
    std::function<void(const std::string&)> outputCallback;

protected:
    virtual int_type overflow(int_type v) {
        char c = static_cast<char>(v);
        if (outputCallback) outputCallback(std::string(1, c));
        return v;
    }

    virtual std::streamsize xsputn(const char* p, std::streamsize n) {
        if (outputCallback) outputCallback(std::string(p, n));
        return n;
    }
};

bool executeRemoteCommand(ssh_session session, const std::string& command) {
    ssh_channel channel = ssh_channel_new(session);
    if (channel == nullptr) return false;

    if (ssh_channel_open_session(channel) != SSH_OK) {
        ssh_channel_free(channel);
        return false;
    }

    if (ssh_channel_request_exec(channel, command.c_str()) != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return false;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return true;
}

bool transferFile(ssh_session session, const std::string& localPath, const std::string& remotePath) {
    sftp_session sftp = sftp_new(session);
    if (sftp == nullptr) return false;

    if (sftp_init(sftp) != SSH_OK) {
        sftp_free(sftp);
        return false;
    }

    sftp_file file = sftp_open(sftp, remotePath.c_str(), SSH_FXF_WRITE | SSH_FXF_CREAT | SSH_FXF_TRUNC, 0);
    if (file == nullptr) {
        sftp_free(sftp);
        return false;
    }

    std::ifstream inputFile(localPath, std::ios::binary);
    if (!inputFile) {
        sftp_close(file);
        sftp_free(sftp);
        return false;
    }

    char buffer[1024];
    while (inputFile.read(buffer, sizeof(buffer))) {
        if (sftp_write(file, buffer, inputFile.gcount()) != inputFile.gcount()) {
            sftp_close(file);
            sftp_free(sftp);
            return false;
        }
    }

    sftp_close(file);
    sftp_free(sftp);
    return true;
}

bool ImageWorker::performImageGet(const QString& cameraIntrinsics, const QString& imageFolderPath, const QString& algorithm, const QString& save, const std::function<void(const QString&)>& logCallback)
{
    // 保存原始buf
    auto coutBuf = std::cout.rdbuf();
    auto cerrBuf = std::cerr.rdbuf();

    StreamBuffer customBuf;
    customBuf.outputCallback = [&logCallback](const std::string& text) {
        // 使用QStringDecoder进行GB2312到UTF-16的转换
        static QTextDecoder decoder(QTextCodec::codecForName("System"));
        QString decodedText = decoder.toUnicode(text.c_str());

        // 现在decodedText是以UTF-16编码的QString，可以正确显示中文字符
        emit logCallback(decodedText);
    };

    // 重定向
    std::cout.rdbuf(&customBuf);
    std::cerr.rdbuf(&customBuf);

    // SSH连接到树莓派
    ssh_session session = ssh_new();
    if (session == nullptr) return false;

    ssh_options_set(session, SSH_OPTIONS_HOST, "113.54.236.105");
    ssh_options_set(session, SSH_OPTIONS_USER, "user");

    if (ssh_connect(session) != SSH_OK) {
        ssh_free(session);
        return false;
    }

    if (ssh_userauth_password(session, nullptr, "1234") != SSH_AUTH_SUCCESS) {
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    // 执行远程命令进行图像拍摄
    if (!executeRemoteCommand(session, "raspistill -o /home/user/image.jpg")) {
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    // 传输图像文件到本地
    if (!transferFile(session, "/home/user/image.jpg", "local_image.jpg")) {
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    ssh_disconnect(session);
    ssh_free(session);


    // 恢复原始buf
    std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);

    return true;
}