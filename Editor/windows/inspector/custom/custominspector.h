#pragma once
#include <vector>

#include "component.h"

class MainContextCommand {
public:
	virtual ~MainContextCommand() {}

public:
	virtual void run() = 0;
};

class CustomInspector {
public:
	virtual void onGui(Component component) = 0;

public:
	virtual ~CustomInspector() {}

public:
	static void runMainContextCommands();

protected:
	static void addMainContextCommand(MainContextCommand* command);

private:
	static std::vector<MainContextCommand*> commands_;
};
