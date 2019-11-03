#include <Windows.h>
#include "../options.hpp"
#include "Resolver.h"
#include "HitPossitionHelper.h"
#include "../RuntimeSaver.h"
#include "Rbot.h"
#include "../features/visuals.hpp"
#include "../bass/dll.h"
#include "../bass/bass.h"

#pragma once
class EventHelper : public Singleton<EventHelper>
{
    class listener : public IGameEventListener2
    {
    public:
        void Start();
        void Stop();
        void FireGameEvent(IGameEvent* event) override;
        int GetEventDebugID(void) override;
    };
public:
    void init()
    {
        Listener.Start();
    }
    void unInit()
    {
        Listener.Stop();
    }
    void on_fire_event(IGameEvent* event)
    {
        Resolver::Get().OnFireEvent(event);
        HitPossitionHelper::Get().OnFireEvent(event);
        Rbot::Get().OnFireEvent(event);

        if (!strcmp(event->GetName(), "player_hurt"))
        {
            if (!g_Config.vis_misc_hitmarker)
            {
                return;
            }
            int attacker = event->GetInt("attacker");
            if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
            {	
				Visuals::Get().RunHitmarker();

				uint32_t hitsound = 0;

				switch (g_Config.vis_misc_hitmarker_sound)
				{
				case 0:
					hitsound = BASS::stream_sounds.cod;
					break;
				case 1:
					hitsound = BASS::stream_sounds.skeet;
					break;
				case 2:
					hitsound = BASS::stream_sounds.punch;
					break;
				case 3:
					hitsound = BASS::stream_sounds.metal;
					break;
				case 4:
					hitsound = BASS::stream_sounds.boom;
					break;
				case 5:
					g_EngineClient->ExecuteClientCmd("play resource/warning.wav");//$
					break;
				default:
					break;
				}
				if (hitsound && hitsound != 5)
				{
					BASS_ChannelSetAttribute(hitsound, BASS_ATTRIB_VOL, g_Config.vis_misc_hitmarker_volume / 100.f);
					BASS_ChannelPlay(hitsound, true);
				}
            }
        }

        if (!strcmp(event->GetName(), "bullet_impact"))
        {
            C_BasePlayer* shooter = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

            if (!shooter || shooter != g_LocalPlayer)
            {
                return;
            }
            Vector p = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
            ShotTracer(g_Saver.LastShotEyePos, p);
        }

    }
    void ShotTracer(Vector shot_pos, Vector hit_pos);
private:
    listener Listener;
};

