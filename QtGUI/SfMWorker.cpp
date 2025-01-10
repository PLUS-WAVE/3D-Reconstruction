#include "SfMWorker.h"

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

bool SfMWorker::performSfMReconstruction(const QString& cameraIntrinsics, const QString& imageFolderPath,const QString& algorithm, const QString &save, const std::function<void(const QString&)>& logCallback)
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

	if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 0))
	{
        logCallback("SfM failed! Please check that the path does not contain Chinese characters.");
        return false;
	}
    logCallback("SfM success!");


    // 恢复原始buf
    std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);

    return true;
}