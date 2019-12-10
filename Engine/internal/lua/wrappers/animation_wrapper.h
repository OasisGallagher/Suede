// Warning: this file is generated by wrapper.py.

#pragma once

#include "animation.h"

#include "lua++.h"
#include "tools/string.h"

class SkeletonBone_Wrapper {
	static int NewSkeletonBone(lua_State* L) {
		return Lua::newObject<SkeletonBone>(L);
	}

	static int ToString(lua_State* L) {
		SkeletonBone* _p = Lua::callerPtr<SkeletonBone>(L);

		lua_pushstring(L, String::Format("SkeletonBone@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<SkeletonBone>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewSkeletonBone", NewSkeletonBone });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<SkeletonBone> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<SkeletonBone>(L, metalib, nullptr);
	}
};

class SkeletonNode_Wrapper {
	static int NewSkeletonNode(lua_State* L) {
		return Lua::newObject<SkeletonNode>(L);
	}

	static int ToString(lua_State* L) {
		SkeletonNode* _p = Lua::callerPtr<SkeletonNode>(L);

		lua_pushstring(L, String::Format("SkeletonNode@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<SkeletonNode>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewSkeletonNode", NewSkeletonNode });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<SkeletonNode> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<SkeletonNode>(L, metalib, nullptr);
	}
};

class Skeleton_Wrapper {
	static int NewSkeleton(lua_State* L) {
		return Lua::fromRef(L, make_ref<ISkeleton>());
	}

	static int ToString(lua_State* L) {
		Skeleton& _p = *Lua::callerRefPtr<Skeleton>(L);

		lua_pushstring(L, String::Format("Skeleton@0x%p", _p.get()).c_str());
		return 1;
	}

	// bool AddBone(const SkeletonBone& bone)
	static int AddBone(lua_State* L) {
		Skeleton& _p = *Lua::callerRefPtr<Skeleton>(L);
		SkeletonBone bone = Lua::get<SkeletonBone>(L, 2);
		
		return Lua::push(L, _p->AddBone(bone));
	}

	// void SetBoneToRootMatrix(uint index, const Matrix4& value)
	static int SetBoneToRootMatrix(lua_State* L) {
		Skeleton& _p = *Lua::callerRefPtr<Skeleton>(L);
		Matrix4 value = Lua::get<Matrix4>(L, 3);
		uint index = Lua::get<uint>(L, 2);
		
		_p->SetBoneToRootMatrix(index, value);
		return 0;
	}

	// int GetBoneIndex(const std::string& name)
	static int GetBoneIndex(lua_State* L) {
		Skeleton& _p = *Lua::callerRefPtr<Skeleton>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetBoneIndex(name));
	}

	// int GetBoneCount()
	static int GetBoneCount(lua_State* L) {
		Skeleton& _p = *Lua::callerRefPtr<Skeleton>(L);
		return Lua::push(L, _p->GetBoneCount());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Skeleton>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewSkeleton", NewSkeleton });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<Skeleton> },
			{ "__tostring", ToString }, 
			{ "AddBone", AddBone },
			{ "SetBoneToRootMatrix", SetBoneToRootMatrix },
			{ "GetBoneIndex", GetBoneIndex },
			{ "GetBoneCount", GetBoneCount },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Skeleton>(L, metalib, TypeID<Object>::string());
	}
};

class AnimationClip_Wrapper {
	static int NewAnimationClip(lua_State* L) {
		return Lua::fromRef(L, make_ref<IAnimationClip>());
	}

	static int ToString(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);

		lua_pushstring(L, String::Format("AnimationClip@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetWrapMode(AnimationWrapMode value)
	static int SetWrapMode(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		AnimationWrapMode value = Lua::get<AnimationWrapMode>(L, 2);
		
		_p->SetWrapMode(value);
		return 0;
	}

	// AnimationWrapMode GetWrapMode()
	static int GetWrapMode(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		return Lua::push(L, _p->GetWrapMode());
	}

	// void SetTicksPerSecond(float value)
	static int SetTicksPerSecond(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetTicksPerSecond(value);
		return 0;
	}

	// float GetTicksPerSecond()
	static int GetTicksPerSecond(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		return Lua::push(L, _p->GetTicksPerSecond());
	}

	// void SetDuration(float value)
	static int SetDuration(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetDuration(value);
		return 0;
	}

	// float GetDuration()
	static int GetDuration(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		return Lua::push(L, _p->GetDuration());
	}

	// void SetAnimation(Animation value)
	static int SetAnimation(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		Animation value = Lua::get<Animation>(L, 2);
		
		_p->SetAnimation(value);
		return 0;
	}

	// Animation GetAnimation()
	static int GetAnimation(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		return Lua::push(L, _p->GetAnimation());
	}

	// bool Sample(float time)
	static int Sample(lua_State* L) {
		AnimationClip& _p = *Lua::callerRefPtr<AnimationClip>(L);
		float time = Lua::get<float>(L, 2);
		
		return Lua::push(L, _p->Sample(time));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<AnimationClip>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewAnimationClip", NewAnimationClip });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<AnimationClip> },
			{ "__tostring", ToString }, 
			{ "SetWrapMode", SetWrapMode },
			{ "GetWrapMode", GetWrapMode },
			{ "SetTicksPerSecond", SetTicksPerSecond },
			{ "GetTicksPerSecond", GetTicksPerSecond },
			{ "SetDuration", SetDuration },
			{ "GetDuration", GetDuration },
			{ "SetAnimation", SetAnimation },
			{ "GetAnimation", GetAnimation },
			{ "Sample", Sample },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<AnimationClip>(L, metalib, TypeID<Object>::string());
	}
};

class AnimationState_Wrapper {
	static int NewAnimationState(lua_State* L) {
		return Lua::fromRef(L, make_ref<IAnimationState>());
	}

	static int ToString(lua_State* L) {
		AnimationState& _p = *Lua::callerRefPtr<AnimationState>(L);

		lua_pushstring(L, String::Format("AnimationState@0x%p", _p.get()).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<AnimationState>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewAnimationState", NewAnimationState });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<AnimationState> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<AnimationState>(L, metalib, TypeID<Object>::string());
	}
};

class AnimationKeys_Wrapper {
	static int NewAnimationKeys(lua_State* L) {
		return Lua::fromRef(L, make_ref<IAnimationKeys>());
	}

	static int ToString(lua_State* L) {
		AnimationKeys& _p = *Lua::callerRefPtr<AnimationKeys>(L);

		lua_pushstring(L, String::Format("AnimationKeys@0x%p", _p.get()).c_str());
		return 1;
	}

	// void AddFloat(float time, int id, float value)
	static int AddFloat(lua_State* L) {
		AnimationKeys& _p = *Lua::callerRefPtr<AnimationKeys>(L);
		float value = Lua::get<float>(L, 4);
		int id = Lua::get<int>(L, 3);
		float time = Lua::get<float>(L, 2);
		
		_p->AddFloat(time, id, value);
		return 0;
	}

	// void AddVector3(float time, int id, const Vector3& value)
	static int AddVector3(lua_State* L) {
		AnimationKeys& _p = *Lua::callerRefPtr<AnimationKeys>(L);
		Vector3 value = Lua::get<Vector3>(L, 4);
		int id = Lua::get<int>(L, 3);
		float time = Lua::get<float>(L, 2);
		
		_p->AddVector3(time, id, value);
		return 0;
	}

	// void AddQuaternion(float time, int id, const Quaternion& value)
	static int AddQuaternion(lua_State* L) {
		AnimationKeys& _p = *Lua::callerRefPtr<AnimationKeys>(L);
		Quaternion value = Lua::get<Quaternion>(L, 4);
		int id = Lua::get<int>(L, 3);
		float time = Lua::get<float>(L, 2);
		
		_p->AddQuaternion(time, id, value);
		return 0;
	}

	// void Remove(float time, int id)
	static int Remove(lua_State* L) {
		AnimationKeys& _p = *Lua::callerRefPtr<AnimationKeys>(L);
		int id = Lua::get<int>(L, 3);
		float time = Lua::get<float>(L, 2);
		
		_p->Remove(time, id);
		return 0;
	}

	// void ToKeyframes(std::vector<AnimationFrame>& keyframes)
	static int ToKeyframes(lua_State* L) {
		AnimationKeys& _p = *Lua::callerRefPtr<AnimationKeys>(L);
		std::vector<AnimationFrame> keyframes = Lua::getList<AnimationFrame>(L, 2);
		
		_p->ToKeyframes(keyframes);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<AnimationKeys>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewAnimationKeys", NewAnimationKeys });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<AnimationKeys> },
			{ "__tostring", ToString }, 
			{ "AddFloat", AddFloat },
			{ "AddVector3", AddVector3 },
			{ "AddQuaternion", AddQuaternion },
			{ "Remove", Remove },
			{ "ToKeyframes", ToKeyframes },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<AnimationKeys>(L, metalib, TypeID<Object>::string());
	}
};

class AnimationFrame_Wrapper {
	static int NewAnimationFrame(lua_State* L) {
		return Lua::fromRef(L, make_ref<IAnimationFrame>());
	}

	static int ToString(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);

		lua_pushstring(L, String::Format("AnimationFrame@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetTime(float value)
	static int SetTime(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetTime(value);
		return 0;
	}

	// float GetTime()
	static int GetTime(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		return Lua::push(L, _p->GetTime());
	}

	// void Assign(AnimationFrame other)
	static int Assign(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		AnimationFrame other = Lua::get<AnimationFrame>(L, 2);
		
		_p->Assign(other);
		return 0;
	}

	// void Lerp(AnimationFrame result, AnimationFrame other, float factor)
	static int Lerp(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		float factor = Lua::get<float>(L, 4);
		AnimationFrame other = Lua::get<AnimationFrame>(L, 3);
		AnimationFrame result = Lua::get<AnimationFrame>(L, 2);
		
		_p->Lerp(result, other, factor);
		return 0;
	}

	// void SetFloat(int id, float value)
	static int SetFloat(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		float value = Lua::get<float>(L, 3);
		int id = Lua::get<int>(L, 2);
		
		_p->SetFloat(id, value);
		return 0;
	}

	// void SetVector3(int id, const Vector3& value)
	static int SetVector3(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		Vector3 value = Lua::get<Vector3>(L, 3);
		int id = Lua::get<int>(L, 2);
		
		_p->SetVector3(id, value);
		return 0;
	}

	// void SetQuaternion(int id, const Quaternion& value)
	static int SetQuaternion(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		Quaternion value = Lua::get<Quaternion>(L, 3);
		int id = Lua::get<int>(L, 2);
		
		_p->SetQuaternion(id, value);
		return 0;
	}

	// float GetFloat(int id)
	static int GetFloat(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		int id = Lua::get<int>(L, 2);
		
		return Lua::push(L, _p->GetFloat(id));
	}

	// Vector3 GetVector3(int id)
	static int GetVector3(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		int id = Lua::get<int>(L, 2);
		
		return Lua::push(L, _p->GetVector3(id));
	}

	// Quaternion GetQuaternion(int id)
	static int GetQuaternion(lua_State* L) {
		AnimationFrame& _p = *Lua::callerRefPtr<AnimationFrame>(L);
		int id = Lua::get<int>(L, 2);
		
		return Lua::push(L, _p->GetQuaternion(id));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<AnimationFrame>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewAnimationFrame", NewAnimationFrame });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<AnimationFrame> },
			{ "__tostring", ToString }, 
			{ "SetTime", SetTime },
			{ "GetTime", GetTime },
			{ "Assign", Assign },
			{ "Lerp", Lerp },
			{ "SetFloat", SetFloat },
			{ "SetVector3", SetVector3 },
			{ "SetQuaternion", SetQuaternion },
			{ "GetFloat", GetFloat },
			{ "GetVector3", GetVector3 },
			{ "GetQuaternion", GetQuaternion },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<AnimationFrame>(L, metalib, TypeID<Object>::string());
	}
};

class AnimationCurve_Wrapper {
	static int NewAnimationCurve(lua_State* L) {
		return Lua::fromRef(L, make_ref<IAnimationCurve>());
	}

	static int ToString(lua_State* L) {
		AnimationCurve& _p = *Lua::callerRefPtr<AnimationCurve>(L);

		lua_pushstring(L, String::Format("AnimationCurve@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetKeyframes(const std::vector<AnimationFrame>& value)
	static int SetKeyframes(lua_State* L) {
		AnimationCurve& _p = *Lua::callerRefPtr<AnimationCurve>(L);
		std::vector<AnimationFrame> value = Lua::getList<AnimationFrame>(L, 2);
		
		_p->SetKeyframes(value);
		return 0;
	}

	// bool Sample(float time, AnimationFrame& frame)
	static int Sample(lua_State* L) {
		AnimationCurve& _p = *Lua::callerRefPtr<AnimationCurve>(L);
		AnimationFrame frame = Lua::get<AnimationFrame>(L, 3);
		float time = Lua::get<float>(L, 2);
		
		return Lua::push(L, _p->Sample(time, frame));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<AnimationCurve>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewAnimationCurve", NewAnimationCurve });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<AnimationCurve> },
			{ "__tostring", ToString }, 
			{ "SetKeyframes", SetKeyframes },
			{ "Sample", Sample },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<AnimationCurve>(L, metalib, TypeID<Object>::string());
	}
};

class Animation_Wrapper {
	static int NewAnimation(lua_State* L) {
		return Lua::fromRef(L, make_ref<IAnimation>());
	}

	static int ToString(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);

		lua_pushstring(L, String::Format("Animation@0x%p", _p.get()).c_str());
		return 1;
	}

	// void AddClip(const std::string& name, AnimationClip value)
	static int AddClip(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		AnimationClip value = Lua::get<AnimationClip>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->AddClip(name, value);
		return 0;
	}

	// AnimationClip GetClip(const std::string& name)
	static int GetClip(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetClip(name));
	}

	// void SetSkeleton(Skeleton value)
	static int SetSkeleton(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		Skeleton value = Lua::get<Skeleton>(L, 2);
		
		_p->SetSkeleton(value);
		return 0;
	}

	// Skeleton GetSkeleton()
	static int GetSkeleton(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		return Lua::push(L, _p->GetSkeleton());
	}

	// void SetRootTransform(const Matrix4& value)
	static int SetRootTransform(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		Matrix4 value = Lua::get<Matrix4>(L, 2);
		
		_p->SetRootTransform(value);
		return 0;
	}

	// Matrix4 GetRootTransform()
	static int GetRootTransform(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		return Lua::push(L, _p->GetRootTransform());
	}

	// void SetWrapMode(AnimationWrapMode value)
	static int SetWrapMode(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		AnimationWrapMode value = Lua::get<AnimationWrapMode>(L, 2);
		
		_p->SetWrapMode(value);
		return 0;
	}

	// bool Play(const std::string& name)
	static int Play(lua_State* L) {
		Animation& _p = *Lua::callerRefPtr<Animation>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->Play(name));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Animation>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewAnimation", NewAnimation });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteRefPtr<Animation> },
			{ "__tostring", ToString }, 
			{ "AddClip", AddClip },
			{ "GetClip", GetClip },
			{ "SetSkeleton", SetSkeleton },
			{ "GetSkeleton", GetSkeleton },
			{ "SetRootTransform", SetRootTransform },
			{ "GetRootTransform", GetRootTransform },
			{ "SetWrapMode", SetWrapMode },
			{ "Play", Play },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Animation>(L, metalib, TypeID<Component>::string());
	}
};
