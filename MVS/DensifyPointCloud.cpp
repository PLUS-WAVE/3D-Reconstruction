#include "openmvs/MVS/Common.h"
#include "openmvs/MVS/Scene.h"
#include <boost/program_options.hpp>
#include "MVSUSE.h"
#include <direct.h>
#include <cstdio>

using namespace MVS;
#define APPNAME _T("DensifyPointCloud")
namespace OPT {
	String strInputFileName;
	String strOutputFileName;
	String strMeshFileName;
	String strDenseConfigFileName;
	int nFusionMode;
	int nArchiveType;
	int nProcessPriority;
	unsigned nMaxThreads;
	String strConfigFileName;
	boost::program_options::variables_map vm;
} // namespace OPT

int getFiles(const char* path, std::vector<std::string>& arr)
{
	int num_of_img = 0;
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	char p[700] = { 0 };
	strcpy(p, path);
	strcat(p, "\\*");
	char buf[256];
	if ((hFile = _findfirst(p, &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					continue;
			}
			else
			{
				strcpy(buf, path);
				strcat(buf, "\\");
				strcat(buf, fileinfo.name);
				struct stat st1;
				stat(buf, &st1);
				arr.push_back(buf);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return num_of_img;
}


// Initialize_Dense and parse the command line parameters
bool Initialize_Dense(size_t argc, LPCTSTR* argv)
{
	// Initialize_Dense log and console
	CLOSE_LOGFILE();
	CLOSE_LOGCONSOLE();
	CLOSE_LOG();
	OPEN_LOG();
	OPEN_LOGCONSOLE();

	// group of options allowed only on command line
	boost::program_options::options_description generic("Generic options");
	generic.add_options()
		("help,h", "produce this help message")
		("working-folder,w", boost::program_options::value<std::string>(&WORKING_FOLDER), "working directory (default current directory)")
		("config-file,c", boost::program_options::value<std::string>(&OPT::strConfigFileName)->default_value(APPNAME _T(".cfg")), "file name containing program options")
		("archive-type", boost::program_options::value(&OPT::nArchiveType)->default_value(2), "project archive type: 0-text, 1-binary, 2-compressed binary")
		("process-priority", boost::program_options::value(&OPT::nProcessPriority)->default_value(-1), "process priority (below normal by default)")
		("max-threads", boost::program_options::value(&OPT::nMaxThreads)->default_value(0), "maximum number of threads (0 for using all available cores)")
#if TD_VERBOSE != TD_VERBOSE_OFF
		("verbosity,v", boost::program_options::value(&g_nVerbosityLevel)->default_value(
#if TD_VERBOSE == TD_VERBOSE_DEBUG
			3
#else
			2
#endif
		), "verbosity level")
#endif
		;

	// group of options allowed both on command line and in config file
	unsigned nResolutionLevel;
	unsigned nMaxResolution;
	unsigned nMinResolution;
	unsigned nNumViews;
	unsigned nMinViewsFuse;
	unsigned nOptimize;
	unsigned nEstimateColors;
	unsigned nEstimateNormals;
	boost::program_options::options_description config("Densify options");
	config.add_options()
		("input-file,i", boost::program_options::value<std::string>(&OPT::strInputFileName), "input filename containing camera poses and image list")
		("output-file,o", boost::program_options::value<std::string>(&OPT::strOutputFileName), "output filename for storing the dense point-cloud")
		("resolution-level", boost::program_options::value(&nResolutionLevel)->default_value(1), "how many times to scale down the images before point cloud computation")
		("max-resolution", boost::program_options::value(&nMaxResolution)->default_value(3200), "do not scale images higher than this resolution")
		("min-resolution", boost::program_options::value(&nMinResolution)->default_value(640), "do not scale images lower than this resolution")
		("number-views", boost::program_options::value(&nNumViews)->default_value(5), "number of views used for depth-map estimation (0 - all neighbor views available)")
		("number-views-fuse", boost::program_options::value(&nMinViewsFuse)->default_value(3), "minimum number of images that agrees with an estimate during fusion in order to consider it inlier")
		("optimize", boost::program_options::value(&nOptimize)->default_value(7), "filter used after depth-map estimation (0 - disabled, 1 - remove speckles, 2 - fill gaps, 4 - cross-adjust)")
		("estimate-colors", boost::program_options::value(&nEstimateColors)->default_value(2), "estimate the colors for the dense point-cloud")
		("estimate-normals", boost::program_options::value(&nEstimateNormals)->default_value(0), "estimate the normals for the dense point-cloud")
		// ("sample-mesh", boost::program_options::value(&OPT::fSampleMesh)->default_value(0.f), "uniformly samples points on a mesh (0 - disabled, <0 - number of points, >0 - sample density per square unit)")
		// ("filter-point-cloud", boost::program_options::value(&OPT::thFilterPointCloud)->default_value(0), "filter dense point-cloud based on visibility (0 - disabled)")
		("fusion-mode", boost::program_options::value(&OPT::nFusionMode)->default_value(0), "depth map fusion mode (-2 - fuse disparity-maps, -1 - export disparity-maps only, 0 - depth-maps & fusion, 1 - export depth-maps only)")
		;

	// hidden options, allowed both on command line and
	// in config file, but will not be shown to the user
	boost::program_options::options_description hidden("Hidden options");
	hidden.add_options()
		("dense-config-file", boost::program_options::value<std::string>(&OPT::strDenseConfigFileName), "optional configuration file for the densifier (overwritten by the command line options)")
		;

	boost::program_options::options_description cmdline_options;
	cmdline_options.add(generic).add(config).add(hidden);

	boost::program_options::options_description config_file_options;
	config_file_options.add(config).add(hidden);

	boost::program_options::positional_options_description p;
	p.add("input-file", -1);

	try {
		// parse command line options
		boost::program_options::store(boost::program_options::command_line_parser((int)argc, argv).options(cmdline_options).positional(p).run(), OPT::vm);
		boost::program_options::notify(OPT::vm);
		INIT_WORKING_FOLDER;
		// parse configuration file
		std::ifstream ifs(MAKE_PATH_SAFE(OPT::strConfigFileName));
		if (ifs) {
			boost::program_options::store(parse_config_file(ifs, config_file_options), OPT::vm);
			boost::program_options::notify(OPT::vm);
		}
	}
	catch (const std::exception& e) {
		LOG(e.what());
		return false;
	}

	// Initialize_Dense the log file
	OPEN_LOGFILE(MAKE_PATH(APPNAME _T("-") + Util::getUniqueName(0) + _T(".log")).c_str());

	// validate input
	Util::ensureValidPath(OPT::strInputFileName);
	Util::ensureUnifySlash(OPT::strInputFileName);
	if (OPT::vm.count("help") || OPT::strInputFileName.IsEmpty()) {
		boost::program_options::options_description visible("Available options");
		visible.add(generic).add(config);
		GET_LOG() << visible;
	}
	if (OPT::strInputFileName.IsEmpty())
		return false;

	// Initialize_Dense optional options
	Util::ensureValidPath(OPT::strOutputFileName);
	Util::ensureUnifySlash(OPT::strOutputFileName);
	if (OPT::strOutputFileName.IsEmpty())
		OPT::strOutputFileName = Util::getFileFullName(OPT::strInputFileName) + _T("_dense.mvs");

	// init dense options
	OPTDENSE::init();
	const bool bValidConfig(OPTDENSE::oConfig.Load(OPT::strDenseConfigFileName));
	OPTDENSE::update();
	OPTDENSE::nResolutionLevel = nResolutionLevel;
	OPTDENSE::nMaxResolution = nMaxResolution;
	OPTDENSE::nMinResolution = nMinResolution;
	OPTDENSE::nNumViews = nNumViews;
	OPTDENSE::nMinViewsFuse = nMinViewsFuse;
	OPTDENSE::nOptimize = nOptimize;
	OPTDENSE::nEstimateColors = nEstimateColors;
	OPTDENSE::nEstimateNormals = nEstimateNormals;
	if (!bValidConfig && !OPT::strDenseConfigFileName.IsEmpty())
		OPTDENSE::oConfig.Save(OPT::strDenseConfigFileName);

	// Initialize_Dense global options
	Process::setCurrentProcessPriority((Process::Priority)OPT::nProcessPriority);
#ifdef _USE_OPENMP
	if (OPT::nMaxThreads != 0)
		omp_set_num_threads(OPT::nMaxThreads);
#endif

	Util::Init();
	return true;
}

void cleanCacheFiles(const std::string& workDir)
{
	std::vector<std::string> fileNames;
	getFiles(workDir.c_str(), fileNames);
	for (int i = 0; i < fileNames.size(); i++)
	{
		std::string& fn = fileNames[i];
		auto pos = fn.rfind('.');
		if (pos != fn.npos)
		{
			std::string ext = fn.substr(pos + 1, fn.size());
			if (ext == "dmap")
			{
				remove(fn.c_str());
			}
		}

	}
}

int MVSUSE::DensifyPointCloud(int agrs_num, const char* d_args[])
{
	if (!Initialize_Dense(agrs_num, d_args))
	{
		VERBOSE("error: failed to initialize");
		return EXIT_FAILURE;
	}

	Scene scene(OPT::nMaxThreads);

	// load and estimate a dense point-cloud
	if (!scene.Load(OPT::strInputFileName))
	{
		VERBOSE("error: failed to load input scene");
		return EXIT_FAILURE;
	}
	if (scene.pointcloud.IsEmpty()) {
		VERBOSE("error: empty initial point-cloud");
		return EXIT_FAILURE;
	}

	if ((ARCHIVE_TYPE)OPT::nArchiveType != ARCHIVE_MVS) {
		TD_TIMER_START();
		if (!scene.DenseReconstruction(OPT::nFusionMode)) 
		{
			VERBOSE("error: dense reconstruction failed");
			return EXIT_FAILURE;
		}
		VERBOSE("稠密点云生成完成: %u points (%s)", scene.pointcloud.GetSize(), TD_TIMER_GET_FMT().c_str());
	}

	// save the final result
	const String baseFileName(Util::getFileFullName(OPT::strOutputFileName));
	scene.Save(baseFileName + _T(".mvs"), (ARCHIVE_TYPE)OPT::nArchiveType);
	scene.pointcloud.Save(baseFileName + _T(".ply"));

	cleanCacheFiles(d_args[6]);
	return EXIT_SUCCESS;
}
