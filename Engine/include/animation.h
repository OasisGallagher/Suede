#pragma once
#include <vector>
#include "bounds.h"
#include "component.h"
#include "tools/enum.h"

SUEDE_DEFINE_OBJECT_POINTER(Skeleton)
SUEDE_DEFINE_OBJECT_POINTER(Animation)
SUEDE_DEFINE_OBJECT_POINTER(AnimationClip)
SUEDE_DEFINE_OBJECT_POINTER(AnimationKeys)
SUEDE_DEFINE_OBJECT_POINTER(AnimationCurve)
SUEDE_DEFINE_OBJECT_POINTER(AnimationState)
SUEDE_DEFINE_OBJECT_POINTER(AnimationFrame)

SUEDE_DECLARE_OBJECT_CREATER(Skeleton)
SUEDE_DECLARE_OBJECT_CREATER(AnimationClip)
SUEDE_DECLARE_OBJECT_CREATER(AnimationKeys)
SUEDE_DECLARE_OBJECT_CREATER(AnimationCurve)
SUEDE_DECLARE_OBJECT_CREATER(AnimationState)
SUEDE_DECLARE_OBJECT_CREATER(AnimationFrame)

struct SkeletonBone {
	std::string name;

	/**
	 * @brief AABB in bone space.
	 */
	Bounds bounds;

	/**
	 * @brief matrix that transforms from mesh space to bone space in bind pose.
	 */
	glm::mat4 meshToBoneMatrix;
};

struct SkeletonNode {
	std::string name;
	glm::mat4 matrix;
	AnimationCurve curve;

	SkeletonNode* parent;
	std::vector<SkeletonNode*> children;
};

class ISkeleton : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(Skeleton)
	SUEDE_DECLARE_IMPLEMENTATION(Skeleton)

public:
	ISkeleton();

public:
	bool AddBone(const SkeletonBone& bone);
	SkeletonBone* GetBone(uint index);
	SkeletonBone* GetBone(const std::string& name);

	SkeletonNode* CreateNode(const std::string& name, const glm::mat4& matrix, AnimationCurve curve);
	void AddNode(SkeletonNode* parent, SkeletonNode* child);
	SkeletonNode* GetRootNode();
	
	void SetBoneToRootMatrix(uint index, const glm::mat4& value);
	glm::mat4* GetBoneToRootMatrices();

	int GetBoneIndex(const std::string& name);
	int GetBoneCount();
};

BETTER_ENUM(AnimationWrapMode, int, 
	Once,
	Loop,
	PingPong,
	ClampForever
)

class SUEDE_API IAnimationClip : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(AnimationClip)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationClip)

public:
	IAnimationClip();

public:
	void SetWrapMode(AnimationWrapMode value);
	AnimationWrapMode GetWrapMode();

	void SetTicksPerSecond(float value);
	float GetTicksPerSecond();

	/**
	 * @param value: duration of the animation in ticks.
	 */
	void SetDuration(float value);
	float GetDuration();

	void SetAnimation(Animation value);
	Animation GetAnimation();

	bool Sample(float time);
};

class SUEDE_API IAnimationState : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(AnimationState)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationState)

public:
	IAnimationState();
};

class SUEDE_API IAnimationKeys : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(AnimationKeys)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationKeys)

public:
	IAnimationKeys();

public:
	void AddFloat(float time, int id, float value);
	void AddVector3(float time, int id, const glm::vec3& value);
	void AddQuaternion(float time, int id, const glm::quat& value);

	void Remove(float time, int id);

	void ToKeyframes(std::vector<AnimationFrame>& keyframes);
};

enum {
	FrameKeyPosition,
	FrameKeyRotation,
	FrameKeyScale,

	FrameKeyUser = 4,
	FrameKeyMaxCount = 8,
};

class SUEDE_API IAnimationFrame : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(AnimationFrame)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationFrame)

public:
	IAnimationFrame();

public:
	void SetTime(float value);
	float GetTime();

	void Assign(AnimationFrame other);
	void Lerp(AnimationFrame result, AnimationFrame other, float factor);

	void SetFloat(int id, float value);
	void SetVector3(int id, const glm::vec3& value);
	void SetQuaternion(int id, const glm::quat& value);

	float GetFloat(int id);
	glm::vec3 GetVector3(int id);
	glm::quat GetQuaternion(int id);
};

class SUEDE_API IAnimationCurve : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(AnimationCurve)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationCurve)

public:
	IAnimationCurve();

public:
	void SetKeyframes(const std::vector<AnimationFrame>& value);

	/**
	 * @returns whether time reaches the last frame.
	 */
	bool Sample(float time, AnimationFrame& frame);
};

class SUEDE_API IAnimation : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Animation)
	SUEDE_DECLARE_IMPLEMENTATION(Animation)

public:
	IAnimation();

public:
	void AddClip(const std::string& name, AnimationClip value);
	AnimationClip GetClip(const std::string& name);

	void SetSkeleton(Skeleton value);
	Skeleton GetSkeleton();
	
	void SetRootTransform(const glm::mat4& value);
	glm::mat4 GetRootTransform();

	void SetWrapMode(AnimationWrapMode value);

	bool Play(const std::string& name);
};
