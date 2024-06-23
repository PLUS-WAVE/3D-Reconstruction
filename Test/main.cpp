#include "SfM.hpp"
#include "Image.hpp"
#include "Match.hpp"
#include "SfP.hpp"

int main()
{

	// 1. Load images
	std::string imagesInputDir = "TestData/images";
	std::string matchesOutputDir = "TestData/images/Matches";
	std::string sfmOutputDir = matchesOutputDir + "/sfm";
	std::string EigenMatrix = "2905.88;0;1416;0;2905.88;1064;0;0;1";
	std::string describerMethod = "AKAZE_FLOAT";
	
	if (LoadingImages(imagesInputDir, matchesOutputDir, EigenMatrix, "1.0;1.0;1.0") == EXIT_FAILURE)
	{
		printf("加载图片失败\n");
		return EXIT_FAILURE;
	}
	printf("加载图片成功\n");

	if (GetFeatures(matchesOutputDir + "/sfm_data.json", matchesOutputDir, describerMethod, "", true, false) == EXIT_FAILURE)
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
		0U,
		0.8, true) == EXIT_FAILURE)
	{
		printf("匹配特征信息失败\n");
		return EXIT_FAILURE;
	}
	printf("匹配完成\n");

	if (StructureFromMotion(
		matchesOutputDir + "/sfm_data.json",
		matchesOutputDir,
		"",
		sfmOutputDir,
		"",
		"",
		"ADJUST_ALL",
		3,
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
	PrintPointColors(sfmOutputDir + "/sfm_data_local.bin", sfmOutputDir + "/colored.ply");

	printf("进行SFP重构\n");

	StructureFromPoses(
		sfmOutputDir + "/sfm_data_local.bin",
		matchesOutputDir,
		sfmOutputDir + "/robust.bin",
		matchesOutputDir + "/matches.f.bin");

	printf("进行点云上色\n");
	PrintPointColors(sfmOutputDir + "/robust.bin", sfmOutputDir + "/robust_colored.ply");

	printf("SFP重构成功\n");

	printf("Data to MVS\n");
	// ExportSparseCloud
	// (
	// 	sfmOutputDir + "/robust.bin",
	// 	sfmOutputDir + "/SparseCloud.J3D",
	// 	sfmOutputDir + "/undistorted_images",
	// 	sfmOutputDir
	// );
	
	printf("\n完成\n");

	return 0;
}