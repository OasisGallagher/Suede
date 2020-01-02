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
	bool AddBone(const SkeletonBone& bone);
	SkeletonBone* GetBone(uint index);
	SkeletonBone* GetBone(const std::string& name);
	
	void SetBoneToRootMatrix(uint index, const Matrix4& value);
	Matrix4* GetBoneToRootMatrices() { return boneToRootMatrices_; }

	int GetBoneIndex(const std::string& name);
	int GetBoneCount() { return current_; }

	SkeletonNode* CreateNode(const std::string& name, const Matrix4& matrix, AnimationCurve* curve);
	void AddNode(SkeletonNode* parent, SkeletonNode* child);
	SkeletonNode* GetRootNode() { return root_; }

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
	void SetWrapMode(AnimationWrapMode value);
	AnimationWrapMode GetWrapMode() { return wrapMode_; }

	void SetTicksPerSecond(float value);
	float GetTicksPerSecond() { return ticksInSecond_; }

	void SetDuration(float value) { duration_ = value; }
	float GetDuration() { return duration_; }

	void SetAnimation(Animation* value) { animation_ = value; }
	Animation* GetAnimation() { return animation_; }

	bool Sample(float time);

private:
	bool SampleHierarchy(float time, SkeletonNode* node, const Matrix4& matrix);

private:
	float duration_;
	float ticksInSecond_;
	float(*wrapper_)(float, float);
	AnimationWrapMode wrapMode_ = AnimationWrapMode::Loop;

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
	void AddFloat(float time, int id, float value);
	void AddVector3(float time, int id, const Vector3& value);
	void AddQuaternion(float time, int id, const Quaternion& value);

	void Remove(float time, int id);

	void ToKeyframes(std::vector<ref_ptr<AnimationFrame>>& keyframes);

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
	AnimationInternal() : ComponentInternal(ObjectType::Animation) {}

public:
	virtual void CullingUpdate(float deltaTime);

public:
	void AddClip(Animation* self, const std::string& name, AnimationClip* value);
	AnimationClip* GetClip(const std::string& name);

	void SetRootTransform(const Matrix4& value) { rootTransform_ = value; }
	Matrix4 GetRootTransform() { return rootTransform_; }

	void SetWrapMode(AnimationWrapMode value);
	AnimationWrapMode GetWrapMode() { return wrapMode_; }

	bool Play(const std::string& name);

	void SetSkeleton(Skeleton* value) { skeleton_ = value; }
	Skeleton* GetSkeleton() { return skeleton_.get(); }

	int GetUpdateStrategy() { return UpdateStrategyCulling; }

private:
	ref_ptr<Skeleton> skeleton_;
	Matrix4 rootTransform_;

	float time_ = 0;
	AnimationWrapMode wrapMode_ = AnimationWrapMode::Loop;

	bool playing_ = false;;
	ref_ptr<AnimationClip> current_;

	typedef std::map<std::string, ref_ptr<AnimationClip>> ClipContainer;
	ClipContainer clips_;
};

class AnimationFrameInternal : public ObjectInternal {
public:
	AnimationFrameInternal() :ObjectInternal(ObjectType::AnimationFrame), time_(0) {}

public:
	void SetTime(float value) { time_ = value; }
	float GetTime() { return time_; }

	void Assign(AnimationFrame* other);
	void Lerp(AnimationFrame* result, AnimationFrame* other, float factor);

	void SetFloat(int id, float value) { attributes_[id] = Variant(value); }
	void SetVector3(int id, const Vector3& value) { attributes_[id] = Variant(value); }
	void SetQuaternion(int id, const Quaternion& value) { attributes_[id] = Variant(value); }

	float GetFloat(int id);
	Vector3 GetVector3(int id);
	Quaternion GetQuaternion(int id);

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
	void SetKeys(AnimationKeys* value);
	bool Sample(float time, AnimationFrame*& frame);

private:
	int FindInterpolateIndex(float time);
	void SampleEndFrame(AnimationFrame*& frame);
	void Lerp(int index, float time, AnimationFrame*& frame);

private:
	std::vector<ref_ptr<AnimationFrame>> keyframes_;
};
