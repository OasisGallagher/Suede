#pragma once
#include <glm/glm.hpp>

#include "tools/singleton.h"
#include "frameeventlistener.h"

enum class KeyCode {
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

	Space,

	_Count,
};

class SUEDE_API InputInternal : public FrameEventListener {
public:
	InputInternal();
	virtual ~InputInternal();

public:
	virtual int GetFrameEventQueue() final;

public:
	virtual void OnFrameLeave() = 0;

	virtual bool GetKey(KeyCode key) = 0;
	virtual bool GetKeyUp(KeyCode key) = 0;
	virtual bool GetKeyDown(KeyCode key) = 0;

	virtual bool GetMouseButton(int button) = 0;
	virtual bool GetMouseButtonUp(int button) = 0;
	virtual bool GetMouseButtonDown(int button) = 0;

	virtual int GetMouseWheelDelta() = 0;
	virtual glm::ivec2 GetMousePosition() = 0;
};

class SUEDE_API Input : public Singleton2<Input> {
	friend class Singleton<Input>;
	SUEDE_DECLARE_IMPLEMENTATION(Input)

public:
	void SetImplementation(InputInternal* impl);

public:
	bool GetKey(KeyCode key);
	bool GetKeyUp(KeyCode key);
	bool GetKeyDown(KeyCode key);

	bool GetMouseButton(int button);
	bool GetMouseButtonUp(int button);
	bool GetMouseButtonDown(int button);

	int GetMouseWheelDelta();
	glm::ivec2 GetMousePosition();

private:
	Input();
};
