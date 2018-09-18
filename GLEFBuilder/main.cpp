#include <Windows.h>

#include <iostream>

#include "glef.h"
#include "os/filesystem.h"

enum {
	InvalidArgument = 1,
	FailedToCreateGLEF = 2,
};

typedef bool (*RebuildMethod)(const char*);

bool InvokeRebuildMethod(const char* path) {
	HMODULE module = LoadLibrary(L"GLEF.dll");
	if (module == nullptr) {
		std::cout << "failed to load GLEF.dll." << std::endl;
		return false;
	}

	RebuildMethod rebuild = (RebuildMethod)GetProcAddress(module, "RebuildGLEF");
	if (rebuild == nullptr) {
		std::cout << "failed to get function RebuildGLEF." << std::endl;
		return false;
	}

	std::cout << "rebuilding GLEF..." << std::endl;

	bool status = rebuild(path);
	FreeLibrary(module);

	return status;
}

bool Rebuild(const char* binpath, const char* dllpath) {
	time_t tmdll = FileSystem::GetFileLastWriteTime(dllpath);
	time_t tmbin = FileSystem::GetFileLastWriteTime(binpath);

	if (tmdll <= tmbin) {
		std::cout << "no need to build GLEF (" << tmdll << " " << tmbin << ")." << std::endl;
		return true;
	}

	return InvokeRebuildMethod(binpath);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "usage: GLEFBuilder.exe GLEF_binary_path GLEF_dll_path." << std::endl;
		return InvalidArgument;
	}

	if (Rebuild(argv[1], argv[2])) {
		std::cout << "GLEF created and saved at " << argv[1] << "." << std::endl;
		return 0;
	}

	std::cout << "failed to create GLEF." << std::endl;
	return FailedToCreateGLEF;
}
