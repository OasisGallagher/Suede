#pragma once
#include <QObject>
#include "behaviour.h"

struct ISuedeObject {
	std::string name = "suede";
};

typedef std::shared_ptr<ISuedeObject> SuedeObject;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(SuedeObject);

class TestBehaviour : public QObject, public Behaviour {
	Q_OBJECT
	Q_PROPERTY(SuedeObject suedeObject READ suedeObject WRITE setSuedeObject)

public:
	void setSuedeObject(SuedeObject value) { suedeObject_ = value; }
	SuedeObject suedeObject() const { return suedeObject_; }

public:
	virtual void Update() {}

private:
	SuedeObject suedeObject_;
};
