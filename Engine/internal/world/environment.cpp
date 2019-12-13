#include "environment.h"

static ref_ptr<Material> skybox;

static float fogDensity = 0.0001f;

static Color fogColor = Color::white;
static Color ambientColor = Color::white * 0.02;

void Environment::SetSkybox(Material* value) { skybox = value; }
Material* Environment::GetSkybox() { return skybox.get(); }

void Environment::SetAmbientColor(const Color& value) { ambientColor = value; }
Color Environment::GetAmbientColor() { return ambientColor; }

void Environment::SetFogColor(const Color& value) { fogColor = value; }
Color Environment::GetFogColor() { return fogColor; }

void Environment::SetFogDensity(float value) { fogDensity = value; }
float Environment::GetFogDensity() { return fogDensity; }
