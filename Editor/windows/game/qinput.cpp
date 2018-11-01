#include "qinput.h"
#include "tools/math2.h"

static QHash<int, KeyCode> keyMap = {
	{ Qt::Key_A, KeyCode::A },
	{ Qt::Key_B, KeyCode::B },
	{ Qt::Key_C, KeyCode::C },
	{ Qt::Key_D, KeyCode::D },
	{ Qt::Key_E, KeyCode::E },
	{ Qt::Key_F, KeyCode::F },
	{ Qt::Key_G, KeyCode::G },
	{ Qt::Key_H, KeyCode::H },
	{ Qt::Key_I, KeyCode::I },
	{ Qt::Key_J, KeyCode::J },
	{ Qt::Key_K, KeyCode::K },
	{ Qt::Key_L, KeyCode::L },
	{ Qt::Key_M, KeyCode::M },
	{ Qt::Key_N, KeyCode::N },
	{ Qt::Key_O, KeyCode::O },
	{ Qt::Key_P, KeyCode::P },
	{ Qt::Key_Q, KeyCode::Q },
	{ Qt::Key_R, KeyCode::R },
	{ Qt::Key_S, KeyCode::S },
	{ Qt::Key_T, KeyCode::T },
	{ Qt::Key_U, KeyCode::U },
	{ Qt::Key_V, KeyCode::V },
	{ Qt::Key_W, KeyCode::W },
	{ Qt::Key_X, KeyCode::X },
	{ Qt::Key_Y, KeyCode::Y },
	{ Qt::Key_Z, KeyCode::Z },

	{ Qt::Key_F1, KeyCode::F1 },
	{ Qt::Key_F2, KeyCode::F2 },
	{ Qt::Key_F3, KeyCode::F3 },
	{ Qt::Key_F4, KeyCode::F4 },
	{ Qt::Key_F5, KeyCode::F5 },
	{ Qt::Key_F6, KeyCode::F6 },
	{ Qt::Key_F7, KeyCode::F7 },
	{ Qt::Key_F8, KeyCode::F8 },
	{ Qt::Key_F9, KeyCode::F9 },
	{ Qt::Key_F10, KeyCode::F10 },
	{ Qt::Key_F11, KeyCode::F11 },
	{ Qt::Key_F12, KeyCode::F12 },

	{ Qt::Key_Space, KeyCode::Space },
};

template <class T>
inline void ResetUpDown(T& state) {
	std::fill(state.up, state.up + T::Size, false);
	std::fill(state.down, state.down + T::Size, false);
}

QInput::QInput(QWidget* view) : view_(view) {
	view_->installEventFilter(this);
}

void QInput::OnFrameLeave() {
	wheelDelta_ = 0;

	ResetUpDown(keyStates_);
	ResetUpDown(mouseStates_);
}

bool QInput::GetKey(KeyCode key) {
	return keyStates_.pressed[(int)key];
}

bool QInput::GetKeyUp(KeyCode key) {
	return keyStates_.up[(int)key];
}

bool QInput::GetKeyDown(KeyCode key) {
	return keyStates_.down[(int)key];
}

bool QInput::GetMouseButton(int button) {
	return mouseStates_.pressed[button];
}

bool QInput::GetMouseButtonUp(int button) {
	return mouseStates_.up[button];
}

bool QInput::GetMouseButtonDown(int button) {
	return mouseStates_.down[button];
}

int QInput::GetMouseWheelDelta() {
	return wheelDelta_;
}

glm::ivec2 QInput::GetMousePosition() {
	QPoint p = view_->mapFromGlobal(QCursor::pos());
	return glm::ivec2(p.x(), p.y());
}

bool QInput::eventFilter(QObject * watched, QEvent * event) {
	switch (event->type()) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
			onMousePress((QMouseEvent*)event);
			break;
		case QEvent::Wheel:
			onMouseWheel((QWheelEvent*)event);
			break;
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
			onKeyPress((QKeyEvent*)event);
			break;
	}

	return true;
}

void QInput::onKeyPress(QKeyEvent * event) {
	if (event->isAutoRepeat()) { return; }

	auto ite = keyMap.find(event->key());
	if (ite == keyMap.end()) { return; }

	bool pressed = (event->type() == QEvent::KeyPress);
	if (pressed != keyStates_.pressed[(int)ite.value()]) {
		keyStates_.pressed[(int)ite.value()] = pressed;
		pressed ? (keyStates_.down[(int)ite.value()] = true) : (keyStates_.up[(int)ite.value()] = true);
	}
}

void QInput::onMouseWheel(QWheelEvent* e) {
	wheelDelta_ = e->delta();
}

void QInput::onMousePress(QMouseEvent* e) {
	bool pressed[] = { 
		e->buttons() & Qt::LeftButton,
		e->buttons() & Qt::MiddleButton,
		e->buttons() & Qt::RightButton
	};

	for (int i = 0; i < decltype(mouseStates_)::Size; ++i) {
		if (mouseStates_.pressed[i] != pressed[i]) {
			mouseStates_.pressed[i] = pressed[i];
			pressed[i] ? (mouseStates_.down[i] = true) : (mouseStates_.up[i] = true);
		}
	}
}
