#include "SfM.hpp"
#include "Image.hpp"
#include "Match.hpp"
#include "SfP.hpp"
#include "Export2MVS.hpp"
#include "Main.hpp"

#include <Windows.h>
#include "../MVS/MVSUSE.h"

void CopyRelevantFiles(const std::string& sourceDirectory, const std::string& destinationDirectory, const std::vector<std::string>& extensions) {
	try {
		std::vector<std::string> files = stlplus::folder_files(sourceDirectory);
		for (const auto& file : files) {
			std::string fullPath = stlplus::create_filespec(sourceDirectory, file);
			std::string extension = stlplus::extension_part(file);
			for (const auto& ext : extensions) {
				if (extension == ext) {
					std::string destPath = stlplus::create_filespec(destinationDirectory, file);
					stlplus::file_copy(fullPath, destPath);
					break;
				}
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

int MAIN::start(std::string imagesInputDir, std::string kmatrix, std::string describerMethod,
                std::string finalExportFormat, int task)
{
	clock_t start, end;
	start = clock();


	std::string resultOutputDir = imagesInputDir + "/Output";
	std::string matchesOutputDir = resultOutputDir + "/Describers&Matches";
	std::string sfmOutputDir = resultOutputDir + "/SfM_Output";

	std::string MVSOutputDir = resultOutputDir + "/MVS_Output";
	std::string densifyOutputDir = MVSOutputDir + "/Densify";
	std::string reconstructMeshOutputDir = MVSOutputDir + "/Mesh";
	std::string refineMeshOutputDir = MVSOutputDir + "/RefineMesh";
	std::string textureMeshOutputDir = MVSOutputDir + "/TextureMesh";

	std::string finalExportDir = resultOutputDir + "/FinalExport";

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
	
		if (LoadingImages(imagesInputDir, matchesOutputDir, kmatrix) == EXIT_FAILURE)
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
	
		if (!stlplus::folder_exists(MVSOutputDir))
		{
			if (!stlplus::folder_create(MVSOutputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}
	
		printf("\n- Data to MVS -\n");
		Export2MVS(sfmOutputDir + "/sfp_data.bin", MVSOutputDir + "/sfm_scene.mvs", MVSOutputDir + "/undistorted_images");
	
		printf("\n完成\n");
	
		break;
	
	case 1:
		std::cout
			<< "\n-----------------------------------------------------------"
			<< "\n Densify Point Cloud:"
			<< "\n-----------------------------------------------------------"
			<< std::endl;
	
		if (!stlplus::folder_exists(densifyOutputDir))
		{
			if (!stlplus::folder_create(densifyOutputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}
	
		stlplus::folder_rename(MVSOutputDir + "/undistorted_images", densifyOutputDir + "/undistorted_images");
	
		std::string densifyInputFile = MVSOutputDir + "/sfm_scene.mvs";
		std::string densifyOutputFile = densifyOutputDir + "/scene_dense.mvs";
	
		const char* d_args[7];
	
		d_args[1] = "-i";
		d_args[2] = (char*)densifyInputFile.data();
		d_args[3] = "-o";
		d_args[4] = (char*)densifyOutputFile.data();
		d_args[5] = "-w";
		d_args[6] = (char*)densifyOutputDir.data();
		MVSUSE::DensifyPointCloud(7, d_args);
	
		std::cout
			<< "\n-----------------------------------------------------------"
			<< "\n Reconstruct Mesh:"
			<< "\n-----------------------------------------------------------"
			<< std::endl;
	
		if (!stlplus::folder_exists(reconstructMeshOutputDir))
		{
			if (!stlplus::folder_create(reconstructMeshOutputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}
	
		stlplus::folder_rename(densifyOutputDir + "/undistorted_images", reconstructMeshOutputDir + "/undistorted_images");
	
		std::string reconstructMeshInputFile = densifyOutputDir + "/scene_dense.mvs";
		std::string reconstructMeshOutputFile = reconstructMeshOutputDir + "/scene_dense_mesh.mvs";
	
		const char* m_args[7];
	
		m_args[1] = "-i";
		m_args[2] = (char*)reconstructMeshInputFile.data();
		m_args[3] = "-o";
		m_args[4] = (char*)reconstructMeshOutputFile.data();
		m_args[5] = "-w";
		m_args[6] = (char*)reconstructMeshOutputDir.data();
		MVSUSE::ReconstructMesh(7, m_args);
	
		std::cout
			<< "\n-----------------------------------------------------------"
			<< "\n Refine Mesh:"
			<< "\n-----------------------------------------------------------"
			<< std::endl;
	
		if (!stlplus::folder_exists(refineMeshOutputDir))
		{
			if (!stlplus::folder_create(refineMeshOutputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}
	
		stlplus::folder_rename(reconstructMeshOutputDir + "/undistorted_images", refineMeshOutputDir + "/undistorted_images");
	
		std::string refineMeshInputFile = reconstructMeshOutputDir + "/scene_dense_mesh.mvs";
		std::string refineMeshOutputFile = refineMeshOutputDir + "/scene_dense_mesh_refine.mvs";
	
		const char* r_args[7];
	
		r_args[1] = "-i";
		r_args[2] = (char*)refineMeshInputFile.data();
		r_args[3] = "-o";
		r_args[4] = (char*)refineMeshOutputFile.data();
		r_args[5] = "-w";
		r_args[6] = (char*)refineMeshOutputDir.data();
		MVSUSE::RefineMesh(7, r_args);
	
		std::cout
			<< "\n-----------------------------------------------------------"
			<< "\n Texture Mesh:"
			<< "\n-----------------------------------------------------------"
			<< std::endl;
	
		if (!stlplus::folder_exists(textureMeshOutputDir))
		{
			if (!stlplus::folder_create(textureMeshOutputDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}
	
		stlplus::folder_rename(refineMeshOutputDir + "/undistorted_images", textureMeshOutputDir + "/undistorted_images");
	
		std::string textureMeshInputFile = refineMeshOutputDir + "/scene_dense_mesh_refine.mvs";
		std::string textureMeshOutputFile = textureMeshOutputDir + "/scene_dense_mesh_refine_texture.mvs";
	
		const char* t_args[9];
	
		t_args[1] = "-i";
		t_args[2] = (char*)textureMeshInputFile.data();
		t_args[3] = "-o";
		t_args[4] = (char*)textureMeshOutputFile.data();
		t_args[5] = "-w";
		t_args[6] = (char*)textureMeshOutputDir.data();
		t_args[7] = "--export-type";
		t_args[8] = (char*)finalExportFormat.data();
		MVSUSE::TextureMesh(9, t_args);

		if (!stlplus::folder_exists(finalExportDir))
		{
			if (!stlplus::folder_create(finalExportDir))
			{
				printf("创建文件夹失败\n");
				return EXIT_FAILURE;
			}
		}

		std::vector<std::string> extensions = { "mtl", "jpg", finalExportFormat };
		CopyRelevantFiles(textureMeshOutputDir, finalExportDir, extensions);

		break;
	}

	end = clock();

	std::cout
		<< "\n-----------------------------------------------------------"
		<< "\n ALL DONE"
		<< "\n-----------------------------------------------------------"
		<< std::endl;

	std::cout << "\nTotal Time: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << std::endl;

	return EXIT_SUCCESS;
}

