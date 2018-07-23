#include "lightdialog.h"
#include "world.h"

LightDialog::LightDialog(QWidget* parent) : PopupWidget(parent) {
	ui_.setupUi(this);
	connect(ui_.ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));
}

void LightDialog::showEvent(QShowEvent* event) {
	ui_.ambient->setValue(WorldInstance()->GetEnvironment()->GetAmbientColor());
	PopupWidget::showEvent(event);
}

void LightDialog::onAmbientChanged(const QColor& color) {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}
