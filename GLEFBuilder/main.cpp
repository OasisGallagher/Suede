#include <Windows.h>

#include <iostream>

#include "glef.h"
#include "os/filesystem.h"

enum {
	InvalidArgument = 1,
	FailedToCreateGLEF = 2,
};

typedef bool(*RebuildFunc)(const char*);

RebuildFunc GetRebuildFunction() {
	HANDLE dll = LoadLibrary(L"GLEF.dll");
	RebuildFunc func = (RebuildFunc)GetProcAddress((HMODULE)dll, "RebuildGLEF");
	if (func == nullptr) {
		std::cout << "failed to get function RebuildGLEF." << std::endl;
	}

	return func;
}

bool Build(const char* binpath, const char* dllpath) {
	time_t tmdll = FileSystem::GetFileLastWriteTime(dllpath);
	time_t tmbin = FileSystem::GetFileLastWriteTime(binpath);

	if (tmdll <= tmbin) {
		std::cout << "no need to build GLEF." << std::endl;
		return true;
	}

	RebuildFunc func = GetRebuildFunction();
	if (func == nullptr) {
		return false;
	}

	std::cout << "rebuilding GLEF..." << std::endl;
	return func(binpath);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "usage: GLEFBuilder.exe GLEF_binary_path GLEF_dll_path." << std::endl;
		return InvalidArgument;
	}

	const char* binpath = argv[1];
	const char* dllpath = argv[2];

	if (Build(binpath, dllpath)) {
		std::cout << "GLEF created and saved at " << binpath << "." << std::endl;
		return 0;
	}

	std::cout << "failed to create GLEF." << std::endl;
	return FailedToCreateGLEF;
}
