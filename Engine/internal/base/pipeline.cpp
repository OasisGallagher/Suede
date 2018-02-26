#include "pipeline.h"
#include "containers/freelist.h"

static free_list<Property> properties_(64);
static free_list<Renderable> renderables_(256);

void Pipeline::Update() {
	for (free_list<Renderable>::iterator ite = renderables_.begin(); ite != renderables_.end(); ++ite) {
		Renderable* renderable = *ite;
	}

	properties_.clear();
	renderables_.clear();
}

Property* Pipeline::CreateProperty() {
	return properties_.spawn();
}

Renderable* Pipeline::CreateRenderable() {
	Renderable* answer = renderables_.spawn();
	if (answer != nullptr) {
		ResetRenderable(answer);
	}

	return answer;
}

void Pipeline::ResetRenderable(Renderable* answer) {
	memset(answer->properties, 0, sizeof(answer->properties));
}
