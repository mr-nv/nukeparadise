
#include "render.hpp"

#include <mutex>

#include "features/visuals.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "menu_helpers.hpp"
#include "options.hpp"
#include "droid.hpp"
#include "helpers/math.hpp"

ImFont* g_pDefaultFont;
ImFont* g_pC4Font;
ImFont* g_pIconFont;
ImFont* g_ManualAAFont;

CRITICAL_SECTION render_cs;

Render::Render()
{
    InitializeCriticalSection(&render_cs);
}

ImDrawListSharedData _data;

std::mutex render_mutex;

void Render::Initialize()
{
    ImGui::CreateContext();

	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

    _data = ImDrawListSharedData();

    draw_list = new ImDrawList(&_data);
    draw_list_act = new ImDrawList(&_data);
    draw_list_rendering = new ImDrawList(&_data);

    GetFonts();
}

void Render::GetFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryCompressedTTF(Droid_compressed_data, Droid_compressed_size, 15.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGuiFreeType::BuildFontAtlas(io.Fonts, 0x00);
}

void Render::ClearDrawList()
{
    render_mutex.lock();
    draw_list_act->Clear();
    render_mutex.unlock();
}

void Render::BeginScene()
{
    draw_list->Clear();
    draw_list->PushClipRectFullScreen();

    Visuals::Get().AddToDrawList();

    render_mutex.lock();
    *draw_list_act = *draw_list;
    render_mutex.unlock();
}

ImDrawList* Render::RenderScene()
{

    if (render_mutex.try_lock())
    {
        *draw_list_rendering = *draw_list_act;
        render_mutex.unlock();
    }

    return draw_list_rendering;
}


float Render::RenderText(const std::string& text, const ImVec2& pos, float size, Color color, bool center, ImFont* pFont)
{
    ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

    if (!pFont->ContainerAtlas)
    {
        return 0.f;
    }

    draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

    if (center)
    {
        draw_list->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y) + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
        draw_list->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y) - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
        draw_list->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y) - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
        draw_list->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y) + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());

        draw_list->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y), GetU32(color), text.c_str());
    }
    else
    {
        draw_list->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y) + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
        draw_list->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y) - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
        draw_list->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y) - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
        draw_list->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y) + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());

        draw_list->AddText(pFont, size, ImVec2(pos.x, pos.y), GetU32(color), text.c_str());
    }

    draw_list->PopTextureID();

    return pos.y + textSize.y;
}

float Render::RenderTextNoOutline(const std::string& text, const ImVec2& pos, float size, Color color, bool center, ImFont* pFont)
{
    ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

    if (!pFont->ContainerAtlas)
    {
        return 0.f;
    }

    draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

    if (center)
    {
        draw_list->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y), GetU32(color), text.c_str());
    }
    else
    {
        draw_list->AddText(pFont, size, ImVec2(pos.x, pos.y), GetU32(color), text.c_str());
    }

    draw_list->PopTextureID();

    return pos.y + textSize.y;
}

void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
    float step = (float)M_PI * 2.0f / points;

    for (float a = 0; a < (M_PI * 2.0f); a += step)
    {
        Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
        Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

        Vector start2d, end2d;
        if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
        {
            return;
        }

        RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
    }
}
