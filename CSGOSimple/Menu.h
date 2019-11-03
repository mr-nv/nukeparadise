#include <d3d9.h>
#include <d3dx9.h>
#include "singleton.hpp"
#include "menu_helpers.hpp"

enum class MenuAvailable : int
{
	RAGEBOT,
	LEGITBOT,
	VISUALS,
	MISC,
	SETTINGS
};

enum class VisualsMenuAvailable : int
{
	LOCAL,
	ENEMY,
	TEAM,
	MISC,
	GLOBAL
};

enum class RbotMenuAvailable : int
{
	STANDING,
	MOVING,
	AIR,
	MISC
};

enum class LbotWeaponsAvailable : int
{
	PISTOL,
	SMG,
	MG,
	RIFLE,
	SHOTGUN,
	SNIPER
};

#pragma once
class Menu : public Singleton<Menu>
{
public:
	Menu();
	void Initialize();
	void Render();

	/* sections */
	void RenderRagebot();
	void RenderLegitbot();
	void RenderVisuals();
	void RenderSkins();
	void RenderList();
	void RenderMisc();
	void TextureInit(IDirect3DDevice9* pDevice);
	IDirect3DTexture9* GetTexture();
private:
	bool Loaded = false;
	IDirect3DTexture9* Texture = nullptr;
};

