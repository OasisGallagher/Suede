#pragma once

class Context;
class LuaBridge {
public:
	LuaBridge(Context* context);
	~LuaBridge();

	void Update(float deltaTime);

public:
	Context* context_ = nullptr;
	struct lua_State* L = nullptr;

	int updateRef_;
};