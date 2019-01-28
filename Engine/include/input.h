#pragma once
#include <glm/glm.hpp>

#include "tools/enum.h"
#include "tools/singleton.h"
#include "frameeventlistener.h"

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

class SUEDE_API InputInternal : public FrameEventListener {
public:	InputInternal();	virtual ~InputInternal();

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

	virtual float GetMouseWheelDelta() = 0;
	virtual glm::ivec2 GetMousePosition() = 0;
};

class SUEDE_API Input : private singleton2<Input> {
	friend class singleton<Input>;
	SUEDE_DECLARE_IMPLEMENTATION(Input)

public:
	static void SetDelegate(InputInternal* impl);

public:
	/**
	 * @returns true while the user holds down the key identified by the key KeyCode enum parameter.
	 */
	static bool GetKey(KeyCode key);

	/**
	 * @returns true during the frame the user releases the key identified by the key KeyCode enum parameter.
	 */
	static bool GetKeyUp(KeyCode key);

	/**
	 * @returns true during the frame the user starts pressing down the key identified by the key KeyCode enum parameter.
	 */
	static bool GetKeyDown(KeyCode key);

	/**
	 * @returns whether the given mouse button is held down.
	 */
	static bool GetMouseButton(int button);

	/**
	 * @returns true during the frame the user pressed the given mouse button.
	 */
	static bool GetMouseButtonUp(int button);

	/**
	 * @returns true during the frame the user releases the given mouse button.
	 */
	static bool GetMouseButtonDown(int button);

	static float GetMouseWheelDelta();

	/**
	 * @returns the current mouse position in pixel coordinates(0 at the bottom and increases upward).
	 */
	static glm::ivec2 GetMousePosition();

private:
	Input();
};
