// Warning: this file is generated by wrapper.py.

#pragma once

#include "camera.h"

#include "lua++.h"
#include "tools/string.h"

class Camera_Wrapper {
	static int ToString(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		lua_pushstring(L, String::Format("Camera@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetDepth(int value)
	static int SetDepth(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		int value = Lua::get<int>(L, 2);
		_p->SetDepth(value);
		return 0;
	}

	// int GetDepth()
	static int GetDepth(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetDepth());
	}

	// bool GetPerspective()
	static int GetPerspective(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetPerspective());
	}

	// void SetPerspective(bool value)
	static int SetPerspective(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		bool value = Lua::get<bool>(L, 2);
		_p->SetPerspective(value);
		return 0;
	}

	// float GetOrthographicSize()
	static int GetOrthographicSize(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetOrthographicSize());
	}

	// void SetOrthographicSize(float value)
	static int SetOrthographicSize(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		float value = Lua::get<float>(L, 2);
		_p->SetOrthographicSize(value);
		return 0;
	}

	// void SetClearType(ClearType value)
	static int SetClearType(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		ClearType value = Lua::get<ClearType>(L, 2);
		_p->SetClearType(value);
		return 0;
	}

	// ClearType GetClearType()
	static int GetClearType(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetClearType());
	}

	// void SetRenderPath(RenderPath value)
	static int SetRenderPath(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		RenderPath value = Lua::get<RenderPath>(L, 2);
		_p->SetRenderPath(value);
		return 0;
	}

	// RenderPath GetRenderPath()
	static int GetRenderPath(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetRenderPath());
	}

	// void SetDepthTextureMode(DepthTextureMode value)
	static int SetDepthTextureMode(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		DepthTextureMode value = Lua::get<DepthTextureMode>(L, 2);
		_p->SetDepthTextureMode(value);
		return 0;
	}

	// DepthTextureMode GetDepthTextureMode()
	static int GetDepthTextureMode(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetDepthTextureMode());
	}

	// void SetClearColor(const Color& value)
	static int SetClearColor(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		Color value = Lua::get<Color>(L, 2);
		_p->SetClearColor(value);
		return 0;
	}

	// Color GetClearColor()
	static int GetClearColor(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetClearColor());
	}

	// void SetTargetTexture(RenderTexture value)
	static int SetTargetTexture(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		RenderTexture value = Lua::get<RenderTexture>(L, 2);
		_p->SetTargetTexture(value);
		return 0;
	}

	// RenderTexture GetTargetTexture()
	static int GetTargetTexture(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetTargetTexture());
	}

	// void SetAspect(float value)
	static int SetAspect(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		float value = Lua::get<float>(L, 2);
		_p->SetAspect(value);
		return 0;
	}

	// float GetAspect()
	static int GetAspect(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetAspect());
	}

	// void SetNearClipPlane(float value)
	static int SetNearClipPlane(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		float value = Lua::get<float>(L, 2);
		_p->SetNearClipPlane(value);
		return 0;
	}

	// float GetNearClipPlane()
	static int GetNearClipPlane(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetNearClipPlane());
	}

	// void SetFarClipPlane(float value)
	static int SetFarClipPlane(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		float value = Lua::get<float>(L, 2);
		_p->SetFarClipPlane(value);
		return 0;
	}

	// float GetFarClipPlane()
	static int GetFarClipPlane(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetFarClipPlane());
	}

	// void SetFieldOfView(float value)
	static int SetFieldOfView(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		float value = Lua::get<float>(L, 2);
		_p->SetFieldOfView(value);
		return 0;
	}

	// float GetFieldOfView()
	static int GetFieldOfView(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->GetFieldOfView());
	}

	// void SetRect(const Rect& value)
	static int SetRect(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		Rect value = Lua::get<Rect>(L, 2);
		_p->SetRect(value);
		return 0;
	}

	// void GetVisibleGameObjects(std::vector<GameObject>& gameObjects)
	static int GetVisibleGameObjects(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		std::vector<GameObject> gameObjects = Lua::getList<GameObject>(L, 2);
		_p->GetVisibleGameObjects(gameObjects);
		return 0;
	}

	// glm::vec3 WorldToScreenPoint(const glm::vec3& position)
	static int WorldToScreenPoint(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		glm::vec3 position = Lua::get<glm::vec3>(L, 2);
		return Lua::push(L, _p->WorldToScreenPoint(position));
	}

	// glm::vec3 ScreenToWorldPoint(const glm::vec3& position)
	static int ScreenToWorldPoint(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		glm::vec3 position = Lua::get<glm::vec3>(L, 2);
		return Lua::push(L, _p->ScreenToWorldPoint(position));
	}

	// Texture2D Capture()
	static int Capture(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		return Lua::push(L, _p->Capture());
	}

	// void Render()
	static int Render(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		_p->Render();
		return 0;
	}

	// void OnBeforeWorldDestroyed()
	static int OnBeforeWorldDestroyed(lua_State* L) {
		Camera& _p = *Lua::callerSharedPtr<Camera>(L);
		_p->OnBeforeWorldDestroyed();
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Camera>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Camera> },
			{ "__tostring", ToString }, 
			{ "SetDepth", SetDepth },
			{ "GetDepth", GetDepth },
			{ "GetPerspective", GetPerspective },
			{ "SetPerspective", SetPerspective },
			{ "GetOrthographicSize", GetOrthographicSize },
			{ "SetOrthographicSize", SetOrthographicSize },
			{ "SetClearType", SetClearType },
			{ "GetClearType", GetClearType },
			{ "SetRenderPath", SetRenderPath },
			{ "GetRenderPath", GetRenderPath },
			{ "SetDepthTextureMode", SetDepthTextureMode },
			{ "GetDepthTextureMode", GetDepthTextureMode },
			{ "SetClearColor", SetClearColor },
			{ "GetClearColor", GetClearColor },
			{ "SetTargetTexture", SetTargetTexture },
			{ "GetTargetTexture", GetTargetTexture },
			{ "SetAspect", SetAspect },
			{ "GetAspect", GetAspect },
			{ "SetNearClipPlane", SetNearClipPlane },
			{ "GetNearClipPlane", GetNearClipPlane },
			{ "SetFarClipPlane", SetFarClipPlane },
			{ "GetFarClipPlane", GetFarClipPlane },
			{ "SetFieldOfView", SetFieldOfView },
			{ "GetFieldOfView", GetFieldOfView },
			{ "SetRect", SetRect },
			{ "GetVisibleGameObjects", GetVisibleGameObjects },
			{ "WorldToScreenPoint", WorldToScreenPoint },
			{ "ScreenToWorldPoint", ScreenToWorldPoint },
			{ "Capture", Capture },
			{ "Render", Render },
			{ "OnBeforeWorldDestroyed", OnBeforeWorldDestroyed },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Camera>(L, metalib, TypeID<Component>::string());
	}
};

class CameraUtility_Wrapper {
	static int ToString(lua_State* L) {
		CameraUtility* _p = Lua::callerPtr<CameraUtility>(L);
		lua_pushstring(L, String::Format("CameraUtility@0x%p", _p).c_str());
		return 1;
	}

	static int ToStringStatic(lua_State* L) {
		lua_pushstring(L, "static CameraUtility");
		return 1;
	}

	static int CameraUtilityStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {
			{ "SetMain", SetMain },
			{ "GetMain", GetMain },
			{ "OnPreRender", OnPreRender },
			{ "OnPostRender", OnPostRender },
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
		// static void SetMain(Camera value)
	static int SetMain(lua_State* L) {
		Camera value = Lua::get<Camera>(L, 2);
		CameraUtility::SetMain(value);
		return 0;
	}

	// static Camera GetMain()
	static int GetMain(lua_State* L) {
		return Lua::push(L, CameraUtility::GetMain());
	}

	// static void OnPreRender()
	static int OnPreRender(lua_State* L) {
		CameraUtility::OnPreRender();
		return 0;
	}

	// static void OnPostRender()
	static int OnPostRender(lua_State* L) {
		CameraUtility::OnPostRender();
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<CameraUtility>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		fields.push_back(luaL_Reg{ "CameraUtility", CameraUtilityStatic });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<CameraUtility> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<CameraUtility>(L, metalib, nullptr);
	}
};
