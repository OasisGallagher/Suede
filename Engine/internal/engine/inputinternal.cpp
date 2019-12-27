#include "inputinternal.h"

#include "debug/debug.h"

void InputDelegate::OnMousePress(bool pressed[3]) { ((InputInternal*)target)->OnMousePress(pressed); }
void InputDelegate::OnMouseWheel(float delta) { ((InputInternal*)target)->OnMouseWheel(delta); }
void InputDelegate::OnKeyPress(KeyCode key, bool pressed) { ((InputInternal*)target)->OnKeyPress(key, pressed); }

Input::Input() : Subsystem(new InputInternal()) {}
void Input::SetDelegate(InputDelegate* value) { _suede_dptr()->SetDelegate(value);  value->target = d_; }
bool Input::GetKey(KeyCode key) { return _suede_dptr()->GetKey(key); }
bool Input::GetKeyUp(KeyCode key) { return _suede_dptr()->GetKeyUp(key); }
bool Input::GetKeyDown(KeyCode key) { return _suede_dptr()->GetKeyDown(key); }
bool Input::GetMouseButton(int button) { return  _suede_dptr()->GetMouseButton(button); }
bool Input::GetMouseButtonUp(int button) { return _suede_dptr()->GetMouseButtonUp(button); }
bool Input::GetMouseButtonDown(int button) { return _suede_dptr()->GetMouseButtonDown(button); }
float Input::GetMouseWheelDelta() { return _suede_dptr()->GetMouseWheelDelta(); }
Vector2 Input::GetMousePosition() { return _suede_dptr()->GetMousePosition(); }
void Input::Update(float deltaTime) { return _suede_dptr()->Update(deltaTime); }

template <class T>
inline void ResetUpDown(T& state) {
	std::fill(state.up, state.up + T::Size, false);
	std::fill(state.down, state.down + T::Size, false);
}

void InputInternal::Update(float deltaTime) {
	wheelDelta_ = 0;

	ResetUpDown(keyStates_);
	ResetUpDown(mouseStates_);
}

void InputInternal::OnMousePress(bool pressed[3]) {
	for (int i = 0; i < 3; ++i) {
		if (mouseStates_.pressed[i] != pressed[i]) {
			mouseStates_.pressed[i] = pressed[i];
			(pressed[i] ? mouseStates_.down : mouseStates_.up)[i] = true;
		}
	}
}

void InputInternal::OnKeyPress(KeyCode key, bool pressed) {
	SUEDE_ASSERT(key >= 0 && key < KeyCode::size());
	if (pressed != keyStates_.pressed[key]) {
		keyStates_.pressed[key] = pressed;
		(pressed ? keyStates_.down : keyStates_.up)[key] = true;
	}
}

bool InputInternal::GetKey(KeyCode key) {
	SUEDE_ASSERT(key >= 0 && key < KeyCode::size());
	return keyStates_.pressed[key];
}

bool InputInternal::GetKeyUp(KeyCode key) {
	SUEDE_ASSERT(key >= 0 && key < KeyCode::size());
	return keyStates_.up[key];
}

bool InputInternal::GetKeyDown(KeyCode key) {
	SUEDE_ASSERT(key >= 0 && key < KeyCode::size());
	return keyStates_.down[key];
}

bool InputInternal::GetMouseButton(int button) {
	SUEDE_ASSERT(button >= 0 && button < 3);
	return mouseStates_.pressed[button];
}

bool InputInternal::GetMouseButtonUp(int button) {
	SUEDE_ASSERT(button >= 0 && button < 3);
	return mouseStates_.up[button];
}

bool InputInternal::GetMouseButtonDown(int button) {
	SUEDE_ASSERT(button >= 0 && button < 3);
	return mouseStates_.down[button];
}

float InputInternal::GetMouseWheelDelta() {
	return wheelDelta_;
}

Vector2 InputInternal::GetMousePosition() {
	SUEDE_ASSERT(delegate_ != nullptr);
	return delegate_->GetMousePosition();
}
