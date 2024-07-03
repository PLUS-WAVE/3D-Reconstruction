#include "sfm_reconstruction.h"
#include <functional>
#include <QMessageBox>
#include <Windows.h>

#include "../Main/Main.hpp"

bool performSFMReconstruction(const QString& cameraIntrinsics, const QString& imageFolderPath,const QString& algorithm, const QString &save, const std::function<void(const QString&)>& logCallback)
{
	if (MAIN::start(imageFolderPath.toStdString(), cameraIntrinsics.toStdString(), algorithm.toStdString(), save.toStdString(), 0))
	{
        logCallback("SFM重建失败");
        return false;
	}
    logCallback("SFM重建成功");
    return true;

    logCallback("开始SFM重建...");
    Sleep(1000);
    logCallback("相机模型: " + cameraIntrinsics);
    Sleep(1000);
    logCallback("图片文件夹: " + imageFolderPath);
    Sleep(1000);
    logCallback("匹配算法: " + algorithm);
    Sleep(1000);
    logCallback("保存格式: " + save);
    Sleep(1000);
    logCallback("SFM重建完成");//只要在这里使用这个函数，然后就可以在text上输出
    
    return true;
}