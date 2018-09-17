#include "custominspector.h"

std::vector<MainContextCommand*> CustomInspector::commands_;

void CustomInspector::addMainContextCommand(MainContextCommand* command) {
	commands_.push_back(command);
}

void CustomInspector::runMainContextCommands() {
	for (MainContextCommand* cmd : commands_) {
		cmd->run();
		delete cmd;
	}

	commands_.clear();
}
