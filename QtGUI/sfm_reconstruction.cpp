#include "sfm_reconstruction.h"
#include <functional>
#include <QDebug>
#include <QMessageBox>
#include<Windows.h>

bool performSFMReconstruction(const QString& cameraModel, const QString& imageFolderPath,const QString& algorithm, const QString &save,const std::function<void(const QString&)>& logCallback)
{
    logCallback("开始SFM重建...");
    Sleep(1000);
    logCallback("相机模型: " + cameraModel);
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