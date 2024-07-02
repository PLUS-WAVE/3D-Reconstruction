#pragma once
#include "openMVG/cameras/Camera_Common.hpp"
#include "openMVG/cameras/Cameras_Common_command_line_helper.hpp"
#include "openMVG/sfm/pipelines/sequential/sequential_SfM.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#include "openMVG/sfm/sfm_view.hpp"
#include "openMVG/sfm/sfm_data_colorization.hpp"
#include "openMVG/system/timer.hpp"
#include "openMVG/types.hpp"
#include "openMVG/geometry/Similarity3.hpp"
#include "openMVG/geometry/pose3.hpp"
#include "openMVG/sfm/sfm.hpp"

#include "SfMPlyHelper.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <utility>


using namespace openMVG;
using namespace openMVG::sfm;
using namespace openMVG::cameras;


bool computeIndexFromImageNames(
	const SfM_Data& sfm_data,
	const std::pair<std::string, std::string>& initialPairName,
	Pair& initialPairIndex)
{
	if (initialPairName.first == initialPairName.second)
	{
		std::cerr << "\n无效的图像名称" << std::endl;
		return false;
	}

	initialPairIndex = {UndefinedIndexT, UndefinedIndexT};

	for (Views::const_iterator it = sfm_data.GetViews().begin();
	     it != sfm_data.GetViews().end(); ++it)
	{
		const View* v = it->second.get();
		const std::string filename = stlplus::filename_part(v->s_Img_path);
		if (filename == initialPairName.first)
		{
			initialPairIndex.first = v->id_view;
		}
		else
		{
			if (filename == initialPairName.second)
			{
				initialPairIndex.second = v->id_view;
			}
		}
	}
	return (initialPairIndex.first != UndefinedIndexT &&
		initialPairIndex.second != UndefinedIndexT);
}


void GetCameraPositions(const SfM_Data& sfm_data, std::vector<Vec3>& vec_camPosition)
{
	for (const auto& view : sfm_data.GetViews())
	{
		if (sfm_data.IsPoseAndIntrinsicDefined(view.second.get()))
		{
			const geometry::Pose3 pose = sfm_data.GetPoseOrDie(view.second.get());
			vec_camPosition.push_back(pose.center());
		}
	}
}


int StructureFromMotion(
	std::string SFMDataFilename,
	std::string MatchesDataPath,
	std::string MatchOutputName,
	std::string OutputDataPath = "",
	std::string initialPairString_first = "",
	std::string initialPairString_second = "",
	std::string sIntrinsic_refinement_options = "ADJUST_ALL",
	std::string sExtrinsic_refinement_options = "ADJUST_ALL",
	int i_User_camera_model = PINHOLE_CAMERA_RADIAL3,
	bool b_use_motion_priors = false,
	bool prior_usage = false,
	int triangulation_method = static_cast<int>(ETriangulationMethod::DEFAULT),
	int resection_method = static_cast<int>(resection::SolverType::DEFAULT)
)
{
	std::pair<std::string, std::string> initialPairString(initialPairString_first, initialPairString_second);


	if (!isValid(static_cast<ETriangulationMethod>(triangulation_method)))
	{
		std::cerr << "\n无效的三角测量方法" << std::endl;
		return EXIT_FAILURE;
	}

	if (!isValid(openMVG::cameras::EINTRINSIC(i_User_camera_model)))
	{
		std::cerr << "\n无效的相机参数" << std::endl;
		return EXIT_FAILURE;
	}

	const cameras::Intrinsic_Parameter_Type intrinsic_refinement_options =
		cameras::StringTo_Intrinsic_Parameter_Type(sIntrinsic_refinement_options);
	if (intrinsic_refinement_options == static_cast<cameras::Intrinsic_Parameter_Type>(0))
	{
		std::cerr << "\n相机内参数优化错误" << std::endl;
		return EXIT_FAILURE;
	}

	const sfm::Extrinsic_Parameter_Type extrinsic_refinement_options =
		sfm::StringTo_Extrinsic_Parameter_Type(sExtrinsic_refinement_options);
	if (extrinsic_refinement_options == static_cast<sfm::Extrinsic_Parameter_Type>(0))
	{
		std::cerr << "\n相机外参数优化错误" << std::endl;
		return EXIT_FAILURE;
	}

	SfM_Data sfm_data;
	if (!Load(sfm_data, SFMDataFilename, ESfM_Data(VIEWS | INTRINSICS)))
	{
		std::cerr << "\nSFMData" << SFMDataFilename << "无法读取" << std::endl;
		return EXIT_FAILURE;
	}

	using namespace openMVG::features;
	const std::string sImage_describer = stlplus::create_filespec(MatchesDataPath, "image_describer", "json");
	std::unique_ptr<Regions> regions_type = Init_region_type_from_file(sImage_describer);
	if (!regions_type)
	{
		std::cerr << "\n无效的特征文件: " << sImage_describer << std::endl;
		return EXIT_FAILURE;
	}

	std::shared_ptr<Features_Provider> feats_provider = std::make_shared<Features_Provider>();
	if (!feats_provider->load(sfm_data, MatchesDataPath, regions_type))
	{
		std::cerr << "\n无效的特征数据" << std::endl;
		return EXIT_FAILURE;
	}

	std::shared_ptr<Matches_Provider> matches_provider = std::make_shared<Matches_Provider>();
	if
	(
		!(matches_provider->load(sfm_data, MatchOutputName) ||
			matches_provider->load(sfm_data, stlplus::create_filespec(MatchesDataPath, "matches.f.txt")) ||
			matches_provider->load(sfm_data, stlplus::create_filespec(MatchesDataPath, "matches.f.bin")))
	)
	{
		std::cerr << "\n无效的匹配数据" << std::endl;
		return EXIT_FAILURE;
	}

	if (OutputDataPath.empty())
	{
		std::cerr << "\n无效的输出路径" << std::endl;
		return EXIT_FAILURE;
	}

	if (!stlplus::folder_exists(OutputDataPath))
	{
		if (!stlplus::folder_create(OutputDataPath))
		{
			std::cerr << "\n无法创建输出目录" << std::endl;
		}
	}

	openMVG::system::Timer timer;

	stlplus::folder_create(OutputDataPath + "/SfM_LogData");
	SequentialSfMReconstructionEngine sfmEngine(sfm_data, OutputDataPath + "/SfM_LogData",
	                                            stlplus::create_filespec(OutputDataPath + "/SfM_LogData", "SfM_log.html"));

	sfmEngine.SetFeaturesProvider(feats_provider.get());
	sfmEngine.SetMatchesProvider(matches_provider.get());

	sfmEngine.Set_Intrinsics_Refinement_Type(intrinsic_refinement_options);
	sfmEngine.Set_Extrinsics_Refinement_Type(extrinsic_refinement_options);

	sfmEngine.SetUnknownCameraType(EINTRINSIC(i_User_camera_model));
	b_use_motion_priors = prior_usage;
	sfmEngine.Set_Use_Motion_Prior(b_use_motion_priors);
	sfmEngine.SetTriangulationMethod(static_cast<ETriangulationMethod>(triangulation_method));
	sfmEngine.SetResectionMethod(static_cast<resection::SolverType>(resection_method));

	if (!initialPairString.first.empty() && !initialPairString.second.empty())
	{
		Pair initialPairIndex;
		if (!computeIndexFromImageNames(sfm_data, initialPairString, initialPairIndex))
		{
			std::cerr << "无法进行匹配初始化 <" << initialPairString.first << ", " << initialPairString.second << ">!\n";
			return EXIT_FAILURE;
		}
		sfmEngine.setInitialPair(initialPairIndex);
	}

	if (!sfmEngine.Process())
	{
		std::cerr << "\nSfM Process失败" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "\nSfM总耗时(秒): " << timer.elapsed() << std::endl;

	Save(sfmEngine.Get_SfM_Data(),
	     stlplus::create_filespec(OutputDataPath, "sfm_data", ".bin"),
	     ESfM_Data(ALL));

	Save(sfmEngine.Get_SfM_Data(),
	     stlplus::create_filespec(OutputDataPath, "sfm_data", ".ply"),
	     ESfM_Data(ALL));

	return EXIT_SUCCESS;
}


int PrintPointColors(
	std::string InputFile,
	std::string OutputFile
)
{
	if (OutputFile.empty())
	{
		std::cerr << "\n没有指定输出文件名" << std::endl;
		return EXIT_FAILURE;
	}

	SfM_Data sfm_data;
	if (!Load(sfm_data, InputFile, ESfM_Data(ALL)))
	{
		std::cerr << "\n输入的SfM Data文件 \"" << InputFile << "\" 无法读取" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<Vec3> vec_3dPoints, vec_tracksColor, vec_camPosition;
	if (ColorizeTracks(sfm_data, vec_3dPoints, vec_tracksColor))
	{
		GetCameraPositions(sfm_data, vec_camPosition);

		if (plyHelper::exportToPly(vec_3dPoints, vec_camPosition, OutputFile, &vec_tracksColor))
		{
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}

int ConvertCoorsToOrigin
(
	std::string sSfM_Data_Filename_In,
	std::string sOutDir
)
{
	Vec3 local_Frame_Origin;
	bool b_first_frame_origin;

	SfM_Data sfm_data;
	if (!Load(sfm_data, sSfM_Data_Filename_In, ESfM_Data(ALL)))
	{
		std::cerr << std::endl << "未能找到SfM数据文件 \"" << sSfM_Data_Filename_In << "\"" << std::endl;
		return EXIT_FAILURE;
	}

	if (sfm_data.poses.empty())
	{
		std::cerr << "该重建场景没有相机姿态数据" << std::endl;
		return EXIT_FAILURE;
	}
	local_Frame_Origin = (sfm_data.poses.cbegin()->second).center();

	Similarity3 sim(Pose3(Mat3::Identity(), local_Frame_Origin), 1.0);

	const bool b_transform_priors = true;
	ApplySimilarity(sim, sfm_data, b_transform_priors);

	if (!Save(sfm_data,
	          stlplus::create_filespec(sOutDir, "sfm_data_local", ".bin"),
	          ESfM_Data(ALL))
		|| !Save(sfm_data,
		         stlplus::create_filespec(sOutDir, "sfm_data_local", ".ply"),
		         ESfM_Data(ALL)))
	{
		std::cerr << "无法保存" << std::endl;
	}

	return EXIT_SUCCESS;
}
