#pragma once

namespace MVSUSE
{
	__declspec(dllexport) int DensifyPointCloud(int agrs_num, const char* d_args[]);

	__declspec(dllexport) int ReconstructMesh(int agrs_num, const char* m_args[]);

	__declspec(dllexport) int RefineMesh(int agrs_num, const char* r_args[]);

	bool Initialize_TextureMesh(size_t argc, LPCTSTR* argv);
	void Finalize_TextureMesh();
	__declspec(dllexport) int TextureMesh(int num, char* cmd[]);

}
