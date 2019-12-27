#pragma once

class Context;
class GLObjectMaintainer {
public:
	GLObjectMaintainer(Context* context);
	~GLObjectMaintainer();

protected:
	virtual void OnContextDestroyed() { context_ = nullptr; }

protected:
	Context* context_;
};
