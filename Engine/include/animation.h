#pragma once
#include <vector>
#include "bounds.h"
#include "component.h"
#include "tools/enum.h"

struct SkeletonBone {
	std::string name;

	/**
	 * @brief AABB in bone space.
	 */
	Bounds bounds;

	/**
	 * @brief matrix that transforms from mesh space to bone space in bind pose.
	 */
	Matrix4 meshToBoneMatrix;
};

class Animation;
class AnimationCurve;
class AnimationFrame;

struct SkeletonNode {
	std::string name;
	Matrix4 matrix;
	ref_ptr<AnimationCurve> curve;

	SkeletonNode* parent;
	std::vector<SkeletonNode*> children;
};

class Skeleton : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Skeleton)
	SUEDE_DECLARE_IMPLEMENTATION(Skeleton)

public:
	Skeleton();

public:
	bool AddBone(const SkeletonBone& bone);
	SkeletonBone* GetBone(uint index);
	SkeletonBone* GetBone(const std::string& name);

	SkeletonNode* CreateNode(const std::string& name, const Matrix4& matrix, AnimationCurve* curve);
	void AddNode(SkeletonNode* parent, SkeletonNode* child);
	SkeletonNode* GetRootNode();
	
	void SetBoneToRootMatrix(uint index, const Matrix4& value);
	Matrix4* GetBoneToRootMatrices();

	int GetBoneIndex(const std::string& name);
	int GetBoneCount();
};

BETTER_ENUM(AnimationWrapMode, int, 
	Once,
	Loop,
	PingPong,
	ClampForever
)

class SUEDE_API AnimationClip : public Object {
	SUEDE_DEFINE_METATABLE_NAME(AnimationClip)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationClip)

public:
	AnimationClip();

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

	void SetAnimation(Animation* value);
	Animation* GetAnimation();

	bool Sample(float time);
};

class SUEDE_API AnimationState : public Object {
	SUEDE_DEFINE_METATABLE_NAME(AnimationState)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationState)

public:
	AnimationState();
};

class SUEDE_API AnimationKeys : public Object {
	SUEDE_DEFINE_METATABLE_NAME(AnimationKeys)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationKeys)

public:
	AnimationKeys();

public:
	void AddFloat(float time, int id, float value);
	void AddVector3(float time, int id, const Vector3& value);
	void AddQuaternion(float time, int id, const Quaternion& value);

	void Remove(float time, int id);

	void ToKeyframes(std::vector<ref_ptr<AnimationFrame>>& keyframes);
};

enum {
	FrameKeyPosition,
	FrameKeyRotation,
	FrameKeyScale,

	FrameKeyUser = 4,
	FrameKeyMaxCount = 8,
};

class SUEDE_API AnimationFrame : public Object {
	SUEDE_DEFINE_METATABLE_NAME(AnimationFrame)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationFrame)

public:
	AnimationFrame();
	~AnimationFrame() {}

public:
	void SetTime(float value);
	float GetTime();

	void Assign(AnimationFrame* other);
	void Lerp(AnimationFrame* result, AnimationFrame* other, float factor);

	void SetFloat(int id, float value);
	void SetVector3(int id, const Vector3& value);
	void SetQuaternion(int id, const Quaternion& value);

	float GetFloat(int id);
	Vector3 GetVector3(int id);
	Quaternion GetQuaternion(int id);
};

class SUEDE_API AnimationCurve : public Object {
	SUEDE_DEFINE_METATABLE_NAME(AnimationCurve)
	SUEDE_DECLARE_IMPLEMENTATION(AnimationCurve)

public:
	AnimationCurve();

public:
	void SetKeys(AnimationKeys* value);

	/**
	 * @returns whether time reaches the last frame.
	 */
	bool Sample(float time, AnimationFrame*& frame);
};

class SUEDE_API Animation : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Animation)
	SUEDE_DECLARE_IMPLEMENTATION(Animation)

public:
	Animation();
	~Animation() {}

public:
	void AddClip(const std::string& name, AnimationClip* value);
	AnimationClip* GetClip(const std::string& name);

	void SetSkeleton(Skeleton* value);
	Skeleton* GetSkeleton();
	
	void SetRootTransform(const Matrix4& value);
	Matrix4 GetRootTransform();

	void SetWrapMode(AnimationWrapMode value);

	bool Play(const std::string& name);
};
