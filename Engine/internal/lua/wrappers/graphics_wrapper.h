// Warning: this file is generated by wrapper.py.

#pragma once

#include "lua++.h"
#include "graphics.h"

class Graphics_Wrapper {
	static int GraphicsInstance(lua_State* L) {
		return Lua::reference<Graphics>(L);
	}

	// void SetShadingMode(ShadingMode value)
	static int SetShadingMode(lua_State* L) {
		Graphics* _p = Graphics::instance();
		ShadingMode value = Lua::get<ShadingMode>(L, 2);
		_p->SetShadingMode(value);
		return 0;
	}

	// ShadingMode GetShadingMode()
	static int GetShadingMode(lua_State* L) {
		Graphics* _p = Graphics::instance();
		return Lua::push(L, _p->GetShadingMode());
	}

	// void SetAmbientOcclusionEnabled(bool value)
	static int SetAmbientOcclusionEnabled(lua_State* L) {
		Graphics* _p = Graphics::instance();
		bool value = Lua::get<bool>(L, 2);
		_p->SetAmbientOcclusionEnabled(value);
		return 0;
	}

	// bool GetAmbientOcclusionEnabled()
	static int GetAmbientOcclusionEnabled(lua_State* L) {
		Graphics* _p = Graphics::instance();
		return Lua::push(L, _p->GetAmbientOcclusionEnabled());
	}

	// void SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer)
	static int SetRenderTarget(lua_State* L) {
		Graphics* _p = Graphics::instance();
		uint depthBuffer = Lua::get<uint>(L, 3);
		std::vector<uint> colorBuffers = Lua::getList<uint>(L, 2);
		_p->SetRenderTarget(colorBuffers, depthBuffer);
		return 0;
	}

	// void Draw(Mesh mesh, Material material)
	static int Draw(lua_State* L) {
		Graphics* _p = Graphics::instance();
		Material material = Lua::get<Material>(L, 3);
		Mesh mesh = Lua::get<Mesh>(L, 2);
		_p->Draw(mesh, material);
		return 0;
	}

	// void Blit(Texture src, RenderTexture dest)
	static int Blit(lua_State* L) {
		Graphics* _p = Graphics::instance();
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		Texture src = Lua::get<Texture>(L, 2);
		_p->Blit(src, dest);
		return 0;
	}

	// void Blit(Texture src, RenderTexture dest, const Rect& rect)
	static int Blit2(lua_State* L) {
		Graphics* _p = Graphics::instance();
		Rect rect = Lua::get<Rect>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		Texture src = Lua::get<Texture>(L, 2);
		_p->Blit(src, dest, rect);
		return 0;
	}

	// void Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect)
	static int Blit3(lua_State* L) {
		Graphics* _p = Graphics::instance();
		Rect destRect = Lua::get<Rect>(L, 5);
		Rect srcRect = Lua::get<Rect>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		Texture src = Lua::get<Texture>(L, 2);
		_p->Blit(src, dest, srcRect, destRect);
		return 0;
	}

	// void Blit(Texture src, RenderTexture dest, Material material)
	static int Blit4(lua_State* L) {
		Graphics* _p = Graphics::instance();
		Material material = Lua::get<Material>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		Texture src = Lua::get<Texture>(L, 2);
		_p->Blit(src, dest, material);
		return 0;
	}

	// void Blit(Texture src, RenderTexture dest, Material material, const Rect& rect)
	static int Blit5(lua_State* L) {
		Graphics* _p = Graphics::instance();
		Rect rect = Lua::get<Rect>(L, 5);
		Material material = Lua::get<Material>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		Texture src = Lua::get<Texture>(L, 2);
		_p->Blit(src, dest, material, rect);
		return 0;
	}

	// void Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect)
	static int Blit6(lua_State* L) {
		Graphics* _p = Graphics::instance();
		Rect destRect = Lua::get<Rect>(L, 6);
		Rect srcRect = Lua::get<Rect>(L, 5);
		Material material = Lua::get<Material>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		Texture src = Lua::get<Texture>(L, 2);
		_p->Blit(src, dest, material, srcRect, destRect);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Graphics>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "GraphicsInstance", GraphicsInstance });

		luaL_Reg metalib[] = {
			{ "SetShadingMode", SetShadingMode },
			{ "GetShadingMode", GetShadingMode },
			{ "SetAmbientOcclusionEnabled", SetAmbientOcclusionEnabled },
			{ "GetAmbientOcclusionEnabled", GetAmbientOcclusionEnabled },
			{ "SetRenderTarget", SetRenderTarget },
			{ "Draw", Draw },
			{ "Blit", Blit },
			{ "Blit2", Blit2 },
			{ "Blit3", Blit3 },
			{ "Blit4", Blit4 },
			{ "Blit5", Blit5 },
			{ "Blit6", Blit6 },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Graphics>(L, metalib, nullptr);
	}
};
