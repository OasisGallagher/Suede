#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QFormLayout>

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
	Mesh mesh = dsp_cast<Mesh>(object_);
	QLabel* topology = new QLabel(this);
	topology->setText(mesh->GetTopology() == MeshTopologyTriangles ? "Triangles" : "TriangleStrips");
	form_->addRow(formatRowName("Topology"), topology);

	QLabel* vertices = new QLabel(this);
	vertices->setText(QString::number(mesh->GetVertices().size()));
	form_->addRow(formatRowName("Vertices"), vertices);

	QListWidget* subMeshList = new QListWidget(this);

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		uint indexCount, baseIndex, baseVertex;
		subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

		int triangles = mesh->GetTopology() == MeshTopologyTriangles ? indexCount / 3 : Math::Max(0u, indexCount - 2);
		subMeshList->addItem(QString::asprintf("Triangles: %d", triangles));
	}

	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, subMeshList);

	shrinkToFit(subMeshList);

	if (mesh->GetType() == ObjectTypeTextMesh) {
		TextMesh textMesh = dsp_cast<TextMesh>(mesh);
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
}

void MeshInspector::onTextChanged() {
	Mesh mesh = dsp_cast<Mesh>(object_);
	QByteArray arr = ((QLineEdit*)sender())->text().toLocal8Bit();

	TextMesh textMesh = dsp_cast<TextMesh>(mesh);
	textMesh->SetText(arr.toStdString());

	QImage image;
	std::vector<uchar> data;
	if (textMesh->GetFont()->GetTexture()->EncodeToPng(data) && image.loadFromData(&data[0], data.size())) {
		image.save("e:/1.png");
	}
}

