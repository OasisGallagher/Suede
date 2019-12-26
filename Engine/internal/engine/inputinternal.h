#pragma once
#include "input.h"
#include "subsysteminternal.h"

class InputInternal : public SubsystemInternal {
public:
	bool GetKey(KeyCode key);
	bool GetKeyUp(KeyCode key);
	bool GetKeyDown(KeyCode key);

	bool GetMouseButton(int button);
	bool GetMouseButtonUp(int button);
	bool GetMouseButtonDown(int button);

	float GetMouseWheelDelta();
	Vector2 GetMousePosition();

	void Update();

public:
	void OnMousePress(bool pressed[3]);
	void OnMouseWheel(float delta) { wheelDelta_ = delta; }
	void OnKeyPress(KeyCode key, bool pressed);
	void SetDelegate(InputDelegate* value) { delegate_ = value; }

private:
	float wheelDelta_ = 0;
	InputDelegate* delegate_ = nullptr;

	template <int N> struct States {
		enum { Size = N };
		bool pressed[Size] = { false };
		bool up[Size] = { false }, down[Size] = { false };
	};

	States<3> mouseStates_;
	States<KeyCode::size()> keyStates_;
};
