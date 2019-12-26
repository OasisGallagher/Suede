#pragma once
#include "subsystem.h"

#include "tools/event.h"
#include "tools/singleton.h"

class GLCanvas;
class SUEDE_API Engine : private Singleton2<Engine> {
	friend class Singleton<Engine>;
	SUEDE_DECLARE_IMPLEMENTATION(Engine)

public:
	static void Startup(GLCanvas* canvas);
	static void Shutdown();

	static void BeginFrame();
	static void EndFrame();

	static void Update();

	static Subsystem* GetSubsystem(SubsystemType type);
	template <class T> static T* GetSubsystem();

private:
	Engine();
};

template <class T> T* Engine::GetSubsystem() {
	return dynamic_cast<T*>(GetSubsystem((SubsystemType)T::SystemType));
}
