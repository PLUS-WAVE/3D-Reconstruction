#include "SfMWorker.h"

#include "../Main/Main.hpp"

bool SfMWorker::performSfMReconstruction(const QString& cameraIntrinsics, const QString& imageFolderPath,const QString& algorithm, const QString &save, const std::function<void(const QString&)>& logCallback)
{
    // 保存原始buf
    auto coutBuf = std::cout.rdbuf();
    auto cerrBuf = std::cerr.rdbuf();

    StreamBuffer customBuf;
    customBuf.outputCallback = [&logCallback](const std::string& text) {
        // 使用QStringDecoder进行GB2312到UTF-16的转换
        static QStringDecoder decoder(QStringDecoder::Encoding::System);
        QString decodedText = decoder(text);

        // 现在decodedText是以UTF-16编码的QString，可以正确显示中文字符
        emit logCallback(decodedText);
        };

    // 重定向
    std::cout.rdbuf(&customBuf);
    std::cerr.rdbuf(&customBuf);

	if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 0))
	{
        logCallback("SfM重建失败");
        return false;
	}
    logCallback("SfM重建成功");


    // 恢复原始buf
    std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);

    return true;
}