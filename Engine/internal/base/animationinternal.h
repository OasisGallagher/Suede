#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "variables.h"
#include "animation.h"
#include "glsldefines.h"
#include "internal/containers/variant.h"
#include "internal/base/objectinternal.h"
#include "internal/containers/sortedvector.h"

class SkeletonInternal : public ISkeleton, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Skeleton)

public:
	SkeletonInternal() : ObjectInternal(ObjectTypeSkeleton), current_(0), root_(nullptr) {}
	~SkeletonInternal() { DestroyNodeHierarchy(root_); }

public:
	virtual bool AddBone(const SkeletonBone& bone);
	virtual SkeletonBone* GetBone(int index);
	virtual SkeletonBone* GetBone(const std::string& name);
	
	virtual void SetBoneToRootSpaceMatrix(int index, const glm::mat4& value);
	virtual glm::mat4* GetBoneToRootSpaceMatrices() { return boneToRootSpaceMatrices_; }

	virtual int GetBoneIndex(const std::string& name);
	virtual int GetBoneCount() { return current_; }

	virtual SkeletonNode* CreateNode(const std::string& name, const glm::mat4& matrix, AnimationCurve curve);
	virtual void AddNode(SkeletonNode* parent, SkeletonNode* child);
	virtual SkeletonNode* GetRootNode() { return root_; }

private:
	void DestroyNodeHierarchy(SkeletonNode*& node);

private:
	int current_;
	SkeletonBone bones_[C_MAX_BONE_COUNT];
	glm::mat4 boneToRootSpaceMatrices_[C_MAX_BONE_COUNT];

	SkeletonNode* root_;

	std::map<std::string, int> boneMap_;
};

class AnimationClipInternal : public IAnimationClip, public ObjectInternal {
	DEFINE_FACTORY_METHOD(AnimationClip)

public:
	AnimationClipInternal();

public:
	virtual void SetWrapMode(AnimationWrapMode value);
	virtual AnimationWrapMode GetWrapMode() { return wrapMode_; }

	virtual void SetTicksPerSecond(float value);
	virtual float GetTicksPerSecond() { return ticksInSecond_; }

	virtual void SetDuration(float value) { duration_ = value; }
	virtual float GetDuration() { return duration_; }

	virtual void SetAnimation(Animation value) { animation_ = value; }
	virtual Animation GetAnimation() { return animation_.lock(); }

	virtual bool Sample(float time);

private:
	bool SampleHierarchy(float time, SkeletonNode* node, const glm::mat4& matrix);

private:
	float duration_;
	float ticksInSecond_;
	float(*wrapper_)(float, float);
	AnimationWrapMode wrapMode_;
	std::weak_ptr<Animation::element_type> animation_;
};

class AnimationStateInternal : public IAnimationState, public ObjectInternal {
	DEFINE_FACTORY_METHOD(AnimationState)

public:
	AnimationStateInternal() : ObjectInternal(ObjectTypeAnimationState) {}
};

class AnimationKeysInternal : public IAnimationKeys, public ObjectInternal {
	DEFINE_FACTORY_METHOD(AnimationKeys)

public:
	AnimationKeysInternal();
	~AnimationKeysInternal();

public:
	virtual void AddFloat(int id, float time, float value);
	virtual void AddVector3(int id, float time, const glm::vec3& value);
	virtual void AddQuaternion(int id, float time, const glm::quat& value);

	virtual void Remove(int id, float time);

	virtual void ToKeyframes(std::vector<AnimationFrame>& keyframes);

private:
	struct Key {
		int id;
		float time;
		Variant value;

		bool operator < (const Key& other) const {
			return time < other.time;
		}
	};

	typedef sorted_vector<Key> Keys;
	typedef std::vector<Keys*> Container;

private:
	void InsertKey(int id, const Key& key);
	void RemoveKey(const Key& key);

	int SmoothKeys();
	void SmoothKey(Keys* keys, float time);

	void InitializeKeyframes(int count, std::vector<AnimationFrame> &keyframes);

private:
	Container container_;
};

class AnimationInternal : public IAnimation, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Animation)

public:
	AnimationInternal() : ObjectInternal(ObjectTypeAnimation), time_(0), playing_(false) {}

public:
	virtual void AddClip(const std::string& name, AnimationClip value);
	virtual AnimationClip GetClip(const std::string& name);

	virtual void SetRootTransform(const glm::mat4& value) { rootTransform_ = value; }
	virtual glm::mat4 GetRootTransform() { return rootTransform_; }

	virtual void SetWrapMode(AnimationWrapMode value);

	virtual bool Play(const std::string& name);

	virtual void Update();

	virtual void SetSkeleton(Skeleton value) { skeleton_ = value; }
	virtual Skeleton GetSkeleton() { return skeleton_; }

	struct Key {
		std::string name;
		AnimationClip value;
		bool operator < (const Key& other) const {
			return name < other.name;
		}
	};

private:
	Skeleton skeleton_;
	glm::mat4 rootTransform_;

	float time_;

	bool playing_;
	AnimationClip current_;

	sorted_vector<Key> clips_;
};

class AnimationFrameInternal : public IAnimationFrame, public ObjectInternal {
	DEFINE_FACTORY_METHOD(AnimationFrame)

public:
	AnimationFrameInternal() :ObjectInternal(ObjectTypeAnimationFrame) {}

public:
	virtual void SetTime(float value) { time_ = value; }
	virtual float GetTime() { return time_; }

	virtual void Assign(AnimationFrame other);
	virtual AnimationFrame Lerp(AnimationFrame other, float factor);

	virtual void SetFloat(int id, float value);
	virtual void SetVector3(int id, const glm::vec3& value);
	virtual void SetQuaternion(int id, const glm::quat& value);

	virtual float GetFloat(int id);
	virtual glm::vec3 GetVector3(int id);
	virtual glm::quat GetQuaternion(int id);

private:
	struct Key {
		int id;
		Variant value;

		bool operator < (const Key& other) const {
			return id < other.id;
		}
	};

private:
	void LerpAttribute(AnimationFrame ans, Key& lhs, Key& rhs, float factor);

private:

	float time_;
	sorted_vector<Key> attributes_;
};

class AnimationCurveInternal : public IAnimationCurve, public ObjectInternal {
	DEFINE_FACTORY_METHOD(AnimationCurve)
	
public:
	AnimationCurveInternal() :ObjectInternal(ObjectTypeAnimationCurve) {}

public:
	virtual void SetKeyframes(const std::vector<AnimationFrame>& value) { keyframes_ = value; }
	virtual bool Sample(float time, AnimationFrame& frame);

private:
	int FindInterpolateIndex(float time);
	void SampleLastFrame(AnimationFrame& frame);
	void Lerp(int index, float time, AnimationFrame& frame);

private:
	std::vector<AnimationFrame> keyframes_;
};
