#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
	
	virtual void SetBoneToRootMatrix(uint index, const glm::mat4& value);
	virtual glm::mat4* GetBoneToRootMatrices() { return boneToRootMatrices_; }

	virtual int GetBoneIndex(const std::string& name);
	virtual int GetBoneCount() { return current_; }

	virtual SkeletonNode* CreateNode(const std::string& name, const glm::mat4& matrix, AnimationCurve curve);
	virtual void AddNode(SkeletonNode* parent, SkeletonNode* child);
	virtual SkeletonNode* GetRootNode() { return root_; }

private:
	void DestroyNodeHierarchy(SkeletonNode*& node);

private:
	int current_;
	SkeletonBone bones_[MAX_BONE_COUNT];
	glm::mat4 boneToRootMatrices_[MAX_BONE_COUNT];

	SkeletonNode* root_;

	std::map<std::string, int> boneMap_;
};

class AnimationClipInternal : public ObjectInternal {
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
	AnimationFrame frame_;
	AnimationWrapMode wrapMode_;
	std::weak_ptr<Animation::element_type> animation_;
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
	virtual void AddVector3(float time, int id, const glm::vec3& value);
	virtual void AddQuaternion(float time, int id, const glm::quat& value);

	virtual void Remove(float time, int id);

	virtual void ToKeyframes(std::vector<AnimationFrame>& keyframes);

private:
	struct FloatCamparer {
		bool operator()(float lhs, float rhs) const;
	};

	struct Key {
		int id;
		Variant value;
	};

	typedef std::map<float, Key, FloatCamparer> Keys;
	typedef std::vector<Keys*> KeysContainer;

private:
	void InsertKey(float time, const Key& key);
	void RemoveKey(int id, float time);

	int SmoothKeys();
	void SmoothKey(Keys* keys, float time);

	void InitializeKeyframes(int count, std::vector<AnimationFrame> &keyframes);

private:
	KeysContainer container_;
};

class AnimationInternal : public ComponentInternal {
public:
	AnimationInternal() : ComponentInternal(ObjectType::Animation), time_(0), playing_(false) {}

public:
	virtual void CullingUpdate();

public:
	virtual void AddClip(Animation self, const std::string& name, AnimationClip value);
	virtual AnimationClip GetClip(const std::string& name);

	virtual void SetRootTransform(const glm::mat4& value) { rootTransform_ = value; }
	virtual glm::mat4 GetRootTransform() { return rootTransform_; }

	virtual void SetWrapMode(AnimationWrapMode value);

	virtual bool Play(const std::string& name);

	virtual void SetSkeleton(Skeleton value) { skeleton_ = value; }
	virtual Skeleton GetSkeleton() { return skeleton_; }

	virtual int GetUpdateStrategy() { return UpdateStrategyCulling; }

private:
	Skeleton skeleton_;
	glm::mat4 rootTransform_;

	float time_;

	bool playing_;
	AnimationClip current_;

	typedef std::map<std::string, AnimationClip> ClipContainer;
	ClipContainer clips_;
};

class AnimationFrameInternal : public ObjectInternal {
public:
	AnimationFrameInternal() :ObjectInternal(ObjectType::AnimationFrame), time_(0) {}

public:
	virtual void SetTime(float value) { time_ = value; }
	virtual float GetTime() { return time_; }

	virtual void Assign(AnimationFrame other);
	virtual void Lerp(AnimationFrame result, AnimationFrame other, float factor);

	virtual void SetFloat(int id, float value);
	virtual void SetVector3(int id, const glm::vec3& value);
	virtual void SetQuaternion(int id, const glm::quat& value);

	virtual float GetFloat(int id);
	virtual glm::vec3 GetVector3(int id);
	virtual glm::quat GetQuaternion(int id);

private:
	void LerpAttribute(AnimationFrame ans, int id, const Variant& lhs, const Variant& rhs, float factor);

private:
	float time_;

	typedef std::map<int, Variant> AttributeContainer;
	AttributeContainer attributes_;
};

class AnimationCurveInternal : public ObjectInternal {
public:
	AnimationCurveInternal() :ObjectInternal(ObjectType::AnimationCurve) {}

public:
	virtual void SetKeyframes(const std::vector<AnimationFrame>& value) { keyframes_ = value; }
	virtual bool Sample(float time, AnimationFrame& frame);

private:
	int FindInterpolateIndex(float time);
	void SampleEndFrame(AnimationFrame& frame);
	void Lerp(int index, float time, AnimationFrame& frame);

private:
	std::vector<AnimationFrame> keyframes_;
};
