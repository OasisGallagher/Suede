#include "qtinputdelegate.h"
#include "math/mathf.h"

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

	{ Qt::Key_Enter, KeyCode::KeypadEnter },

	{ Qt::Key_Control, KeyCode::Ctrl },
	{ Qt::Key_Shift, KeyCode::Shift },

	{ Qt::Key_Space, KeyCode::Space },
	{ Qt::Key_Return, KeyCode::Return },
	{ Qt::Key_Backspace, KeyCode::Backspace },
	{ Qt::Key_Tab, KeyCode::Tab },
	{ Qt::Key_Escape, KeyCode::Escape },
	{ Qt::Key_Up, KeyCode::UpArrow },
	{ Qt::Key_Down, KeyCode::DownArrow },
	{ Qt::Key_Left, KeyCode::LeftArrow },
	{ Qt::Key_Right, KeyCode::RightArrow },
};

QtInputDelegate::QtInputDelegate(QWidget* view) : view_(view) {
	view_->installEventFilter(this);
	view->setFocusPolicy(Qt::StrongFocus);
}

Vector2 QtInputDelegate::GetMousePosition() {
	QPoint p = view_->mapFromGlobal(QCursor::pos());
	return Vector2(p.x(), view_->height() - p.y());
}

bool QtInputDelegate::eventFilter(QObject * watched, QEvent * event) {
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

	return QObject::eventFilter(watched, event);
}

void QtInputDelegate::onKeyPress(QKeyEvent* event) {
	if (event->isAutoRepeat()) { return; }

	auto ite = keyMap.find(event->key());
	if (ite != keyMap.end()) {
		InputDelegate::OnKeyPress(ite.value(), (event->type() == QEvent::KeyPress));
	}
}

void QtInputDelegate::onMouseWheel(QWheelEvent* e) {
	InputDelegate::OnMouseWheel(e->delta());
}

void QtInputDelegate::onMousePress(QMouseEvent* e) {
	bool pressed[] = { 
		!!(e->buttons() & Qt::LeftButton),
		!!(e->buttons() & Qt::MiddleButton),
		!!(e->buttons() & Qt::RightButton)
	};

	InputDelegate::OnMousePress(pressed);
}
