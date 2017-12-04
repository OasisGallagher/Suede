#include <fstream>
#include "debug.h"
#include "textloader.h"

bool TextLoader::Load(const std::string& file, std::string& text) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs) {
		Debug::LogError("failed to open file %s.", file.c_str());
		return false;
	}

	text.clear();
	const char* seperator = "";

	std::string line;
	for (; getline(ifs, line);) {
		if (!line.empty()) {
			text += seperator;
			text += line;
			seperator = "\n";
		}
	}

	ifs.close();

	return true;
}

bool TextLoader::Load(const std::string& file, std::vector<std::string>& lines) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs) {
		Debug::LogError("failed to open file %s.", file.c_str());
		return false;
	}

	lines.clear();

	std::string line;
	for (; getline(ifs, line);) {
		if (!line.empty()) {
			lines.push_back(line);
		}
	}

	ifs.close();

	return true;
}
