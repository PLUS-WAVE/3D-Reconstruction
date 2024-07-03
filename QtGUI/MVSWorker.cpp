#include "MVSWorker.h"

#include "../Main/Main.hpp"

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

bool MVSWorker::performMVSReconstruction(const QString& cameraIntrinsics, const QString& imageFolderPath, const QString& algorithm, const QString& save, const std::function<void(const QString&)>& logCallback)
{
    auto coutBuf = std::cout.rdbuf();
    auto cerrBuf = std::cerr.rdbuf();

    StreamBuffer customBuf;
    customBuf.outputCallback = [&logCallback](const std::string& text) {
        static QStringDecoder decoder(QStringDecoder::Encoding::System);
        QString decodedText = decoder(text);
        emit logCallback(decodedText);
        };

    std::cout.rdbuf(&customBuf);
    std::cerr.rdbuf(&customBuf);

    if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 1))
    {
        logCallback("MVS 失败");
        return false;
    }
    logCallback("MVS 成功");


    // �ָ�ԭʼbuf
    std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);

    return true;
}
