// Warning: this file is generated by luawrapper.py.

#pragma once

#include "animation_wrapper.h"
#include "behaviour_wrapper.h"
#include "bounds_wrapper.h"
#include "camera_wrapper.h"
#include "color_wrapper.h"
#include "component_wrapper.h"
#include "engine_wrapper.h"
#include "font_wrapper.h"
#include "gameobject_wrapper.h"
#include "gizmos_wrapper.h"
#include "graphics_wrapper.h"
#include "imageeffect_wrapper.h"
#include "input_wrapper.h"
#include "light_wrapper.h"
#include "material_wrapper.h"
#include "mesh_wrapper.h"
#include "particlesystem_wrapper.h"
#include "physics_wrapper.h"
#include "plane_wrapper.h"
#include "polygon_wrapper.h"
#include "projector_wrapper.h"
#include "ray_wrapper.h"
#include "rect_wrapper.h"
#include "renderer_wrapper.h"
#include "resources_wrapper.h"
#include "rigidbody_wrapper.h"
#include "screen_wrapper.h"
#include "shader_wrapper.h"
#include "tagmanager_wrapper.h"
#include "texture_wrapper.h"
#include "time2_wrapper.h"
#include "transform_wrapper.h"
#include "world_wrapper.h"

namespace Lua {

static int configure(lua_State* L) {
	luaL_checkversion(L);

#pragma region register metatables

	SkeletonBone_Wrapper::create(L);
	SkeletonNode_Wrapper::create(L);
	Skeleton_Wrapper::create(L);
	AnimationClip_Wrapper::create(L);
	AnimationState_Wrapper::create(L);
	AnimationKeys_Wrapper::create(L);
	AnimationFrame_Wrapper::create(L);
	AnimationCurve_Wrapper::create(L);
	Animation_Wrapper::create(L);
	Behaviour_Wrapper::create(L);
	Bounds_Wrapper::create(L);
	Camera_Wrapper::create(L);
	Color_Wrapper::create(L);
	Component_Wrapper::create(L);
	Engine_Wrapper::create(L);
	CharacterInfo_Wrapper::create(L);
	Font_Wrapper::create(L);
	GameObject_Wrapper::create(L);
	Gizmos_Wrapper::create(L);
	Graphics_Wrapper::create(L);
	ImageEffect_Wrapper::create(L);
	Input_Wrapper::create(L);
	Light_Wrapper::create(L);
	Material_Wrapper::create(L);
	TriangleBias_Wrapper::create(L);
	SubMesh_Wrapper::create(L);
	BlendAttribute_Wrapper::create(L);
	InstanceAttribute_Wrapper::create(L);
	MeshAttribute_Wrapper::create(L);
	Mesh_Wrapper::create(L);
	MeshProvider_Wrapper::create(L);
	TextMesh_Wrapper::create(L);
	MeshFilter_Wrapper::create(L);
	ParticleBurst_Wrapper::create(L);
	Particle_Wrapper::create(L);
	ParticleEmitter_Wrapper::create(L);
	SphereParticleEmitter_Wrapper::create(L);
	ParticleAnimator_Wrapper::create(L);
	ParticleSystem_Wrapper::create(L);
	RaycastHit_Wrapper::create(L);
	Physics_Wrapper::create(L);
	Plane_Wrapper::create(L);
	Polygon_Wrapper::create(L);
	Triangle_Wrapper::create(L);
	Decal_Wrapper::create(L);
	Projector_Wrapper::create(L);
	Ray_Wrapper::create(L);
	Rect_Wrapper::create(L);
	Renderer_Wrapper::create(L);
	MeshRenderer_Wrapper::create(L);
	ParticleRenderer_Wrapper::create(L);
	SkinnedMeshRenderer_Wrapper::create(L);
	Resources_Wrapper::create(L);
	Rigidbody_Wrapper::create(L);
	Screen_Wrapper::create(L);
	Property_Wrapper::create(L);
	ShaderProperty_Wrapper::create(L);
	Shader_Wrapper::create(L);
	TagManager_Wrapper::create(L);
	Texture_Wrapper::create(L);
	Texture2D_Wrapper::create(L);
	TextureCube_Wrapper::create(L);
	TextureBuffer_Wrapper::create(L);
	RenderTexture_Wrapper::create(L);
	MRTRenderTexture_Wrapper::create(L);
	Time_Wrapper::create(L);
	PRS_Wrapper::create(L);
	Transform_Wrapper::create(L);
	Environment_Wrapper::create(L);
	FrameStatistics_Wrapper::create(L);
	World_Wrapper::create(L);

	std::vector<luaL_Reg> funcs, fields;

	SkeletonBone_Wrapper::initialize(L, funcs, fields);
	SkeletonNode_Wrapper::initialize(L, funcs, fields);
	Skeleton_Wrapper::initialize(L, funcs, fields);
	AnimationClip_Wrapper::initialize(L, funcs, fields);
	AnimationState_Wrapper::initialize(L, funcs, fields);
	AnimationKeys_Wrapper::initialize(L, funcs, fields);
	AnimationFrame_Wrapper::initialize(L, funcs, fields);
	AnimationCurve_Wrapper::initialize(L, funcs, fields);
	Animation_Wrapper::initialize(L, funcs, fields);
	Behaviour_Wrapper::initialize(L, funcs, fields);
	Bounds_Wrapper::initialize(L, funcs, fields);
	Camera_Wrapper::initialize(L, funcs, fields);
	Color_Wrapper::initialize(L, funcs, fields);
	Component_Wrapper::initialize(L, funcs, fields);
	Engine_Wrapper::initialize(L, funcs, fields);
	CharacterInfo_Wrapper::initialize(L, funcs, fields);
	Font_Wrapper::initialize(L, funcs, fields);
	GameObject_Wrapper::initialize(L, funcs, fields);
	Gizmos_Wrapper::initialize(L, funcs, fields);
	Graphics_Wrapper::initialize(L, funcs, fields);
	ImageEffect_Wrapper::initialize(L, funcs, fields);
	Input_Wrapper::initialize(L, funcs, fields);
	Light_Wrapper::initialize(L, funcs, fields);
	Material_Wrapper::initialize(L, funcs, fields);
	TriangleBias_Wrapper::initialize(L, funcs, fields);
	SubMesh_Wrapper::initialize(L, funcs, fields);
	BlendAttribute_Wrapper::initialize(L, funcs, fields);
	InstanceAttribute_Wrapper::initialize(L, funcs, fields);
	MeshAttribute_Wrapper::initialize(L, funcs, fields);
	Mesh_Wrapper::initialize(L, funcs, fields);
	MeshProvider_Wrapper::initialize(L, funcs, fields);
	TextMesh_Wrapper::initialize(L, funcs, fields);
	MeshFilter_Wrapper::initialize(L, funcs, fields);
	ParticleBurst_Wrapper::initialize(L, funcs, fields);
	Particle_Wrapper::initialize(L, funcs, fields);
	ParticleEmitter_Wrapper::initialize(L, funcs, fields);
	SphereParticleEmitter_Wrapper::initialize(L, funcs, fields);
	ParticleAnimator_Wrapper::initialize(L, funcs, fields);
	ParticleSystem_Wrapper::initialize(L, funcs, fields);
	RaycastHit_Wrapper::initialize(L, funcs, fields);
	Physics_Wrapper::initialize(L, funcs, fields);
	Plane_Wrapper::initialize(L, funcs, fields);
	Polygon_Wrapper::initialize(L, funcs, fields);
	Triangle_Wrapper::initialize(L, funcs, fields);
	Decal_Wrapper::initialize(L, funcs, fields);
	Projector_Wrapper::initialize(L, funcs, fields);
	Ray_Wrapper::initialize(L, funcs, fields);
	Rect_Wrapper::initialize(L, funcs, fields);
	Renderer_Wrapper::initialize(L, funcs, fields);
	MeshRenderer_Wrapper::initialize(L, funcs, fields);
	ParticleRenderer_Wrapper::initialize(L, funcs, fields);
	SkinnedMeshRenderer_Wrapper::initialize(L, funcs, fields);
	Resources_Wrapper::initialize(L, funcs, fields);
	Rigidbody_Wrapper::initialize(L, funcs, fields);
	Screen_Wrapper::initialize(L, funcs, fields);
	Property_Wrapper::initialize(L, funcs, fields);
	ShaderProperty_Wrapper::initialize(L, funcs, fields);
	Shader_Wrapper::initialize(L, funcs, fields);
	TagManager_Wrapper::initialize(L, funcs, fields);
	Texture_Wrapper::initialize(L, funcs, fields);
	Texture2D_Wrapper::initialize(L, funcs, fields);
	TextureCube_Wrapper::initialize(L, funcs, fields);
	TextureBuffer_Wrapper::initialize(L, funcs, fields);
	RenderTexture_Wrapper::initialize(L, funcs, fields);
	MRTRenderTexture_Wrapper::initialize(L, funcs, fields);
	Time_Wrapper::initialize(L, funcs, fields);
	PRS_Wrapper::initialize(L, funcs, fields);
	Transform_Wrapper::initialize(L, funcs, fields);
	Environment_Wrapper::initialize(L, funcs, fields);
	FrameStatistics_Wrapper::initialize(L, funcs, fields);
	World_Wrapper::initialize(L, funcs, fields);

	lua_createtable(L, 0, (int)funcs.size());

#pragma endregion

#pragma region register enumerations

	// AnimationWrapMode
	lua_newtable(L);
	for (int i = 0; i < AnimationWrapMode::size(); ++i) {
		lua_pushinteger(L, AnimationWrapMode::value(i));
		lua_setfield(L, -2, AnimationWrapMode::value(i).to_string());
	}

	lua_setfield(L, -2, "AnimationWrapMode");

	// ClearType
	lua_newtable(L);
	for (int i = 0; i < ClearType::size(); ++i) {
		lua_pushinteger(L, ClearType::value(i));
		lua_setfield(L, -2, ClearType::value(i).to_string());
	}

	lua_setfield(L, -2, "ClearType");

	// RenderPath
	lua_newtable(L);
	for (int i = 0; i < RenderPath::size(); ++i) {
		lua_pushinteger(L, RenderPath::value(i));
		lua_setfield(L, -2, RenderPath::value(i).to_string());
	}

	lua_setfield(L, -2, "RenderPath");

	// DepthTextureMode
	lua_newtable(L);
	for (int i = 0; i < DepthTextureMode::size(); ++i) {
		lua_pushinteger(L, DepthTextureMode::value(i));
		lua_setfield(L, -2, DepthTextureMode::value(i).to_string());
	}

	lua_setfield(L, -2, "DepthTextureMode");

	// FrameEventQueue
	lua_newtable(L);
	lua_pushinteger(L, (int)FrameEventQueue::Time);
	lua_setfield(L, -2, "Time");
	lua_pushinteger(L, (int)FrameEventQueue::Profiler);
	lua_setfield(L, -2, "Profiler");
	lua_pushinteger(L, (int)FrameEventQueue::Statistics);
	lua_setfield(L, -2, "Statistics");
	lua_pushinteger(L, (int)FrameEventQueue::Physics);
	lua_setfield(L, -2, "Physics");
	lua_pushinteger(L, (int)FrameEventQueue::User);
	lua_setfield(L, -2, "User");
	lua_pushinteger(L, (int)FrameEventQueue::Max);
	lua_setfield(L, -2, "Max");
	lua_pushinteger(L, (int)FrameEventQueue::Gizmos);
	lua_setfield(L, -2, "Gizmos");
	lua_pushinteger(L, (int)FrameEventQueue::Inputs);
	lua_setfield(L, -2, "Inputs");

	lua_setfield(L, -2, "FrameEventQueue");

	// ShadingMode
	lua_newtable(L);
	for (int i = 0; i < ShadingMode::size(); ++i) {
		lua_pushinteger(L, ShadingMode::value(i));
		lua_setfield(L, -2, ShadingMode::value(i).to_string());
	}

	lua_setfield(L, -2, "ShadingMode");

	// KeyCode
	lua_newtable(L);
	for (int i = 0; i < KeyCode::size(); ++i) {
		lua_pushinteger(L, KeyCode::value(i));
		lua_setfield(L, -2, KeyCode::value(i).to_string());
	}

	lua_setfield(L, -2, "KeyCode");

	// LightImportance
	lua_newtable(L);
	for (int i = 0; i < LightImportance::size(); ++i) {
		lua_pushinteger(L, LightImportance::value(i));
		lua_setfield(L, -2, LightImportance::value(i).to_string());
	}

	lua_setfield(L, -2, "LightImportance");

	// LightType
	lua_newtable(L);
	for (int i = 0; i < LightType::size(); ++i) {
		lua_pushinteger(L, LightType::value(i));
		lua_setfield(L, -2, LightType::value(i).to_string());
	}

	lua_setfield(L, -2, "LightType");

	// RenderQueue
	lua_newtable(L);
	lua_pushinteger(L, (int)RenderQueue::Background);
	lua_setfield(L, -2, "Background");
	lua_pushinteger(L, (int)RenderQueue::Geometry);
	lua_setfield(L, -2, "Geometry");
	lua_pushinteger(L, (int)RenderQueue::Transparent);
	lua_setfield(L, -2, "Transparent");
	lua_pushinteger(L, (int)RenderQueue::Overlay);
	lua_setfield(L, -2, "Overlay");

	lua_setfield(L, -2, "RenderQueue");

	// MeshTopology
	lua_newtable(L);
	lua_pushinteger(L, (int)MeshTopology::Points);
	lua_setfield(L, -2, "Points");
	lua_pushinteger(L, (int)MeshTopology::Lines);
	lua_setfield(L, -2, "Lines");
	lua_pushinteger(L, (int)MeshTopology::LineStripe);
	lua_setfield(L, -2, "LineStripe");
	lua_pushinteger(L, (int)MeshTopology::Triangles);
	lua_setfield(L, -2, "Triangles");
	lua_pushinteger(L, (int)MeshTopology::TriangleStripe);
	lua_setfield(L, -2, "TriangleStripe");
	lua_pushinteger(L, (int)MeshTopology::TriangleFan);
	lua_setfield(L, -2, "TriangleFan");

	lua_setfield(L, -2, "MeshTopology");

	// PrimitiveType
	lua_newtable(L);
	for (int i = 0; i < PrimitiveType::size(); ++i) {
		lua_pushinteger(L, PrimitiveType::value(i));
		lua_setfield(L, -2, PrimitiveType::value(i).to_string());
	}

	lua_setfield(L, -2, "PrimitiveType");

	// ObjectType
	lua_newtable(L);
	for (int i = 0; i < ObjectType::size(); ++i) {
		lua_pushinteger(L, ObjectType::value(i));
		lua_setfield(L, -2, ObjectType::value(i).to_string());
	}

	lua_setfield(L, -2, "ObjectType");

	// TextureMinFilterMode
	lua_newtable(L);
	for (int i = 0; i < TextureMinFilterMode::size(); ++i) {
		lua_pushinteger(L, TextureMinFilterMode::value(i));
		lua_setfield(L, -2, TextureMinFilterMode::value(i).to_string());
	}

	lua_setfield(L, -2, "TextureMinFilterMode");

	// TextureMagFilterMode
	lua_newtable(L);
	for (int i = 0; i < TextureMagFilterMode::size(); ++i) {
		lua_pushinteger(L, TextureMagFilterMode::value(i));
		lua_setfield(L, -2, TextureMagFilterMode::value(i).to_string());
	}

	lua_setfield(L, -2, "TextureMagFilterMode");

	// TextureWrapMode
	lua_newtable(L);
	for (int i = 0; i < TextureWrapMode::size(); ++i) {
		lua_pushinteger(L, TextureWrapMode::value(i));
		lua_setfield(L, -2, TextureWrapMode::value(i).to_string());
	}

	lua_setfield(L, -2, "TextureWrapMode");

	// TextureFormat
	lua_newtable(L);
	lua_pushinteger(L, (int)TextureFormat::Rgb);
	lua_setfield(L, -2, "Rgb");
	lua_pushinteger(L, (int)TextureFormat::Rgba);
	lua_setfield(L, -2, "Rgba");
	lua_pushinteger(L, (int)TextureFormat::RgbS);
	lua_setfield(L, -2, "RgbS");
	lua_pushinteger(L, (int)TextureFormat::RgbaS);
	lua_setfield(L, -2, "RgbaS");
	lua_pushinteger(L, (int)TextureFormat::Rgb16F);
	lua_setfield(L, -2, "Rgb16F");
	lua_pushinteger(L, (int)TextureFormat::Rgb32F);
	lua_setfield(L, -2, "Rgb32F");
	lua_pushinteger(L, (int)TextureFormat::Rgba16F);
	lua_setfield(L, -2, "Rgba16F");
	lua_pushinteger(L, (int)TextureFormat::Rgba32F);
	lua_setfield(L, -2, "Rgba32F");

	lua_setfield(L, -2, "TextureFormat");

	// ColorStreamFormat
	lua_newtable(L);
	lua_pushinteger(L, (int)ColorStreamFormat::Rgb);
	lua_setfield(L, -2, "Rgb");
	lua_pushinteger(L, (int)ColorStreamFormat::RgbF);
	lua_setfield(L, -2, "RgbF");
	lua_pushinteger(L, (int)ColorStreamFormat::Bgr);
	lua_setfield(L, -2, "Bgr");
	lua_pushinteger(L, (int)ColorStreamFormat::Rgba);
	lua_setfield(L, -2, "Rgba");
	lua_pushinteger(L, (int)ColorStreamFormat::RgbaF);
	lua_setfield(L, -2, "RgbaF");
	lua_pushinteger(L, (int)ColorStreamFormat::Argb);
	lua_setfield(L, -2, "Argb");
	lua_pushinteger(L, (int)ColorStreamFormat::Bgra);
	lua_setfield(L, -2, "Bgra");
	lua_pushinteger(L, (int)ColorStreamFormat::LuminanceAlpha);
	lua_setfield(L, -2, "LuminanceAlpha");

	lua_setfield(L, -2, "ColorStreamFormat");

	// RenderTextureFormat
	lua_newtable(L);
	for (int i = 0; i < RenderTextureFormat::size(); ++i) {
		lua_pushinteger(L, RenderTextureFormat::value(i));
		lua_setfield(L, -2, RenderTextureFormat::value(i).to_string());
	}

	lua_setfield(L, -2, "RenderTextureFormat");

	// WorldEventType
	lua_newtable(L);
	for (int i = 0; i < WorldEventType::size(); ++i) {
		lua_pushinteger(L, WorldEventType::value(i));
		lua_setfield(L, -2, WorldEventType::value(i).to_string());
	}

	lua_setfield(L, -2, "WorldEventType");

	// WalkCommand
	lua_newtable(L);
	lua_pushinteger(L, (int)WalkCommand::Next);
	lua_setfield(L, -2, "Next");
	lua_pushinteger(L, (int)WalkCommand::Break);
	lua_setfield(L, -2, "Break");
	lua_pushinteger(L, (int)WalkCommand::Continue);
	lua_setfield(L, -2, "Continue");

	lua_setfield(L, -2, "WalkCommand");

#pragma endregion

#pragma region register constructors and getters.
	funcs.push_back(luaL_Reg{ nullptr, nullptr });
	luaL_setfuncs(L, funcs.data(), 0);

	for (luaL_Reg& field : fields) {
		field.func(L);
		lua_setfield(L, -2, field.name);
	}
	
#pragma endregion

	return 1;
}

}	// namespace Lua
