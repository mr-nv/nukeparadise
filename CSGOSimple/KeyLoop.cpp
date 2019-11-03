
#include "KeyLoop.h"
#include "config.hpp"
#include "helpers/input.hpp"
#include "RuntimeSaver.h"
#include "options.hpp"

void KeyLoop::OnCreateMove()
{
    if(InputSys::Get().WasKeyPressed(g_Config.vis_misc_thirdperson_hotkey))
    {
        g_Config.vis_misc_thirdperson = !g_Config.vis_misc_thirdperson;
    }

	if (g_Config.misc_fakelag_on_key)
	{
		if (InputSys::Get().IsKeyDown(g_Config.misc_fakelag_key))
		{
			g_Config.misc_fakelag_enable = true;
		}
		else
		{
			g_Config.misc_fakelag_enable = false;
		}
	}

    if (InputSys::Get().WasKeyPressed(g_Config.rbot_manual_key_right))
    {
        if(g_Config.rbot_manual_aa_state == 1)
        {
            g_Config.rbot_manual_aa_state = 0;
        }
        else
        {
            g_Config.rbot_manual_aa_state = 1;
        }
        g_Saver.CurrentShouldSkipAnimations = true;
    }
    if (InputSys::Get().WasKeyPressed(g_Config.rbot_manual_key_left))
    {
        if (g_Config.rbot_manual_aa_state == 2)
        {
            g_Config.rbot_manual_aa_state = 0;
        }
        else
        {
            g_Config.rbot_manual_aa_state = 2;
        }
        g_Saver.CurrentShouldSkipAnimations = true;
    }
    if (InputSys::Get().WasKeyPressed(g_Config.rbot_manual_key_back))
    {
        if (g_Config.rbot_manual_aa_state == 3)
        {
            g_Config.rbot_manual_aa_state = 0;
        }
        else
        {
            g_Config.rbot_manual_aa_state = 3;
        }
        g_Saver.CurrentShouldSkipAnimations = true;
    }
}