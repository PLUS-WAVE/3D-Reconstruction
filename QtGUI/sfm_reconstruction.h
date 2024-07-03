#ifndef SFM_RECONSTRUCTION_H
#define SFM_RECONSTRUCTION_H

#include <QString>

bool performSFMReconstruction(const QString& cameraIntrinsics,
    const QString& imageFolderPath,
    const QString& algorithm,
    const QString& save,
    const std::function<void(const QString&)>& logCallback);
#endif // SFM_RECONSTRUCTION_H