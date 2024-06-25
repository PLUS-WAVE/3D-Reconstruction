#include "SfM.hpp"
#include "Image.hpp"
#include "Match.hpp"
#include "SfP.hpp"
#include "Export2MVS.hpp"

int main()
{
	std::string imagesInputDir = "TestData/Castle";
	std::string resultOutputDir = imagesInputDir + "/Output";
	std::string matchesOutputDir = resultOutputDir + "/Describers&Matches";
	std::string sfmOutputDir = resultOutputDir + "/SfM_Output";

	std::string EigenMatrix = "2905.88;0;1416;0;2905.88;1064;0;0;1";
	std::string describerMethod = "AKAZE_FLOAT";

	if(!stlplus::folder_exists(resultOutputDir))
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

	printf("\n- Data to MVS -\n");
	ExportSparseCloud(sfmOutputDir + "/sfp_data.bin", sfmOutputDir + "/scene.mvs", sfmOutputDir + "/undistorted_images");
	printf("\n完成\n");

	return 0;
}