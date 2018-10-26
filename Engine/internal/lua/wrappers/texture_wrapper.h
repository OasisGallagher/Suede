// Warning: this file is generated by wrapper.py.

#pragma once

#include "texture.h"

#include "lua++.h"
#include "tools/string.h"

class Texture_Wrapper {
	static int ToString(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		lua_pushstring(L, String::Format("Texture@0x%p", _p.get()).c_str());
		return 1;
	}

	// void Bind(uint index)
	static int Bind(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 1);
		uint index = Lua::get<uint>(L, 2);
		_p->Bind(index);
		return 0;
	}

	// void Unbind()
	static int Unbind(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		_p->Unbind();
		return 0;
	}

	// uint GetNativePointer()
	static int GetNativePointer(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetNativePointer());
	}

	// void SetMinFilterMode(TextureMinFilterMode value)
	static int SetMinFilterMode(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 1);
		TextureMinFilterMode value = Lua::get<TextureMinFilterMode>(L, 2);
		_p->SetMinFilterMode(value);
		return 0;
	}

	// TextureMinFilterMode GetMinFilterMode()
	static int GetMinFilterMode(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetMinFilterMode());
	}

	// void SetMagFilterMode(TextureMagFilterMode value)
	static int SetMagFilterMode(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 1);
		TextureMagFilterMode value = Lua::get<TextureMagFilterMode>(L, 2);
		_p->SetMagFilterMode(value);
		return 0;
	}

	// TextureMagFilterMode GetMagFilterMode()
	static int GetMagFilterMode(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetMagFilterMode());
	}

	// void SetWrapModeS(TextureWrapMode value)
	static int SetWrapModeS(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 1);
		TextureWrapMode value = Lua::get<TextureWrapMode>(L, 2);
		_p->SetWrapModeS(value);
		return 0;
	}

	// TextureWrapMode GetWrapModeS()
	static int GetWrapModeS(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetWrapModeS());
	}

	// void SetWrapModeT(TextureWrapMode value)
	static int SetWrapModeT(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 1);
		TextureWrapMode value = Lua::get<TextureWrapMode>(L, 2);
		_p->SetWrapModeT(value);
		return 0;
	}

	// TextureWrapMode GetWrapModeT()
	static int GetWrapModeT(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetWrapModeT());
	}

	// uint GetWidth()
	static int GetWidth(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetWidth());
	}

	// uint GetHeight()
	static int GetHeight(lua_State* L) {
		Texture& _p = *Lua::callerSharedPtr<Texture>(L, 0);
		return Lua::push(L, _p->GetHeight());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Texture>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Texture> },
			{ "__tostring", ToString }, 
			{ "Bind", Bind },
			{ "Unbind", Unbind },
			{ "GetNativePointer", GetNativePointer },
			{ "SetMinFilterMode", SetMinFilterMode },
			{ "GetMinFilterMode", GetMinFilterMode },
			{ "SetMagFilterMode", SetMagFilterMode },
			{ "GetMagFilterMode", GetMagFilterMode },
			{ "SetWrapModeS", SetWrapModeS },
			{ "GetWrapModeS", GetWrapModeS },
			{ "SetWrapModeT", SetWrapModeT },
			{ "GetWrapModeT", GetWrapModeT },
			{ "GetWidth", GetWidth },
			{ "GetHeight", GetHeight },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Texture>(L, metalib, TypeID<Object>::name());
	}
};

class Texture2D_Wrapper {
	static int NewTexture2D(lua_State* L) {
		return Lua::fromShared(L, ::NewTexture2D());
	}

	static int ToString(lua_State* L) {
		Texture2D& _p = *Lua::callerSharedPtr<Texture2D>(L, 0);
		lua_pushstring(L, String::Format("Texture2D@0x%p", _p.get()).c_str());
		return 1;
	}

	// bool Create(const std::string& path)
	static int Create(lua_State* L) {
		Texture2D& _p = *Lua::callerSharedPtr<Texture2D>(L, 1);
		std::string path = Lua::get<std::string>(L, 2);
		return Lua::push(L, _p->Create(path));
	}

	// TextureFormat GetFormat()
	static int GetFormat(lua_State* L) {
		Texture2D& _p = *Lua::callerSharedPtr<Texture2D>(L, 0);
		return Lua::push(L, _p->GetFormat());
	}

	// bool EncodeToPNG(std::vector<uchar>& data)
	static int EncodeToPNG(lua_State* L) {
		Texture2D& _p = *Lua::callerSharedPtr<Texture2D>(L, 1);
		std::vector<uchar> data = Lua::getList<uchar>(L, 2);
		return Lua::push(L, _p->EncodeToPNG(data));
	}

	// bool EncodeToJPG(std::vector<uchar>& data)
	static int EncodeToJPG(lua_State* L) {
		Texture2D& _p = *Lua::callerSharedPtr<Texture2D>(L, 1);
		std::vector<uchar> data = Lua::getList<uchar>(L, 2);
		return Lua::push(L, _p->EncodeToJPG(data));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Texture2D>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewTexture2D", NewTexture2D });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Texture2D> },
			{ "__tostring", ToString }, 
			{ "Create", Create },
			{ "GetFormat", GetFormat },
			{ "EncodeToPNG", EncodeToPNG },
			{ "EncodeToJPG", EncodeToJPG },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Texture2D>(L, metalib, TypeID<Texture>::name());
	}
};

class TextureCube_Wrapper {
	static int NewTextureCube(lua_State* L) {
		return Lua::fromShared(L, ::NewTextureCube());
	}

	static int ToString(lua_State* L) {
		TextureCube& _p = *Lua::callerSharedPtr<TextureCube>(L, 0);
		lua_pushstring(L, String::Format("TextureCube@0x%p", _p.get()).c_str());
		return 1;
	}

	// bool Load(const std::string textures[6])
	static int Load(lua_State* L) {
		TextureCube& _p = *Lua::callerSharedPtr<TextureCube>(L, 1);
		std::vector<std::string> textures = Lua::getList<std::string>(L, 2);
		return Lua::push(L, _p->Load(textures.data()));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<TextureCube>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewTextureCube", NewTextureCube });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<TextureCube> },
			{ "__tostring", ToString }, 
			{ "Load", Load },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<TextureCube>(L, metalib, TypeID<Texture>::name());
	}
};

class TextureBuffer_Wrapper {
	static int NewTextureBuffer(lua_State* L) {
		return Lua::fromShared(L, ::NewTextureBuffer());
	}

	static int ToString(lua_State* L) {
		TextureBuffer& _p = *Lua::callerSharedPtr<TextureBuffer>(L, 0);
		lua_pushstring(L, String::Format("TextureBuffer@0x%p", _p.get()).c_str());
		return 1;
	}

	// uint GetSize()
	static int GetSize(lua_State* L) {
		TextureBuffer& _p = *Lua::callerSharedPtr<TextureBuffer>(L, 0);
		return Lua::push(L, _p->GetSize());
	}

	// bool Create(uint size)
	static int Create(lua_State* L) {
		TextureBuffer& _p = *Lua::callerSharedPtr<TextureBuffer>(L, 1);
		uint size = Lua::get<uint>(L, 2);
		return Lua::push(L, _p->Create(size));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<TextureBuffer>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewTextureBuffer", NewTextureBuffer });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<TextureBuffer> },
			{ "__tostring", ToString }, 
			{ "GetSize", GetSize },
			{ "Create", Create },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<TextureBuffer>(L, metalib, TypeID<Texture>::name());
	}
};

class RenderTexture_Wrapper {
	static int NewRenderTexture(lua_State* L) {
		return Lua::fromShared(L, ::NewRenderTexture());
	}

	static int ToString(lua_State* L) {
		RenderTexture& _p = *Lua::callerSharedPtr<RenderTexture>(L, 0);
		lua_pushstring(L, String::Format("RenderTexture@0x%p", _p.get()).c_str());
		return 1;
	}

	// bool Create(RenderTextureFormat format, uint width, uint height)
	static int Create(lua_State* L) {
		RenderTexture& _p = *Lua::callerSharedPtr<RenderTexture>(L, 3);
		uint height = Lua::get<uint>(L, 4);
		uint width = Lua::get<uint>(L, 3);
		RenderTextureFormat format = Lua::get<RenderTextureFormat>(L, 2);
		return Lua::push(L, _p->Create(format, width, height));
	}

	// void Resize(uint width, uint height)
	static int Resize(lua_State* L) {
		RenderTexture& _p = *Lua::callerSharedPtr<RenderTexture>(L, 2);
		uint height = Lua::get<uint>(L, 3);
		uint width = Lua::get<uint>(L, 2);
		_p->Resize(width, height);
		return 0;
	}

	// void Clear(const Rect& normalizedRect, const Color& color, float depth)
	static int Clear(lua_State* L) {
		RenderTexture& _p = *Lua::callerSharedPtr<RenderTexture>(L, 3);
		float depth = Lua::get<float>(L, 4);
		Color color = Lua::get<Color>(L, 3);
		Rect normalizedRect = Lua::get<Rect>(L, 2);
		_p->Clear(normalizedRect, color, depth);
		return 0;
	}

	// void BindWrite(const Rect& normalizedRect)
	static int BindWrite(lua_State* L) {
		RenderTexture& _p = *Lua::callerSharedPtr<RenderTexture>(L, 1);
		Rect normalizedRect = Lua::get<Rect>(L, 2);
		_p->BindWrite(normalizedRect);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<RenderTexture>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewRenderTexture", NewRenderTexture });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<RenderTexture> },
			{ "__tostring", ToString }, 
			{ "Create", Create },
			{ "Resize", Resize },
			{ "Clear", Clear },
			{ "BindWrite", BindWrite },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<RenderTexture>(L, metalib, TypeID<Texture>::name());
	}
};

class MRTRenderTexture_Wrapper {
	static int NewMRTRenderTexture(lua_State* L) {
		return Lua::fromShared(L, ::NewMRTRenderTexture());
	}

	static int ToString(lua_State* L) {
		MRTRenderTexture& _p = *Lua::callerSharedPtr<MRTRenderTexture>(L, 0);
		lua_pushstring(L, String::Format("MRTRenderTexture@0x%p", _p.get()).c_str());
		return 1;
	}

	// bool AddColorTexture(TextureFormat format)
	static int AddColorTexture(lua_State* L) {
		MRTRenderTexture& _p = *Lua::callerSharedPtr<MRTRenderTexture>(L, 1);
		TextureFormat format = Lua::get<TextureFormat>(L, 2);
		return Lua::push(L, _p->AddColorTexture(format));
	}

	// Texture2D GetColorTexture(uint index)
	static int GetColorTexture(lua_State* L) {
		MRTRenderTexture& _p = *Lua::callerSharedPtr<MRTRenderTexture>(L, 1);
		uint index = Lua::get<uint>(L, 2);
		return Lua::push(L, _p->GetColorTexture(index));
	}

	// uint GetColorTextureCount()
	static int GetColorTextureCount(lua_State* L) {
		MRTRenderTexture& _p = *Lua::callerSharedPtr<MRTRenderTexture>(L, 0);
		return Lua::push(L, _p->GetColorTextureCount());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<MRTRenderTexture>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewMRTRenderTexture", NewMRTRenderTexture });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<MRTRenderTexture> },
			{ "__tostring", ToString }, 
			{ "AddColorTexture", AddColorTexture },
			{ "GetColorTexture", GetColorTexture },
			{ "GetColorTextureCount", GetColorTextureCount },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MRTRenderTexture>(L, metalib, TypeID<RenderTexture>::name());
	}
};
