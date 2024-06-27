#include "SfM.hpp"
#include "Image.hpp"
#include "Match.hpp"
#include "SfP.hpp"
#include "Export2MVS.hpp"

#include <Windows.h>
#include "../MVS/MVSEngine.h"

int main()
{
	std::string imagesInputDir = "data/Castle";
	std::string resultOutputDir = imagesInputDir + "/Output";
	std::string matchesOutputDir = resultOutputDir + "/Describers&Matches";
	std::string sfmOutputDir = resultOutputDir + "/SfM_Output";

	std::string EigenMatrix = "2905.88;0;1416;0;2905.88;1064;0;0;1";
	std::string describerMethod = "AKAZE_FLOAT";

	std::string MVSdensifyInputDir = resultOutputDir + "/MVS_Output";
	std::string densifyWorkingDir = MVSdensifyInputDir + "/Densify";
	std::string densifyOutputDir = MVSdensifyInputDir + "/Densify";

	std::string reconstructMeshOutputDir = MVSdensifyInputDir + "/Mesh";
	std::string reconstructMeshWorkingDir = MVSdensifyInputDir + "/Mesh";

	std::string refineMeshOutputDir = MVSdensifyInputDir + "/RefineMesh";
	std::string refineMeshWorkingDir = MVSdensifyInputDir + "/RefineMesh";

	int task = 1;
	switch (task)
	{
	case 0:
		if (!stlplus::folder_exists(resultOutputDir))
		{
			if (!stlplus::folder_create(resultOutputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}

		if (LoadingImages(imagesInputDir, matchesOutputDir, EigenMatrix) == EXIT_FAILURE)
		{
			printf("加载图片失败\n");
			return EXIT_FAILURE;
		}
		printf("加载图片成功\n");

		if (GetFeatures(matchesOutputDir + "/sfm_data.json",
		                matchesOutputDir,
		                describerMethod,
		                "",
		                true,
		                false) == EXIT_FAILURE)
		{
			printf("获取特征信息失败\n");
			return EXIT_FAILURE;
		}
		printf("获取特征点完成\n\n");


		if (GetMatches(
			matchesOutputDir + "/sfm_data.json",
			matchesOutputDir,
			"f",
			"",
			"AUTO",
			-1,
			2048,
			0,
			0.8,
			false) == EXIT_FAILURE)
		{
			printf("匹配特征信息失败\n");
			return EXIT_FAILURE;
		}
		printf("匹配完成\n\n");

		std::cout
			<< "\n-----------------------------------------------------------"
			<< "\n Structure from Motion:"
			<< "\n-----------------------------------------------------------"
			<< std::endl;

		if (StructureFromMotion(
			matchesOutputDir + "/sfm_data.json",
			matchesOutputDir,
			"matches.f.bin", // GetMatches 中 sGeometricModel = "f"
			sfmOutputDir,
			"",
			"",
			"ADJUST_ALL",
			"ADJUST_ALL",
			PINHOLE_CAMERA_RADIAL3,
			true,
			true))
		{
			printf("SfM失败\n");
			return EXIT_FAILURE;
		}
		printf("SfM完成\n");


		ConvertCoorsToOrigin
		(
			sfmOutputDir + "/sfm_data.bin",
			sfmOutputDir
		);

		printf("进行点云上色\n");
		PrintPointColors(sfmOutputDir + "/sfm_data_local.bin", sfmOutputDir + "/sfm_data_local_colored.ply");

		std::cout
			<< "\n-----------------------------------------------------------"
			<< "\n Compute Structure From Known Poses:"
			<< "\n-----------------------------------------------------------"
			<< std::endl;

		StructureFromPoses(
			sfmOutputDir + "/sfm_data_local.bin",
			matchesOutputDir,
			sfmOutputDir + "/sfp_data.bin",
			matchesOutputDir + "/matches.f.bin");

		printf("\n进行点云上色\n");
		PrintPointColors(sfmOutputDir + "/sfp_data.bin", sfmOutputDir + "/sfp_data_colored.ply");

		if (!stlplus::folder_exists(MVSdensifyInputDir))
		{
			if (!stlplus::folder_create(MVSdensifyInputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}

		printf("\n- Data to MVS -\n");
		Export2MVS(sfmOutputDir + "/sfp_data.bin", MVSdensifyInputDir + "/sfm_scene.mvs", MVSdensifyInputDir + "/undistorted_images");

		if (!stlplus::folder_exists(densifyWorkingDir))
		{
			if (!stlplus::folder_create(densifyWorkingDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}

		stlplus::folder_rename(MVSdensifyInputDir + "/undistorted_images", densifyWorkingDir + "/undistorted_images");
		printf("\n完成\n");

		break;

	case 1:
		char* cmd[7];
		char t[200];

		std::string densifyInputFile = MVSdensifyInputDir + "/sfm_scene.mvs";
		std::string densifyOutputFile = densifyOutputDir + "/scene_dense.mvs";

		cmd[0] = t;
		cmd[1] = "-i";
		cmd[2] = (char*)densifyInputFile.data();
		cmd[3] = "-w";
		cmd[4] = (char*)densifyWorkingDir.data();
		cmd[5] = "-o";
		cmd[6] = (char*)densifyOutputFile.data();
		// MVSEngine::DensifyPointCloud(7, cmd);


		if (!stlplus::folder_exists(reconstructMeshWorkingDir))
		{
			if (!stlplus::folder_create(reconstructMeshWorkingDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}

		stlplus::folder_rename(densifyWorkingDir + "/undistorted_images", reconstructMeshWorkingDir + "/undistorted_images");

		std::string reconstructMeshInputFile = densifyWorkingDir + "/scene_dense.mvs";
		std::string reconstructMeshOutputFile = reconstructMeshOutputDir + "/scene_dense_mesh.mvs";

		char* cmd1[9];
		char t1[200];

		cmd1[0] = t1;
		cmd1[1] = "-i";
		cmd1[2] = (char*)reconstructMeshInputFile.data();
		cmd1[3] = "-d";
		cmd1[4] = "2.5";
		cmd1[5] = "-o";
		cmd1[6] = (char*)reconstructMeshOutputFile.data();
		cmd1[7] = "-w";
		cmd1[8] = (char*)reconstructMeshWorkingDir.data();
		// MVSEngine::ReconstructMesh(9, cmd1);


		if (!stlplus::folder_exists(refineMeshWorkingDir))
		{
			if (!stlplus::folder_create(refineMeshWorkingDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}

		stlplus::folder_rename(reconstructMeshWorkingDir + "/undistorted_images", refineMeshWorkingDir + "/undistorted_images");

		std::string refineMeshInputFile = reconstructMeshWorkingDir + "/scene_dense_mesh.mvs";
		std::string refineMeshOutputFile = refineMeshOutputDir + "/scene_dense_mesh_refine.mvs";

		char* cmd2[9];
		char t2[200];

		cmd2[0] = t2;
		cmd2[1] = "-i";
		cmd2[2] = (char*)refineMeshInputFile.data();
		cmd2[3] = "--resolution-level";
		cmd2[4] = "0";
		cmd2[5] = "-o";
		cmd2[6] = (char*)refineMeshOutputFile.data();
		cmd2[7] = "-w";
		cmd2[8] = (char*)refineMeshWorkingDir.data();
		MVSEngine::RefineMesh(9, cmd2);
		

		break;
	}

	return 0;
}
