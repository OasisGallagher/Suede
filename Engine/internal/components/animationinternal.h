#include <map>

#include "variant.h"
#include "animation.h"
#include "builtinproperties.h"
#include "componentinternal.h"
#include "internal/base/renderdefines.h"

class SkeletonInternal : public ObjectInternal {
public:
	SkeletonInternal() : ObjectInternal(ObjectType::Skeleton), current_(0), root_(nullptr) {}
	~SkeletonInternal() { DestroyNodeHierarchy(root_); }

public:
	virtual bool AddBone(const SkeletonBone& bone);
	virtual SkeletonBone* GetBone(uint index);
	virtual SkeletonBone* GetBone(const std::string& name);
	
	virtual void SetBoneToRootMatrix(uint index, const Matrix4& value);
	virtual Matrix4* GetBoneToRootMatrices() { return boneToRootMatrices_; }

	virtual int GetBoneIndex(const std::string& name);
	virtual int GetBoneCount() { return current_; }

	virtual SkeletonNode* CreateNode(const std::string& name, const Matrix4& matrix, AnimationCurve* curve);
	virtual void AddNode(SkeletonNode* parent, SkeletonNode* child);
	virtual SkeletonNode* GetRootNode() { return root_; }

private:
	void DestroyNodeHierarchy(SkeletonNode*& node);

private:
	int current_;
	SkeletonBone bones_[MAX_BONE_COUNT];
	Matrix4 boneToRootMatrices_[MAX_BONE_COUNT];

	SkeletonNode* root_;

	std::map<std::string, int> boneMap_;
};

class AnimationClipInternal : public ObjectInternal {
public:
	AnimationClipInternal();
	~AnimationClipInternal();

public:
	virtual void SetWrapMode(AnimationWrapMode value);
	virtual AnimationWrapMode GetWrapMode() { return wrapMode_; }

	virtual void SetTicksPerSecond(float value);
	virtual float GetTicksPerSecond() { return ticksInSecond_; }

	virtual void SetDuration(float value) { duration_ = value; }
	virtual float GetDuration() { return duration_; }

	virtual void SetAnimation(Animation* value) { animation_ = value; }
	virtual Animation* GetAnimation() { return animation_; }

	virtual bool Sample(float time);

private:
	bool SampleHierarchy(float time, SkeletonNode* node, const Matrix4& matrix);

private:
	float duration_;
	float ticksInSecond_;
	float(*wrapper_)(float, float);
	AnimationWrapMode wrapMode_;

	AnimationFrame* frame_;
	Animation* animation_;
};

class AnimationStateInternal : public ObjectInternal {
public:
	AnimationStateInternal() : ObjectInternal(ObjectType::AnimationState) {}
};

class AnimationKeysInternal : public ObjectInternal {
public:
	AnimationKeysInternal();
	~AnimationKeysInternal();

public:
	virtual void AddFloat(float time, int id, float value);
	virtual void AddVector3(float time, int id, const Vector3& value);
	virtual void AddQuaternion(float time, int id, const Quaternion& value);

	virtual void Remove(float time, int id);

	virtual void ToKeyframes(std::vector<ref_ptr<AnimationFrame>>& keyframes);

private:
	struct ApproximatelyFloatComparer {
		bool operator()(float lhs, float rhs) const {
			return !Mathf::Approximately(lhs, rhs) && lhs < rhs;
		}
	};

	struct Key {
		int id;
		Variant value;
	};

	typedef std::map<float, Key, ApproximatelyFloatComparer> Keys;
	typedef std::vector<Keys*> KeysContainer;

private:
	void InsertKey(float time, const Key& key);
	void RemoveKey(int id, float time);

	int SmoothKeys();
	void SmoothKey(Keys* keys, float time);

	void InitializeKeyframes(int count, std::vector<ref_ptr<AnimationFrame>> &keyframes);

private:
	KeysContainer container_;
};

class AnimationInternal : public ComponentInternal {
public:
	AnimationInternal() : ComponentInternal(ObjectType::Animation), time_(0), playing_(false) {}

public:
	virtual void CullingUpdate(float deltaTime);

public:
	virtual void AddClip(Animation* self, const std::string& name, AnimationClip* value);
	virtual AnimationClip* GetClip(const std::string& name);

	virtual void SetRootTransform(const Matrix4& value) { rootTransform_ = value; }
	virtual Matrix4 GetRootTransform() { return rootTransform_; }

	virtual void SetWrapMode(AnimationWrapMode value);

	virtual bool Play(const std::string& name);

	virtual void SetSkeleton(Skeleton* value) { skeleton_ = value; }
	virtual Skeleton* GetSkeleton() { return skeleton_.get(); }

	virtual int GetUpdateStrategy() { return UpdateStrategyCulling; }

private:
	ref_ptr<Skeleton> skeleton_;
	Matrix4 rootTransform_;

	float time_;

	bool playing_;
	ref_ptr<AnimationClip> current_;

	typedef std::map<std::string, ref_ptr<AnimationClip>> ClipContainer;
	ClipContainer clips_;
};

class AnimationFrameInternal : public ObjectInternal {
public:
	AnimationFrameInternal() :ObjectInternal(ObjectType::AnimationFrame), time_(0) {}

public:
	virtual void SetTime(float value) { time_ = value; }
	virtual float GetTime() { return time_; }

	virtual void Assign(AnimationFrame* other);
	virtual void Lerp(AnimationFrame* result, AnimationFrame* other, float factor);

	virtual void SetFloat(int id, float value);
	virtual void SetVector3(int id, const Vector3& value);
	virtual void SetQuaternion(int id, const Quaternion& value);

	virtual float GetFloat(int id);
	virtual Vector3 GetVector3(int id);
	virtual Quaternion GetQuaternion(int id);

private:
	void LerpAttribute(AnimationFrame* ans, int id, const Variant& lhs, const Variant& rhs, float factor);

private:
	float time_;

	typedef std::map<int, Variant> AttributeContainer;
	AttributeContainer attributes_;
};

class AnimationCurveInternal : public ObjectInternal {
public:
	AnimationCurveInternal() :ObjectInternal(ObjectType::AnimationCurve) {}
	~AnimationCurveInternal() {}

public:
	virtual void SetKeys(AnimationKeys* value);
	virtual bool Sample(float time, AnimationFrame*& frame);

private:
	int FindInterpolateIndex(float time);
	void SampleEndFrame(AnimationFrame*& frame);
	void Lerp(int index, float time, AnimationFrame*& frame);

private:
	std::vector<ref_ptr<AnimationFrame>> keyframes_;
};
