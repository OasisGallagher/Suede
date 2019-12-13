#include "engine.h"
#include <vector>

// Disable lua supports.
//struct lua_State;
class EngineInternal {
public:
	EngineInternal();

public:
	bool Startup(uint width, uint height);
	void Shutdown();

	void Update();
	
private:
	// Disable lua supports.
	//int updateRef_;
	// Disable lua supports.
	//lua_State* L;
};
