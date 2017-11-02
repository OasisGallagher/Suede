#pragma once
#include <vector>
#include "object.h"

class ISkeleton;
class IAnimation;
class IAnimationClip;
class IAnimationKeys;
class IAnimationCurve;
class IAnimationState;
class IAnimationFrame;

typedef std::shared_ptr<ISkeleton> Skeleton;
typedef std::shared_ptr<IAnimation> Animation;
typedef std::shared_ptr<IAnimationClip> AnimationClip;
typedef std::shared_ptr<IAnimationKeys> AnimationKeys;
typedef std::shared_ptr<IAnimationCurve> AnimationCurve;
typedef std::shared_ptr<IAnimationState> AnimationState;
typedef std::shared_ptr<IAnimationFrame> AnimationFrame;

struct SkeletonBone {
	std::string name;
	glm::mat4 localToBoneSpaceMatrix;
};

struct SkeletonNode {
	std::string name;
	glm::mat4 matrix;
	AnimationCurve curve;

	SkeletonNode* parent;
	std::vector<SkeletonNode*> children;
};

class ISkeleton : virtual public IObject {
public:
	virtual bool AddBone(const SkeletonBone& bone) = 0;
	virtual SkeletonBone* GetBone(int index) = 0;
	virtual SkeletonBone* GetBone(const std::string& name) = 0;

	virtual SkeletonNode* CreateNode(const std::string& name, const glm::mat4& matrix, AnimationCurve curve) = 0;
	virtual void AddNode(SkeletonNode* parent, SkeletonNode* child) = 0;
	virtual SkeletonNode* GetRootNode() = 0;
	
	virtual void SetBoneToRootSpaceMatrix(int index, const glm::mat4& value) = 0;
	virtual glm::mat4* GetBoneToRootSpaceMatrices() = 0;

	virtual int GetBoneIndex(const std::string& name) = 0;
	virtual int GetBoneCount() = 0;
};

enum AnimationWrapMode {
	AnimationWrapModeOnce,
	AnimationWrapModeLoop,
	AnimationWrapModePingPong,
	AnimationWrapModeClampForever,
};

class ENGINE_EXPORT IAnimationClip : virtual public IObject {
public:
	virtual void SetWrapMode(AnimationWrapMode value) = 0;
	virtual AnimationWrapMode GetWrapMode() = 0;

	virtual void SetTicksPerSecond(float value) = 0;
	virtual float GetTicksPerSecond() = 0;

	/**
	 * @param value: Duration of the animation in ticks.
	 */
	virtual void SetDuration(float value) = 0;
	virtual float GetDuration() = 0;

	virtual void SetAnimation(Animation value) = 0;
	virtual Animation GetAnimation() = 0;

	virtual bool Sample(float time) = 0;
};

class ENGINE_EXPORT IAnimationState : virtual public IObject {
};

class ENGINE_EXPORT IAnimationKeys : virtual public IObject {
public:
	virtual void AddFloat(int id, float time, float value) = 0;
	virtual void AddVector3(int id, float time, const glm::vec3& value) = 0;
	virtual void AddQuaternion(int id, float time, const glm::quat& value) = 0;

	virtual void Remove(int id, float time) = 0;

	virtual void ToKeyframes(std::vector<AnimationFrame>& keyframes) = 0;
};

enum {
	FrameKeyPosition,
	FrameKeyRotation,
	FrameKeyScale,

	FrameKeyUser = 4,
	FrameKeyMaxCount = 8,
};

class ENGINE_EXPORT IAnimationFrame : virtual public IObject {
public:
	virtual void SetTime(float value) = 0;
	virtual float GetTime() = 0;

	virtual void Assign(AnimationFrame other) = 0;
	virtual AnimationFrame Lerp(AnimationFrame other, float factor) = 0;

	virtual void SetFloat(int id, float value) = 0;
	virtual void SetVector3(int id, const glm::vec3& value) = 0;
	virtual void SetQuaternion(int id, const glm::quat& value) = 0;

	virtual float GetFloat(int id) = 0;
	virtual glm::vec3 GetVector3(int id) = 0;
	virtual glm::quat GetQuaternion(int id) = 0;
};

class ENGINE_EXPORT IAnimationCurve : virtual public IObject {
public:
	virtual void SetKeyframes(const std::vector<AnimationFrame>& value) = 0;
	virtual bool Sample(float time, AnimationFrame& frame) = 0;
};

class ENGINE_EXPORT IAnimation : virtual public IObject {
public:
	virtual void AddClip(const std::string& name, AnimationClip value) = 0;
	virtual AnimationClip GetClip(const std::string& name) = 0;

	virtual void SetSkeleton(Skeleton value) = 0;
	virtual Skeleton GetSkeleton() = 0;
	
	virtual void SetRootTransform(const glm::mat4& value) = 0;
	virtual glm::mat4 GetRootTransform() = 0;

	virtual void SetWrapMode(AnimationWrapMode value) = 0;

	virtual bool Play(const std::string& name) = 0;

	virtual void Update() = 0;
};
