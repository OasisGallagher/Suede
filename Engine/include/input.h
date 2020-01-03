#pragma once
#include "subsystem.h"

#include "tools/enum.h"
#include "math/vector2.h"

BETTER_ENUM(KeyCode, int,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,

	KeypadEnter,

	Ctrl,
	Shift,

	Space,
	Return,
	Backspace,
	Tab,
	Escape,
	UpArrow,
	DownArrow,
	LeftArrow,
	RightArrow
)

class SUEDE_API InputDelegate {
	friend class Input;

public:
	virtual Vector2 GetMousePosition() = 0;

public:
	void OnMousePress(bool pressed[3]);
	void OnMouseWheel(float delta);

	void OnKeyPress(KeyCode key, bool pressed);

private:
	void* target;
};

class SUEDE_API Input : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Input)

public:
	enum {
		SystemType = SubsystemType::Input,
	};

public:
	Input();

public:
	void SetDelegate(InputDelegate* value);

	bool GetKey(KeyCode key);
	bool GetKeyUp(KeyCode key);
	bool GetKeyDown(KeyCode key);

	bool GetMouseButton(int button);
	bool GetMouseButtonUp(int button);
	bool GetMouseButtonDown(int button);

	float GetMouseWheelDelta();
	Vector2 GetMousePosition();

public:
	virtual void Update(float deltaTime);
};
