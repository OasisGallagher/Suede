#include "shaderselector.h"

ShaderSelector::ShaderSelector(QWidget* parent) :QDialog(parent) {
	ui.setupUi(this);
	connect(ui.comboBox, SIGNAL(selectionChanged(const QString&)), this, SLOT(onSelectShader(const QString&)));
}

QString ShaderSelector::select(const QString& current) {
	selected_.clear();
	ui.comboBox->setDirectory("resources/shaders", current, ".*\\.shader");
	if (exec() == 0) {
		selected_.clear();
	}

	return selected_;
}

void ShaderSelector::onSelectShader(const QString& path) {
	selected_ = path;
}
