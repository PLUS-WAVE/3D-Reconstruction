#include "ImageGetter.h"


class StreamBuffer : public std::streambuf {
public:
    std::function<void(const std::string&)> outputCallback;

protected:
    virtual int_type overflow(int_type v) override {
        char c = static_cast<char>(v);
        if (outputCallback) outputCallback(std::string(1, c));
        return v;
    }

    virtual std::streamsize xsputn(const char* p, std::streamsize n) override {
        if (outputCallback) outputCallback(std::string(p, n));
        return n;
    }
};

bool downloadFile(ssh_session session, const std::string& remotePath, const std::string& localPath) {
    sftp_session sftp = sftp_new(session);
    if (sftp == nullptr) {
        std::cerr << "Error: Unable to create SFTP session." << std::endl;
        return false;
    }

    if (sftp_init(sftp) != SSH_OK) {
        std::cerr << "Error: Unable to initialize SFTP session." << std::endl;
        sftp_free(sftp);
        return false;
    }

    // Open the remote file for reading
    sftp_file file = sftp_open(sftp, remotePath.c_str(), O_RDONLY, 0);
    if (file == nullptr) {
        std::cerr << "Error: Unable to open remote file. " << ssh_get_error(session) << std::endl;
        sftp_free(sftp);
        return false;
    }

    // Open the local file for writing
    std::ofstream outputFile(localPath, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Unable to open local file: " << localPath << std::endl;
                sftp_close(file);
        sftp_free(sftp);
        return false;
    }

    // Read from remote file and write to local file
    char buffer[1024];
    int bytesRead;
    while ((bytesRead = sftp_read(file, buffer, sizeof(buffer))) > 0) {
                outputFile.write(buffer, bytesRead);
    }

    if (bytesRead < 0) {
        std::cerr << "Error: Reading from remote file failed. " << ssh_get_error(session) << std::endl;
            sftp_close(file);
        sftp_free(sftp);
        return false;
    }

    sftp_close(file);
    sftp_free(sftp);
    return true;
}

bool downloadDirectory(ssh_session session, const std::string& remoteDir, const std::string& localDir) {
    sftp_session sftp = sftp_new(session);
    if (!sftp || sftp_init(sftp) != SSH_OK) {
        std::cerr << "Error: SFTP init." << std::endl;
        return false;
    }

    sftp_dir dir = sftp_opendir(sftp, remoteDir.c_str());
    if (!dir) {
        std::cerr << "Error: sftp_opendir." << std::endl;
        sftp_free(sftp);
        return false;
    }

    std::vector<std::string> files;
    while (true) {
        sftp_attributes attr = sftp_readdir(sftp, dir);
        if (!attr) break;
        if (!(attr->type & SSH_FILEXFER_TYPE_DIRECTORY)) {
            files.push_back(attr->name);
        }
        sftp_attributes_free(attr);
    }

    std::sort(files.begin(), files.end());

    for (const auto& file : files) {
        std::string remoteFilePath = remoteDir + "/" + file;
        std::string localFilePath = localDir + "\\" + file;
        downloadFile(session, remoteFilePath, localFilePath);
        std::cout << "Downloaded: " << remoteFilePath << std::endl;
    }

    sftp_closedir(dir);
    sftp_free(sftp);
    return true;
}

bool ImageWorker::performImageGet(const QString& imageFolderPath, const QString& ssh_host, const QString& ssh_user, const QString& ssh_password, const std::function<void(const QString&)>& logCallback)
{
    auto coutBuf = std::cout.rdbuf();
    auto cerrBuf = std::cerr.rdbuf();

    StreamBuffer customBuf;
    customBuf.outputCallback = [&logCallback](const std::string& text) {
        static QTextDecoder decoder(QTextCodec::codecForName("System"));
        QString decodedText = decoder.toUnicode(text.c_str());
        emit logCallback(decodedText);
    };

    std::cout.rdbuf(&customBuf);
    std::cerr.rdbuf(&customBuf);

    ssh_session session = ssh_new();
    if (session == nullptr) {
        std::cerr << "Error: Unable to create SSH session." << std::endl;
        return false;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, ssh_host.toStdString().c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, ssh_user.toStdString().c_str());

    if (ssh_connect(session) != SSH_OK) {
        std::cerr << "Error: Unable to connect to SSH server." << std::endl;
        ssh_free(session);
        return false;
    }
    std::cout << "Connected to remote server" << std::endl;

    if (ssh_userauth_password(session, nullptr, ssh_password.toStdString().c_str()) != SSH_AUTH_SUCCESS) {
        std::cerr << "Error: Authentication failed." << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    std::cout << "Authenticated" << std::endl;

    ssh_channel channel = ssh_channel_new(session);
    if (!channel) {
        std::cerr << "Error: Unable to create SSH channel." << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    if (ssh_channel_open_session(channel) != SSH_OK) {
        std::cerr << "Error: Unable to open SSH channel session." << std::endl;
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    if (ssh_channel_request_exec(channel, "bash -i /home/user/run.sh") != SSH_OK) {
        std::cerr << "Error: Unable to execute remote script." << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    {
        char buffer[256];
        while (true) {
            if (!ssh_channel_is_open(channel) || ssh_channel_is_eof(channel)) {
                std::cout << "Channel closed or EOF reached." << std::endl;
                break;
            }

            int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
            if (nbytes > 0) {
                std::cout.write(buffer, nbytes);
                std::cout.flush();
            }
            else if (nbytes == SSH_ERROR) {
                std::cerr << "Error reading from SSH channel." << std::endl;
                break;
            }
            else if (nbytes == 0) {
                // No more data available
                break;
            }
        }

    }
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    std::string folder = imageFolderPath.toStdString();
    if (!downloadDirectory(session, "/home/user/dev/processed_img", folder)) {
        std::cerr << "Error: Failed to download directory." << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    std::cout << "Image downloaded" << std::endl;

    ssh_disconnect(session);
    ssh_free(session);

    std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);

    return true;
}