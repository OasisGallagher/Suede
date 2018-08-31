#include <Windows.h>

#include <iostream>

#include "glef.h"
#include "os/filesystem.h"

enum {
	InvalidArgument = 1,
	FailedToCreateGLEF = 2,
};

typedef bool(*RebuildMethod)(const char*);

bool InvokeRebuildMethod(const char* path) {
	HANDLE hDLL = LoadLibrary(L"GLEF.dll");
	if (hDLL == nullptr) {
		std::cout << "failed to load GLEF.dll." << std::endl;
		return false;
	}

	RebuildMethod func = (RebuildMethod)GetProcAddress((HMODULE)hDLL, "RebuildGLEF");
	if (func == nullptr) {
		std::cout << "failed to get function RebuildGLEF." << std::endl;
		return false;
	}

	std::cout << "rebuilding GLEF..." << std::endl;

	bool status = func(path);
	FreeLibrary((HMODULE)hDLL);

	return status;
}

bool Build(const char* exepath, const char* binpath, const char* dllpath) {
	time_t tmdll = FileSystem::GetFileLastWriteTime(dllpath);
	time_t tmbin = FileSystem::GetFileLastWriteTime(binpath);
	time_t tmexe = FileSystem::GetFileLastWriteTime(exepath);

	if (tmexe <= tmbin || tmdll <= tmbin) {
		std::cout << "no need to build GLEF." << std::endl;
		return true;
	}

	return InvokeRebuildMethod(binpath);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "usage: GLEFBuilder.exe GLEF_binary_path GLEF_dll_path." << std::endl;
		return InvalidArgument;
	}

	if (Build(argv[0], argv[1], argv[2])) {
		std::cout << "GLEF created and saved at " << argv[1] << "." << std::endl;
		return 0;
	}

	std::cout << "failed to create GLEF." << std::endl;
	return FailedToCreateGLEF;
}
