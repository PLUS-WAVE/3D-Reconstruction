#define _USE_EIGEN
#define _USE_CUDA
#define _USE_BOOST
#define _CRT_SECURE_NO_WARNINGS
#include <bitset>

#include "openmvs/MVS/Common.h"
#include "openmvs/MVS/Scene.h"
#include "openMVS.hpp"

using namespace MVS;

int openMVS::fnMVS(std::string MVSFilePath)
{
	Scene scene;
	if (!scene.Load(MAKE_PATH(MVSFilePath))) {
		VERBOSE("ERROR: TestDataset failed loading the scene!");
		return false;
	}

	if (!scene.DenseReconstruction() || scene.pointcloud.GetSize() < 200000u) {
		VERBOSE("ERROR: TestDataset failed estimating dense point cloud!");
		return false;
	}
	scene.pointcloud.Save(MAKE_PATH("scene_dense.ply"));
}
