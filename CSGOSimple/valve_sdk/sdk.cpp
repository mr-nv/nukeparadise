
#include "sdk.hpp"
#include "csgostructs.hpp"
#include "../Helpers/Utils.hpp"

IVEngineClient*       g_EngineClient   = nullptr;
IBaseClientDLL*       g_CHLClient      = nullptr;
IClientEntityList*    g_EntityList     = nullptr;
CGlobalVarsBase*      g_GlobalVars     = nullptr;
IEngineTrace*         g_EngineTrace    = nullptr;
ICvar*                g_CVar           = nullptr;
IPanel*               g_VGuiPanel      = nullptr;
IClientMode*          g_ClientMode     = nullptr;
IVDebugOverlay*       g_DebugOverlay   = nullptr;
ISurface*             g_VGuiSurface    = nullptr;
CInput*               g_Input          = nullptr;
IVModelInfoClient*    g_MdlInfo        = nullptr;
IVModelRender*        g_MdlRender      = nullptr;
IVRenderView*         g_RenderView     = nullptr;
DT_CSPlayerResource*  g_PlayerResource = nullptr;
IMaterialSystem*      g_MatSystem      = nullptr;
IGameEventManager2*   g_GameEvents     = nullptr;
IMoveHelper*          g_MoveHelper     = nullptr;
IMDLCache*            g_MdlCache       = nullptr;
IPrediction*          g_Prediction     = nullptr;
CGameMovement*        g_GameMovement   = nullptr;
IEngineSound*         g_EngineSound    = nullptr;
CGlowObjectManager*   g_GlowObjManager = nullptr;
IViewRender*          g_ViewRender     = nullptr;
IDirect3DDevice9*     g_D3DDevice9     = nullptr;
CClientState*         g_ClientState    = nullptr;
IPhysicsSurfaceProps* g_PhysSurface    = nullptr;
IStudioRender*		  g_StudioRender = nullptr;
C_LocalPlayer         g_LocalPlayer;
IMemAlloc*			  g_pMemAlloc = nullptr;
IViewRenderBeams*     g_RenderBeams = nullptr;
CUtlVectorSimple* g_ClientSideAnimationList = nullptr;
ILocalize* g_Localize = nullptr;

namespace Interfaces
{
    CreateInterfaceFn get_module_factory(HMODULE module)
    {
        return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, "CreateInterface"));
    }

    template<typename T>
    T* get_interface(CreateInterfaceFn f, const char* szInterfaceVersion)
    {
        auto result = reinterpret_cast<T*>(f(szInterfaceVersion, nullptr));

        if(!result)
        {
            throw std::runtime_error(std::string("[get_interface] Failed to GetOffset interface: ") + szInterfaceVersion);
        }

        return result;
    }

    void Initialize()
    {
        auto engineFactory    = get_module_factory(GetModuleHandleW(L"engine.dll"));
        auto clientFactory    = get_module_factory(GetModuleHandleW(L"client_panorama.dll"));
        auto valveStdFactory  = get_module_factory(GetModuleHandleW(L"vstdlib.dll"));
        auto vguiFactory      = get_module_factory(GetModuleHandleW(L"vguimatsurface.dll"));
        auto vgui2Factory     = get_module_factory(GetModuleHandleW(L"vgui2.dll"));
        auto matSysFactory    = get_module_factory(GetModuleHandleW(L"materialsystem.dll"));
        auto dataCacheFactory = get_module_factory(GetModuleHandleW(L"datacache.dll"));
        auto vphysicsFactory  = get_module_factory(GetModuleHandleW(L"vphysics.dll"));
		auto studioFactory = get_module_factory(GetModuleHandleW(L"studiorender.dll"));
		auto localizeFactory = get_module_factory(GetModuleHandleW(L"localize.dll"));

        g_CHLClient           = get_interface<IBaseClientDLL>      (clientFactory, "VClient018");
        g_EntityList          = get_interface<IClientEntityList>   (clientFactory, "VClientEntityList003");
        g_Prediction          = get_interface<IPrediction>         (clientFactory, "VClientPrediction001");
        g_GameMovement        = get_interface<CGameMovement>       (clientFactory, "GameMovement001");
        g_MdlCache            = get_interface<IMDLCache>           (dataCacheFactory, "MDLCache004");
        g_EngineClient        = get_interface<IVEngineClient>      (engineFactory, "VEngineClient014");
        g_MdlInfo             = get_interface<IVModelInfoClient>   (engineFactory, "VModelInfoClient004");
        g_MdlRender           = get_interface<IVModelRender>       (engineFactory, "VEngineModel016");
        g_RenderView          = get_interface<IVRenderView>        (engineFactory, "VEngineRenderView014");
        g_EngineTrace         = get_interface<IEngineTrace>        (engineFactory, "EngineTraceClient004");
        g_DebugOverlay        = get_interface<IVDebugOverlay>      (engineFactory, "VDebugOverlay004");
        g_GameEvents          = get_interface<IGameEventManager2>  (engineFactory, "GAMEEVENTSMANAGER002");
        g_EngineSound         = get_interface<IEngineSound>        (engineFactory, "IEngineSoundClient003");
        g_MatSystem           = get_interface<IMaterialSystem>     (matSysFactory, "VMaterialSystem080");
        g_CVar                = get_interface<ICvar>               (valveStdFactory, "VEngineCvar007");
        g_VGuiPanel           = get_interface<IPanel>              (vgui2Factory, "VGUI_Panel009");
        g_VGuiSurface         = get_interface<ISurface>            (vguiFactory, "VGUI_Surface031");
        g_PhysSurface         = get_interface<IPhysicsSurfaceProps>(vphysicsFactory, "VPhysicsSurfaceProps001");
		g_StudioRender = get_interface< IStudioRender >(studioFactory, "VStudioRender026");
		g_Localize = get_interface<ILocalize>(localizeFactory, "Localize_001");
        g_pMemAlloc = *(IMemAlloc**)(GetProcAddress(GetModuleHandleW(L"tier0.dll"), "g_pMemAlloc"));

        auto client = GetModuleHandleW(L"client_panorama.dll");
        auto engine = GetModuleHandleW(L"engine.dll");
        auto dx9api = GetModuleHandleW(L"shaderapidx9.dll");

		g_PlayerResource = **reinterpret_cast<DT_CSPlayerResource * **>(Utils::PatternScan(client, "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7") + 2);
        g_GlobalVars      =  **(CGlobalVarsBase***)(Utils::PatternScan(client, "A1 ? ? ? ? 5E 8B 40 10") + 1);
		g_ClientMode	  = *(IClientMode * *)(Utils::PatternScan(client, "B9 ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 53") + 1);
        g_Input           =             *(CInput**)(Utils::PatternScan(client, "B9 ? ? ? ? 8B 40 38 FF D0 84 C0 0F 85") + 1);
        g_MoveHelper      =      **(IMoveHelper***)(Utils::PatternScan(client, "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01") + 2);
        g_GlowObjManager  = *(CGlowObjectManager**)(Utils::PatternScan(client, "0F 11 05 ? ? ? ? 83 C8 01") + 3);
		g_ViewRender      =        *(IViewRender**)(Utils::PatternScan(client, "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10") + 1);
        g_D3DDevice9      = **(IDirect3DDevice9***)(Utils::PatternScan(dx9api, "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
        g_ClientState     =     **(CClientState***)(Utils::PatternScan(engine, "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);
g_ClientSideAnimationList = (CUtlVectorSimple*)*(DWORD*)(Utils::PatternScan(client, "A1 ? ? ? ? F6 44 F0 04 01 74 0B") + 1);
		g_RenderBeams     = *(IViewRenderBeams**)(Utils::PatternScan(client, "A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 0x1);
		g_LocalPlayer     = *(C_LocalPlayer*)(Utils::PatternScan(client, "8B 0D ? ? ? ? 83 FF FF 74 07") + 2);
		g_GameRules = *(CGameRules * *)(Utils::PatternScan(client, "8B 0D ?? ?? ?? ?? 85 C0 74 0A 8B 01 FF 50 78 83 C0 54") + 2); //ne rpabatoet((((((()(()())()(()()())((((((()))))))))))))(((((((((())))))))))
    }


    void Dump()
    {
        // Ugly macros ugh
#define STRINGIFY_IMPL(s) #s
#define STRINGIFY(s)      STRINGIFY_IMPL(s)
#define PRINT_INTERFACE(name) Utils::ConsolePrint("%-20s: %p\n", STRINGIFY(name), name)

        PRINT_INTERFACE(g_CHLClient   );
        PRINT_INTERFACE(g_EntityList  );
        PRINT_INTERFACE(g_Prediction  );
        PRINT_INTERFACE(g_GameMovement);
        PRINT_INTERFACE(g_MdlCache    );
        PRINT_INTERFACE(g_EngineClient);
        PRINT_INTERFACE(g_MdlInfo     );
        PRINT_INTERFACE(g_MdlRender   );
        PRINT_INTERFACE(g_RenderView  );
        PRINT_INTERFACE(g_EngineTrace );
        PRINT_INTERFACE(g_DebugOverlay);
        PRINT_INTERFACE(g_GameEvents  );
        PRINT_INTERFACE(g_EngineSound );
        PRINT_INTERFACE(g_MatSystem   );
        PRINT_INTERFACE(g_CVar        );
        PRINT_INTERFACE(g_VGuiPanel   );
        PRINT_INTERFACE(g_VGuiSurface );
        PRINT_INTERFACE(g_PhysSurface );
        PRINT_INTERFACE(g_RenderBeams );
		PRINT_INTERFACE(g_StudioRender);
        PRINT_INTERFACE(g_pMemAlloc   );
    }
}