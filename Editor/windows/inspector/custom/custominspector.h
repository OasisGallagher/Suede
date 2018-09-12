#pragma once
#include <vector>

#include "object.h"

class MainContextCommand {
public:
	virtual ~MainContextCommand() {}

public:
	virtual void Run() = 0;
};

class CustomInspector {
public:
	virtual void onGui() = 0;
	virtual void targetObject(Object object) = 0;

public:
	virtual ~CustomInspector() {}

public:
	static void runMainContextCommands();

protected:
	static void addMainContextCommand(MainContextCommand* command);

private:
	static std::vector<MainContextCommand*> commands_;
};

template <class T>
class CustomInspectorT : public CustomInspector {
public:
	virtual void targetObject(Object object) {
		target_ = suede_dynamic_cast<T>(object);
	}

protected:
	T target_;
};
