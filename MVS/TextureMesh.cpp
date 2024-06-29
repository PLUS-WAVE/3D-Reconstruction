#include "openmvs/MVS/Common.h"
#include "openmvs/MVS/Scene.h"
#include <boost/program_options.hpp>
#include "MVSUSE.h"
using namespace MVS;
#define APPNAME _T("TextureMesh")
namespace OPT_TextureMesh {
	String strInputFileName;
	String strOutputFileName;
	unsigned nCloseHoles;
	unsigned nResolutionLevel;
	unsigned nMinResolution;
	unsigned minCommonCameras;
	float fOutlierThreshold;
	float fRatioDataSmoothness;
	bool bGlobalSeamLeveling;
	bool bLocalSeamLeveling;
	unsigned nTextureSizeMultiple;
	unsigned nRectPackingHeuristic;
	uint32_t nColEmpty;
	float fSharpnessWeight;
	unsigned nArchiveType;
	int nProcessPriority;
	unsigned nMaxThreads;
	String strExportType;
	String strConfigFileName;
	boost::program_options::variables_map vm;
} // namespace OPT_TextureMesh

// Initialize_TextureMesh and parse the command line parameters
bool Initialize_TextureMesh(size_t argc, LPCTSTR* argv)
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
		("config-file,c", boost::program_options::value<std::string>(&OPT_TextureMesh::strConfigFileName)->default_value(APPNAME _T(".cfg")), "file name containing program options")
		("export-type", boost::program_options::value<std::string>(&OPT_TextureMesh::strExportType)->default_value(_T("ply")), "file type used to export the 3D scene (ply or obj)")
		("archive-type", boost::program_options::value<unsigned>(&OPT_TextureMesh::nArchiveType)->default_value(2), "project archive type: 0-text, 1-binary, 2-compressed binary")
		("process-priority", boost::program_options::value<int>(&OPT_TextureMesh::nProcessPriority)->default_value(-1), "process priority (below normal by default)")
		("max-threads", boost::program_options::value<unsigned>(&OPT_TextureMesh::nMaxThreads)->default_value(0), "maximum number of threads (0 for using all available cores)")
#if TD_VERBOSE != TD_VERBOSE_OFF
		("verbosity,v", boost::program_options::value<int>(&g_nVerbosityLevel)->default_value(
#if TD_VERBOSE == TD_VERBOSE_DEBUG
			3
#else
			2
#endif
		), "verbosity level")
#endif
		;

	// group of options allowed both on command line and in config file
	boost::program_options::options_description config("Texture options");
	config.add_options()
		("input-file,i", boost::program_options::value<std::string>(&OPT_TextureMesh::strInputFileName), "input filename containing camera poses and image list")
		("output-file,o", boost::program_options::value<std::string>(&OPT_TextureMesh::strOutputFileName), "output filename for storing the mesh")
		//("decimate", boost::program_options::value<float>(&OPT_TextureMesh::fDecimateMesh)->default_value(1.f), "decimation factor in range [0..1] to be applied to the input surface before refinement (0 - auto, 1 - disabled)")
		("close-holes", boost::program_options::value<unsigned>(&OPT_TextureMesh::nCloseHoles)->default_value(30), "try to close small holes in the input surface (0 - disabled)")
		("resolution-level", boost::program_options::value<unsigned>(&OPT_TextureMesh::nResolutionLevel)->default_value(0), "how many times to scale down the images before mesh refinement")
		("min-resolution", boost::program_options::value<unsigned>(&OPT_TextureMesh::nMinResolution)->default_value(640), "do not scale images lower than this resolution")
		("outlier-threshold", boost::program_options::value<float>(&OPT_TextureMesh::fOutlierThreshold)->default_value(6e-2f), "threshold used to find and remove outlier face textures (0 - disabled)")
		("cost-smoothness-ratio", boost::program_options::value<float>(&OPT_TextureMesh::fRatioDataSmoothness)->default_value(0.1f), "ratio used to adjust the preference for more compact patches (1 - best quality/worst compactness, ~0 - worst quality/best compactness)")
		("virtual-face-images", boost::program_options::value(&OPT_TextureMesh::minCommonCameras)->default_value(0), "generate texture patches using virtual faces composed of coplanar triangles sharing at least this number of views (0 - disabled, 3 - good value)")
		("global-seam-leveling", boost::program_options::value<bool>(&OPT_TextureMesh::bGlobalSeamLeveling)->default_value(true), "generate uniform texture patches using global seam leveling")
		("local-seam-leveling", boost::program_options::value<bool>(&OPT_TextureMesh::bLocalSeamLeveling)->default_value(true), "generate uniform texture patch borders using local seam leveling")
		("texture-size-multiple", boost::program_options::value<unsigned>(&OPT_TextureMesh::nTextureSizeMultiple)->default_value(0), "texture size should be a multiple of this value (0 - power of two)")
		("patch-packing-heuristic", boost::program_options::value<unsigned>(&OPT_TextureMesh::nRectPackingHeuristic)->default_value(3), "specify the heuristic used when deciding where to place a new patch (0 - best fit, 3 - good speed, 100 - best speed)")
		("empty-color", boost::program_options::value<uint32_t>(&OPT_TextureMesh::nColEmpty)->default_value(0x00696969), "color used for faces not covered by any image")
		("sharpness-weight", boost::program_options::value(&OPT_TextureMesh::fSharpnessWeight)->default_value(0.5f), "amount of sharpness to be applied on the texture (0 - disabled)")
		// ("orthographic-image-resolution", boost::program_options::value<unsigned>(&OPT_TextureMesh::nOrthoMapResolution)->default_value(0), "orthographic image resolution to be generated from the textured mesh - the mesh is expected to be already geo-referenced or at least properly oriented (0 - disabled)")
		;

	// hidden options, allowed both on command line and
	// in config file, but will not be shown to the user
	boost::program_options::options_description hidden("Hidden options");
	hidden.add_options()
		// ("mesh-file", boost::program_options::value<std::string>(&OPT_TextureMesh::strMeshFileName), "mesh file name to texture (overwrite the existing mesh)")
		;

	boost::program_options::options_description cmdline_options;
	cmdline_options.add(generic).add(config).add(hidden);

	boost::program_options::options_description config_file_options;
	config_file_options.add(config).add(hidden);

	boost::program_options::positional_options_description p;
	p.add("input-file", -1);

	try {

		boost::program_options::store(boost::program_options::command_line_parser((int)argc, argv).options(cmdline_options).positional(p).run(), OPT_TextureMesh::vm);
		boost::program_options::notify(OPT_TextureMesh::vm);
		INIT_WORKING_FOLDER;
		// parse configuration file
		std::ifstream ifs(MAKE_PATH_SAFE(OPT_TextureMesh::strConfigFileName));
		if (ifs) {
			boost::program_options::store(parse_config_file(ifs, config_file_options), OPT_TextureMesh::vm);
			boost::program_options::notify(OPT_TextureMesh::vm);
		}
	}
	catch (const std::exception& e) {
		LOG(e.what());
		return false;
	}


	OPEN_LOGFILE(MAKE_PATH(APPNAME _T("-") + Util::getUniqueName(0) + _T(".log")).c_str());


	// validate input
	Util::ensureValidPath(OPT_TextureMesh::strInputFileName);
	Util::ensureUnifySlash(OPT_TextureMesh::strInputFileName);
	if (OPT_TextureMesh::vm.count("help") || OPT_TextureMesh::strInputFileName.IsEmpty()) {
		boost::program_options::options_description visible("Available options");
		visible.add(generic).add(config);
		GET_LOG() << visible;
	}
	if (OPT_TextureMesh::strInputFileName.IsEmpty())
		return false;
	OPT_TextureMesh::strExportType = OPT_TextureMesh::strExportType.ToLower() == _T("obj") ? _T(".obj") : _T(".ply");


	Util::ensureValidPath(OPT_TextureMesh::strOutputFileName);
	Util::ensureUnifySlash(OPT_TextureMesh::strOutputFileName);
	if (OPT_TextureMesh::strOutputFileName.IsEmpty())
		OPT_TextureMesh::strOutputFileName = Util::getFileFullName(OPT_TextureMesh::strInputFileName) + _T("_texture.mvs");

	Process::setCurrentProcessPriority((Process::Priority)OPT_TextureMesh::nProcessPriority);
#ifdef _USE_OPENMP
	if (OPT_TextureMesh::nMaxThreads != 0)
		omp_set_num_threads(OPT_TextureMesh::nMaxThreads);
#endif

#ifdef _USE_BREAKPAD
	// start memory dumper
	MiniDumper::Create(APPNAME, WORKING_FOLDER);
#endif

	Util::Init();
	return true;
}

int MVSUSE::TextureMesh(int agrs_num, const char* t_args[])
{
	if (!Initialize_TextureMesh(agrs_num, t_args))
	{
		VERBOSE("error: failed to initialize texture mesh");
		return EXIT_FAILURE;
	}

	Scene scene(OPT_TextureMesh::nMaxThreads);
	// load and texture the mesh
	if (!scene.Load(OPT_TextureMesh::strInputFileName))
	{
		VERBOSE("error: failed to load scene");
		return EXIT_FAILURE;
	}

	if (scene.mesh.IsEmpty()) 
	{
		VERBOSE("error: empty initial mesh");
		return EXIT_FAILURE;
	}

	const String baseFileName(Util::getFileFullName(OPT_TextureMesh::strOutputFileName));

	TD_TIMER_START();
	if (!scene.TextureMesh(
		OPT_TextureMesh::nResolutionLevel, 
		OPT_TextureMesh::nMinResolution, 
		OPT_TextureMesh::minCommonCameras, 
		OPT_TextureMesh::fOutlierThreshold, 
		OPT_TextureMesh::fRatioDataSmoothness,
		OPT_TextureMesh::bGlobalSeamLeveling, 
		OPT_TextureMesh::bLocalSeamLeveling, 
		OPT_TextureMesh::nTextureSizeMultiple, 
		OPT_TextureMesh::nRectPackingHeuristic, 
		Pixel8U(OPT_TextureMesh::nColEmpty),
		OPT_TextureMesh::fSharpnessWeight))
	{
		VERBOSE("error: failed to texture mesh");
		return EXIT_FAILURE;	
	}

	VERBOSE("Mesh 纹理生成完成: %u vertices, %u faces (%s)", scene.mesh.vertices.GetSize(), scene.mesh.faces.GetSize(), TD_TIMER_GET_FMT().c_str());

	scene.Save(baseFileName + _T(".mvs"), (ARCHIVE_TYPE)OPT_TextureMesh::nArchiveType);
	scene.mesh.Save(baseFileName + OPT_TextureMesh::strExportType);

	return EXIT_SUCCESS;
}
