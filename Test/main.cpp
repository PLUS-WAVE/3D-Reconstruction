#define LEMON_SCOPE_FIX(OUTER, NESTED) typename OUTER::template NESTED

#include "SfM.hpp"
#include "Image.hpp"
#include "Match.hpp"

int main()
{

	// 1. Load images
	std::string imagesInputDir = "TestData/images";
	std::string matchesOutputDir = "TestData/images/Matches";
	std::string EigenMatrix = "2905.88;0;1416;0;2905.88;1064;0;0;1";
	
	if (LoadingImages(imagesInputDir, matchesOutputDir, EigenMatrix, "1.0;1.0;1.0") == EXIT_FAILURE)
	{
		printf("加载图片失败\n");
		return EXIT_FAILURE;
	}
	printf("加载图片成功\n");

	std::string describerMethod = "AKAZE_FLOAT";

	if (GetFeatures(matchesOutputDir + "/sfm_data.json", matchesOutputDir, describerMethod, "", true, false) == EXIT_FAILURE)
	{
		printf("获取特征信息失败\n");
		return EXIT_FAILURE;
	}
	printf("获取特征点完成\n");


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
	}
	printf("匹配完成\n");
	printf("\n任务完成\n");

	return 0;
}