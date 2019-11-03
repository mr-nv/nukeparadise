#pragma once

#include "../singleton.hpp"

#include "../valve_sdk/misc/vfunc.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class MaterialManager : public Singleton<MaterialManager>
{
public:

	MaterialManager();
	~MaterialManager();

	void set_ignorez(const bool enabled, IMaterial* mat);
	void OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool glow, const Color rgba);
	void OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool glow);
private:
    IMaterial* materialRegular = nullptr;
    IMaterial* materialRegularIgnoreZ = nullptr;
    IMaterial* materialFlatIgnoreZ = nullptr;
    IMaterial* materialFlat = nullptr;
	IMaterial* materialReflective = nullptr;
	IMaterial* materialGlow = nullptr;
	IMaterial* materialGlowIgnoreZ = nullptr;
	IMaterial* materialReflectiveIgnoreZ = nullptr;
};