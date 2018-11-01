#include "input.h"
#include "engine.h"
#include "memory/memory.h"

InputInternal::InputInternal() {
	Engine::instance()->AddFrameEventListener(this);
}

InputInternal::~InputInternal() {
	Engine::instance()->RemoveFrameEventListener(this);
}

void InputInternal::OnFrameLeave() {
}

int InputInternal::GetFrameEventQueue() {
	return FrameEventQueueInputs;
}

Input::Input() : Singleton2<Input>(nullptr, Memory::DeleteRaw<InputInternal>) {}

void Input::SetImplementation(InputInternal* impl) {
	if (destroyer_ != nullptr) { destroyer_(d_); };
	d_ = impl;
}

bool Input::GetKey(KeyCode key) { return _suede_dptr()->GetKey(key); }
bool Input::GetKeyUp(KeyCode key) { return _suede_dptr()->GetKeyUp(key); }
bool Input::GetKeyDown(KeyCode key) { return _suede_dptr()->GetKeyDown(key); }

bool Input::GetMouseButton(int button) { return  _suede_dptr()->GetMouseButton(button); }
bool Input::GetMouseButtonUp(int button) { return _suede_dptr()->GetMouseButtonUp(button); }
bool Input::GetMouseButtonDown(int button) { return _suede_dptr()->GetMouseButtonDown(button); }

int Input::GetMouseWheelDelta() { return _suede_dptr()->GetMouseWheelDelta(); }
glm::ivec2 Input::GetMousePosition() { return _suede_dptr()->GetMousePosition(); }
