#include <QLabel>
#include <QLineEdit>
#include <QListWidget>

#include "tools/math2.h"

#include "mesh.h"
#include "meshinspector.h"

namespace Constants {
	static uint minFieldOfView = 0;
	static uint maxFieldOfView = 180;
}

namespace Literals {
	DEFINE_LITERAL(cameraFovLabel);
	DEFINE_LITERAL(cameraFovSlider);
	DEFINE_LITERAL(cameraFovSlider2);
}

MeshInspector::MeshInspector(Object object) : CustomInspector("Mesh", object) {
	drawMesh();

	if (suede_dynamic_cast<TextMesh>(target_)) {
		drawTextMesh();
	}
}

void MeshInspector::drawMesh() {
	Mesh mesh = suede_dynamic_cast<Mesh>(target_);
	QLabel* topology = new QLabel(this);
	topology->setText(mesh->GetTopology() == MeshTopologyTriangles ? "Triangles" : "TriangleStrips");
	form_->addRow(formatRowName("Topology"), topology);

	QLabel* vertices = new QLabel(this);
	
	uint count = mesh->GetVertexCount();
	vertices->setText(QString::number(count));
	form_->addRow(formatRowName("Vertices"), vertices);

	QListWidget* subMeshList = new QListWidget(this);

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		const TriangleBias& bias = subMesh->GetTriangleBias();

		int triangles = mesh->GetTopology() == MeshTopologyTriangles ? bias.indexCount / 3 : Math::Max(0u, bias.indexCount - 2);
		subMeshList->addItem(QString::asprintf("Triangles: %d", triangles));
	}

	resizeGeometryToFit(subMeshList);
	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, subMeshList);
}

void MeshInspector::drawTextMesh() {
	TextMesh textMesh = suede_dynamic_cast<TextMesh>(target_);
	QLineEdit* text = new QLineEdit(this);
	text->setText(QString::fromLocal8Bit(textMesh->GetText().c_str()));
	connect(text, SIGNAL(editingFinished()), this, SLOT(onTextChanged()));
	form_->addRow(formatRowName("Text"), text);

	Font font = textMesh->GetFont();
	QLabel* fontName = new QLabel(this);
	fontName->setText(font->GetFamilyName().c_str());
	form_->addRow(formatRowName("Font"), fontName);

	QLabel* fontSize = new QLabel(this);
	fontSize->setText(QString::number(font->GetFontSize()));
	form_->addRow(formatRowName("Size"), fontSize);
}

void MeshInspector::onTextChanged() {
	Mesh mesh = suede_dynamic_cast<Mesh>(target_);
	QByteArray arr = ((QLineEdit*)sender())->text().toLocal8Bit();

	TextMesh textMesh = suede_dynamic_cast<TextMesh>(mesh);
	textMesh->SetText(arr.toStdString());
}
