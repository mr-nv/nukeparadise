

#include "MaterialManager.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../config.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"

//"$basetexture" "vgui/white_additive"

void set_ignorez(const bool enabled, IMaterial* mat)
{
	if (!g_EngineClient->IsInGame()) return;
	mat->IncrementReferenceCount();
	mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, enabled);
}

MaterialManager::MaterialManager()
{
    std::ofstream("csgo\\materials\\simple_regular.vmt") << R"#("VertexLitGeneric"
{
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";//fix plz
	std::ofstream("csgo\\materials\\simple_reflective.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white"
  "$envmap"       "env_cubemap"
  "$model" "1"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast" "1"
  "$flat" "1"
  "$nocull" "0"
  "$selfillum" "1"
  "$halflambert" "1"
  "$nofog" "0"
  "$ignorez" "0"
  "$znearer" "0"
  "$wireframe" "0"
}
)#";//fix plz
	std::ofstream("csgo\\materials\\simple_reflectiveignorez.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white" 
  "$envmap"       "env_cubemap"
  "$model" "1"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast" "1"
  "$flat" "1"
  "$nocull" "0"
  "$selfillum" "1"
  "$halflambert" "1"
  "$nofog" "0"
  "$ignorez" "1"
  "$znearer" "0"
  "$wireframe" "0"
}
)#";
    std::ofstream("csgo\\materials\\simple_ignorez.vmt") << R"#("VertexLitGeneric"
{
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
    std::ofstream("csgo\\materials\\simple_flat.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
    std::ofstream("csgo\\materials\\simple_flat_ignorez.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\glowOverlay.vmt") << R"#("VertexLitGeneric" {
	"$ignorez"      "0"
    "$additive" "1"
    "$envmap" "models/effects/cube_white"
    "$envmapint" "[1 1 1]"
    "$envmapfresnel" "1"
    "$envmapfresnelminmaxexp" "[0 1 2]"
	"$alpha" "0.8"
	"$ignorez" "0"
})#";
	std::ofstream("csgo\\materials\\glowOverlay_ignorez.vmt") << R"#("VertexLitGeneric" {
	"$ignorez"      "0"
    "$additive" "1"
    "$envmap" "models/effects/cube_white"
    "$envmapint" "[1 1 1]"
    "$envmapfresnel" "1"
    "$envmapfresnelminmaxexp" "[0 1 2]"
	"$alpha" "0.8"
	"$ignorez"      "1"
})#";
    materialRegular = g_MatSystem->FindMaterial("simple_regular", TEXTURE_GROUP_MODEL);
    materialRegularIgnoreZ = g_MatSystem->FindMaterial("simple_ignorez", TEXTURE_GROUP_MODEL);
    materialFlatIgnoreZ = g_MatSystem->FindMaterial("simple_flat_ignorez", TEXTURE_GROUP_MODEL);
    materialFlat = g_MatSystem->FindMaterial("simple_flat", TEXTURE_GROUP_MODEL);
	materialReflective = g_MatSystem->FindMaterial("simple_reflective", TEXTURE_GROUP_MODEL);
	materialReflectiveIgnoreZ = g_MatSystem->FindMaterial("simple_reflectiveignorez", TEXTURE_GROUP_MODEL);
	materialGlow = g_MatSystem->FindMaterial("glowOverlay", TEXTURE_GROUP_MODEL);
	materialGlowIgnoreZ = g_MatSystem->FindMaterial("glowOverlay_ignorez", TEXTURE_GROUP_MODEL);
}

MaterialManager::~MaterialManager()
{
	std::remove("csgo\\materials\\simple_regular.vmt");
	std::remove("csgo\\materials\\simple_ignorez.vmt");
	std::remove("csgo\\materials\\simple_ignorez.vmt");
	std::remove("csgo\\materials\\simple_flat.vmt");
	std::remove("csgo\\materials\\simple_flat_ignorez.vmt");
	std::remove("csgo\\materials\\regular_reflective.vmt");
	std::remove("csgo\\materials\\simple_reflective.vmt");
	std::remove("csgo\\materials\\glowOverlay.vmt");
}
void MaterialManager::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool glow, const Color rgba)
{
	IMaterial* material = nullptr;

	if (flat)
	{
		if (ignoreZ)
		{
			material = materialFlatIgnoreZ;
		}
		else
		{
			material = materialFlat;
		}
	}
	else
		if (glow)
		{
			if (ignoreZ)
			{
				material = materialGlowIgnoreZ; ///!!!!!!!!!!!!!!!!!! fix !!!!!!!!!!!!!!!!!
			}
			else
			{
				material = materialGlow;
			}
		}
		else
		{
		if (ignoreZ && !metallic)
		{
			material = materialRegularIgnoreZ;
		}
		else if (ignoreZ && metallic)
		{
			material = materialReflectiveIgnoreZ;
		}
		else if (metallic)
		{
			material = materialReflective;
		}
		else
		{
			material = materialRegular;
		}
	}

	if (glass)
	{
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}
	else
	{
		material->AlphaModulate(
			rgba.a() / 255.0f);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}

void MaterialManager::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool glow)
{
	IMaterial* material = nullptr;

	if (flat)
	{
		if (ignoreZ)
		{
			material = materialFlatIgnoreZ;
		}
		else
		{
			material = materialFlat;
		}
	}
	else
	if (glow)
	{
		if (ignoreZ)
		{
			material = materialGlow; ///!!!!!!!!!!!!!!!!!! fix !!!!!!!!!!!!!!!!!
		}
		else
		{
			material = materialGlow;
		}
	}
	else
	{
		if (ignoreZ && !metallic && !flat)
		{
			material = materialRegularIgnoreZ;
		}
		else if (ignoreZ && metallic && !flat)
		{
			material = materialReflectiveIgnoreZ;
		}
		else if (metallic)
		{
			material = materialReflective;
		}
		else
		{
			material = materialRegular;
		}
	}


	if (glass)
	{
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);

	g_MdlRender->ForcedMaterialOverride(material);
}

