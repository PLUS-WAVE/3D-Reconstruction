#pragma once

namespace MAIN
{
	__declspec(dllexport) int start(std::string imagesInputDir, std::string kmatrix, std::string describerMethod,
	                                std::string finalExportFormat, int task);

}