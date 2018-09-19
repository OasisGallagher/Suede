#pragma once
#include <memory>

#include <QObject>
#include <QVector>
#include <QMetaProperty>

struct ISuedeObject {
	std::string name = "suede";
};

typedef std::shared_ptr<ISuedeObject> SuedeObject;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(SuedeObject);

class TestBehaviour : public QObject {
	Q_OBJECT
	Q_PROPERTY(QVector<SuedeObject> suedeObjects READ suedeObjects WRITE setSuedeObjects)

public:
	QVector<SuedeObject> suedeObjects() {
		return suedeObjects_;
// 		QList<QVariant> answer;
// 		for (SuedeObject object : suedeObjects_) {
// 			answer.push_back(QVariant::fromValue(object));
// 		}
// 
// 		return answer;
	}

	void setSuedeObjects(const QVector<SuedeObject>& value) {
		suedeObjects_ = value;
		/*
		suedeObjects_.clear();

		for (QVariant variant : value) {
			suedeObjects_.push_back(variant.value<SuedeObject>());
		}*/
	}

private:
	QVector<SuedeObject> suedeObjects_;
};