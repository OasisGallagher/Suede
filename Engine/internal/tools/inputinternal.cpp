#include "input.h"
#include "world.h"
#include "memory/refptr.h"
#include "frameeventqueue.h"

InputInternal::InputInternal() {
	World::frameLeave().subscribe(this, &InputInternal::OnFrameLeave, (int)FrameEventQueue::Inputs);
}

InputInternal::~InputInternal() {
	World::frameLeave().unsubscribe(this);
}

Input::Input() : Singleton2<Input>(nullptr, t_delete<InputInternal>) {}

void Input::SetDelegate(InputInternal* delegate) {
	instance()->_destroy();
	instance()->d_ = delegate;
}

bool Input::GetKey(KeyCode key) { return _suede_dinstance()->GetKey(key); }
bool Input::GetKeyUp(KeyCode key) { return _suede_dinstance()->GetKeyUp(key); }
bool Input::GetKeyDown(KeyCode key) { return _suede_dinstance()->GetKeyDown(key); }

bool Input::GetMouseButton(int button) { return  _suede_dinstance()->GetMouseButton(button); }
bool Input::GetMouseButtonUp(int button) { return _suede_dinstance()->GetMouseButtonUp(button); }
bool Input::GetMouseButtonDown(int button) { return _suede_dinstance()->GetMouseButtonDown(button); }

float Input::GetMouseWheelDelta() { return _suede_dinstance()->GetMouseWheelDelta(); }
Vector2 Input::GetMousePosition() { return _suede_dinstance()->GetMousePosition(); }
