#pragma once
#include "component.h"

class BehaviourImpl;

class SUEDE_API Behaviour : public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	Behaviour();
	virtual ~Behaviour();

public:
	virtual void Awake() {}
	virtual void Update() {}

public:
	virtual bool GetEnabled() const final;
	virtual void SetEnabled(bool value) final;

	virtual void SetGameObject(GameObject value) final;
	virtual GameObject GetGameObject() final;

	virtual Transform GetTransform() final;

	virtual void CullingUpdate() final;
	virtual void RenderingUpdate() final;
	virtual int GetUpdateStrategy() final;

	virtual Object Clone() final;

	virtual ObjectType GetObjectType() final;
	virtual uint GetInstanceID() final;

private:
	BehaviourImpl* impl;
};
