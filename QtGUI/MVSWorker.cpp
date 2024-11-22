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

bool MVSWorker::performMVSReconstruction(const QString& cameraIntrinsics, const QString& imageFolderPath, const QString& algorithm, const QString& save, QtGUI* GUI, const std::function<void(const QString&)>& logCallback)
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

    if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 1))
    {
        logCallback("MVS Densify 失败");
        return false;
    }
    logCallback("MVS Densify 成功");

    emit updateViewer(QString::fromStdString(imageFolderPath.toStdString() + "/Output/MVS_Output/Densify/scene_dense.mvs"));

    if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 2))
    {
        logCallback("MVS Reconstruct Mesh 失败");
        return false;
    }
    logCallback("MVS Reconstruct Mesh 成功");

    emit updateViewer(QString::fromStdString(imageFolderPath.toStdString() + "/Output/MVS_Output/Mesh/scene_dense_mesh.mvs"));

    if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 3))
    {
        logCallback("MVS Refine Mesh 失败");
        return false;
    }
    logCallback("MVS Refine Mesh 成功");

    emit updateViewer(QString::fromStdString(imageFolderPath.toStdString() + "/Output/MVS_Output/RefineMesh/scene_dense_mesh_refine.mvs"));

	if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 4))
    {
        logCallback("MVS Texture Mesh 失败");
        return false;
    }
    logCallback("MVS Texture Mesh 成功");

    emit updateViewer(QString::fromStdString(imageFolderPath.toStdString() + "/Output/MVS_Output/TextureMesh/scene_dense_mesh_refine_texture.mvs"));

	std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);

    return true;
}
