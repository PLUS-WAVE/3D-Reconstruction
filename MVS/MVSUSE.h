#pragma once

namespace MVSUSE
{
	__declspec(dllexport) int DensifyPointCloud(int agrs_num, const char* d_args[]);

	__declspec(dllexport) int ReconstructMesh(int agrs_num, const char* m_args[]);

	bool Initialize_RefineMesh(size_t argc, LPCTSTR* argv);
	void Finalize_RefineMesh();
	__declspec(dllexport) int RefineMesh(int num, char* cmd[]);

	bool Initialize_TextureMesh(size_t argc, LPCTSTR* argv);
	void Finalize_TextureMesh();
	__declspec(dllexport) int TextureMesh(int num, char* cmd[]);

}
