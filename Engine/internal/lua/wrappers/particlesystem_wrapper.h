// Warning: this file is generated by wrapper.py.

#pragma once

#include "particlesystem.h"

#include "lua++.h"
#include "tools/string.h"

class ParticleBurst_Wrapper {
	static int NewParticleBurst(lua_State* L) {
		return Lua::newObject<ParticleBurst>(L);
	}

	static int ToString(lua_State* L) {
		ParticleBurst* _p = Lua::callerPtr<ParticleBurst>(L);

		lua_pushstring(L, String::Format("ParticleBurst@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<ParticleBurst>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewParticleBurst", NewParticleBurst });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<ParticleBurst> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<ParticleBurst>(L, metalib, nullptr);
	}
};

class Particle_Wrapper {
	static int NewParticle(lua_State* L) {
		return Lua::newObject<Particle>(L);
	}

	static int ToString(lua_State* L) {
		Particle* _p = Lua::callerPtr<Particle>(L);

		lua_pushstring(L, String::Format("Particle@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Particle>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewParticle", NewParticle });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Particle> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Particle>(L, metalib, nullptr);
	}
};

class ParticleEmitter_Wrapper {
	static int ToString(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);

		lua_pushstring(L, String::Format("ParticleEmitter@0x%p", _p).c_str());
		return 1;
	}

	// void SetRate(uint value)
	static int SetRate(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		uint value = Lua::get<uint>(L, 2);
		
		_p->SetRate(value);
		return 0;
	}

	// uint GetRate()
	static int GetRate(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		return Lua::push(L, _p->GetRate());
	}

	// void SetStartDuration(float value)
	static int SetStartDuration(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetStartDuration(value);
		return 0;
	}

	// float GetStartDuration()
	static int GetStartDuration(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		return Lua::push(L, _p->GetStartDuration());
	}

	// void SetStartSize(float value)
	static int SetStartSize(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetStartSize(value);
		return 0;
	}

	// float GetStartSize()
	static int GetStartSize(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		return Lua::push(L, _p->GetStartSize());
	}

	// void SetStartVelocity(const Vector3& value)
	static int SetStartVelocity(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		Vector3 value = Lua::get<Vector3>(L, 2);
		
		_p->SetStartVelocity(value);
		return 0;
	}

	// Vector3 GetStartVelocity()
	static int GetStartVelocity(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		return Lua::push(L, _p->GetStartVelocity());
	}

	// void SetStartColor(const Vector4& value)
	static int SetStartColor(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		Vector4 value = Lua::get<Vector4>(L, 2);
		
		_p->SetStartColor(value);
		return 0;
	}

	// Vector4 GetStartColor()
	static int GetStartColor(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		return Lua::push(L, _p->GetStartColor());
	}

	// void AddBurst(const ParticleBurst& value)
	static int AddBurst(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		ParticleBurst value = Lua::get<ParticleBurst>(L, 2);
		
		_p->AddBurst(value);
		return 0;
	}

	// void SetBurst(int i, const ParticleBurst& value)
	static int SetBurst(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		ParticleBurst value = Lua::get<ParticleBurst>(L, 3);
		int i = Lua::get<int>(L, 2);
		
		_p->SetBurst(i, value);
		return 0;
	}

	// ParticleBurst GetBurst(int i)
	static int GetBurst(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		int i = Lua::get<int>(L, 2);
		
		return Lua::push(L, _p->GetBurst(i));
	}

	// void RemoveBurst(int i)
	static int RemoveBurst(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		int i = Lua::get<int>(L, 2);
		
		_p->RemoveBurst(i);
		return 0;
	}

	// int GetBurstCount()
	static int GetBurstCount(lua_State* L) {
		ParticleEmitter* _p = Lua::callerPtr<ParticleEmitter>(L);
		return Lua::push(L, _p->GetBurstCount());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<ParticleEmitter>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<ParticleEmitter> },
			{ "__tostring", ToString }, 
			{ "SetRate", SetRate },
			{ "GetRate", GetRate },
			{ "SetStartDuration", SetStartDuration },
			{ "GetStartDuration", GetStartDuration },
			{ "SetStartSize", SetStartSize },
			{ "GetStartSize", GetStartSize },
			{ "SetStartVelocity", SetStartVelocity },
			{ "GetStartVelocity", GetStartVelocity },
			{ "SetStartColor", SetStartColor },
			{ "GetStartColor", GetStartColor },
			{ "AddBurst", AddBurst },
			{ "SetBurst", SetBurst },
			{ "GetBurst", GetBurst },
			{ "RemoveBurst", RemoveBurst },
			{ "GetBurstCount", GetBurstCount },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<ParticleEmitter>(L, metalib, TypeID<Object>::string());
	}
};

class SphereParticleEmitter_Wrapper {
	static int NewSphereParticleEmitter(lua_State* L) {
		return Lua::newObject<SphereParticleEmitter>(L);
	}

	static int ToString(lua_State* L) {
		SphereParticleEmitter* _p = Lua::callerPtr<SphereParticleEmitter>(L);

		lua_pushstring(L, String::Format("SphereParticleEmitter@0x%p", _p).c_str());
		return 1;
	}

	// void SetRadius(float value)
	static int SetRadius(lua_State* L) {
		SphereParticleEmitter* _p = Lua::callerPtr<SphereParticleEmitter>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetRadius(value);
		return 0;
	}

	// float GetRadius()
	static int GetRadius(lua_State* L) {
		SphereParticleEmitter* _p = Lua::callerPtr<SphereParticleEmitter>(L);
		return Lua::push(L, _p->GetRadius());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<SphereParticleEmitter>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewSphereParticleEmitter", NewSphereParticleEmitter });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<SphereParticleEmitter> },
			{ "__tostring", ToString }, 
			{ "SetRadius", SetRadius },
			{ "GetRadius", GetRadius },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<SphereParticleEmitter>(L, metalib, TypeID<ParticleEmitter>::string());
	}
};

class ParticleAnimator_Wrapper {
	static int NewParticleAnimator(lua_State* L) {
		return Lua::newObject<ParticleAnimator>(L);
	}

	static int ToString(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);

		lua_pushstring(L, String::Format("ParticleAnimator@0x%p", _p).c_str());
		return 1;
	}

	// void SetForce(const Vector3& value)
	static int SetForce(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		Vector3 value = Lua::get<Vector3>(L, 2);
		
		_p->SetForce(value);
		return 0;
	}

	// Vector3 GetForce()
	static int GetForce(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		return Lua::push(L, _p->GetForce());
	}

	// void SetRandomForce(const Vector3& value)
	static int SetRandomForce(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		Vector3 value = Lua::get<Vector3>(L, 2);
		
		_p->SetRandomForce(value);
		return 0;
	}

	// Vector3 GetRandomForce()
	static int GetRandomForce(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		return Lua::push(L, _p->GetRandomForce());
	}

	// void SetGravityScale(float value)
	static int SetGravityScale(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetGravityScale(value);
		return 0;
	}

	// float GetGravityScale()
	static int GetGravityScale(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		return Lua::push(L, _p->GetGravityScale());
	}

	// void Update(Particle& particle)
	static int Update(lua_State* L) {
		ParticleAnimator* _p = Lua::callerPtr<ParticleAnimator>(L);
		Particle particle = Lua::get<Particle>(L, 2);
		
		_p->Update(particle);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<ParticleAnimator>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewParticleAnimator", NewParticleAnimator });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<ParticleAnimator> },
			{ "__tostring", ToString }, 
			{ "SetForce", SetForce },
			{ "GetForce", GetForce },
			{ "SetRandomForce", SetRandomForce },
			{ "GetRandomForce", GetRandomForce },
			{ "SetGravityScale", SetGravityScale },
			{ "GetGravityScale", GetGravityScale },
			{ "Update", Update },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<ParticleAnimator>(L, metalib, TypeID<Object>::string());
	}
};

class ParticleSystem_Wrapper {
	static int NewParticleSystem(lua_State* L) {
		return Lua::newObject<ParticleSystem>(L);
	}

	static int ToString(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);

		lua_pushstring(L, String::Format("ParticleSystem@0x%p", _p).c_str());
		return 1;
	}

	// void SetDuration(float value)
	static int SetDuration(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetDuration(value);
		return 0;
	}

	// float GetDuration()
	static int GetDuration(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		return Lua::push(L, _p->GetDuration());
	}

	// void SetLooping(bool value)
	static int SetLooping(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		bool value = Lua::get<bool>(L, 2);
		
		_p->SetLooping(value);
		return 0;
	}

	// bool GetLooping()
	static int GetLooping(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		return Lua::push(L, _p->GetLooping());
	}

	// void SetStartDelay(float value)
	static int SetStartDelay(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetStartDelay(value);
		return 0;
	}

	// float GetStartDelay()
	static int GetStartDelay(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		return Lua::push(L, _p->GetStartDelay());
	}

	// void SetMaxParticles(uint value)
	static int SetMaxParticles(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		uint value = Lua::get<uint>(L, 2);
		
		_p->SetMaxParticles(value);
		return 0;
	}

	// uint GetMaxParticles()
	static int GetMaxParticles(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		return Lua::push(L, _p->GetMaxParticles());
	}

	// uint GetParticlesCount()
	static int GetParticlesCount(lua_State* L) {
		ParticleSystem* _p = Lua::callerPtr<ParticleSystem>(L);
		return Lua::push(L, _p->GetParticlesCount());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<ParticleSystem>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewParticleSystem", NewParticleSystem });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<ParticleSystem> },
			{ "__tostring", ToString }, 
			{ "SetDuration", SetDuration },
			{ "GetDuration", GetDuration },
			{ "SetLooping", SetLooping },
			{ "GetLooping", GetLooping },
			{ "SetStartDelay", SetStartDelay },
			{ "GetStartDelay", GetStartDelay },
			{ "SetMaxParticles", SetMaxParticles },
			{ "GetMaxParticles", GetMaxParticles },
			{ "GetParticlesCount", GetParticlesCount },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<ParticleSystem>(L, metalib, TypeID<Component>::string());
	}
};
