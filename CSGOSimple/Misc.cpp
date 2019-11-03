#include "Misc.h"
#include "features/Rbot.h"
#include "config.hpp"
#include "options.hpp"
#include <string>
#include "features/Logger.h"
#include "./helpers/bfReader.hpp"
#include <time.h>
#include "RuntimeSaver.h"
#include <chrono>
#include "helpers/input.hpp"

//troll
std::vector<std::string> msgs =
{
"NU CHTO TY TAM OLUH EBANUTIJ ESCE JIV ILI MNE POZVAT` MOJ HUJ NADOET TEBE PO SCAM ))",
"TY CHO KON` MYCHISH` NA MOJ HUJ BYCHISH`,SLYSH VASYA NAHUJ YA TEBYA VSKROYU BLYAT`,TY NA ORGANY POJDESH` DCP EBANOE)) I ZAL`YU TEBYA SPERMOJ DAJE ZASHIVAT` NE BUDU) SPERMIT TY KONCHENIJ))",
"POCHEMU TVOYA MAMASHKA EBETSYA V ZATYLOK ? )U NEYO NEBRITAYA PIZDEN` I VOOBSCE MNE EBAT` EYO STYDNO I ZA TO CHTO SYN U NEYO CHLENOSOSNIK TOJE KRASNET` PRIHODICA))",
"TY LOH) TEBYA OBVESTI VOKRUG OCHKA TVOEJ MAMASHKI PROSTO IZI)  NA RAZ DVA SPOKOJNO)",
"TVOYA MAT` EBETSYA S PRAVYMI NO IZMENYAET IM S HACHAMI ? )KAKOJ TOLK V TOM,CHTO TVOYA MAT` JERTVA AKUSHERKI ? ))",
"CHMO – CHURKA MUDASKAYA OBOSRANKA,ETO PRO TVOYU MAMASHKU))",
"TVOYA MAMON`KA MAMONT EBANIJ)) TY PONIMAESH` CHTO ETO EYO CYGANE PERDUY TAK NAKACHALI SVOIM GAZOM)",
"TY ZNAESH` CHTO TVOYA MAT` NEODNOKRATNO NARUSHALA PRAVILA DOROJNOGO DVIJENIYA I ZA ETO BYLA NAKAZANA SOTRUDNIKOM DOROJNOJ POLICII ? ) A TOCHNEE ONI VYEBALI EYO PRYAM NA PROEZJEJ CHASTI NA KAPOTE MASHINY)))))",
"SLYSH TY TUPAYA KURVA TY V KURSE CHTO MENYA HOTYAT POSADIT` ZA IZNASILOVANIE VSEJ TVOEJ EBUCHEJ SEMEJKI)) NO POCHEMU - TO TVOYA MAT` V VOSTOORGE)",
"TY EBANASHKA YA UJE SVOJ HUJ PODMOROZIL) PODGONISH` SCA SVOYU MAMASHKU BUDU EYO NA ZADNEM SIDENII SVOEGO AVTO SHPARIT` HUEM KAK PAROVARKOJ)))",
"TVOYA MAMASHKA OVOSCEM STALA EYO GUBOCHKI USTALI SOSAT` A POLOVYE VOOBSCE STERLIS`… TAM POL MOSKVY POBYVALO… NERUSSKIE VSE POCHLENNO….",
"ETO TVOYA MAT` CYGANOCHKA MODNOGO TIPA NAUCHILA TEBYA JEVAT` MENTOL PERED TEM KAK TY DELAESH` MINET)) MMM,MOJ HUJ PROSTO V VOSTORGE!!!!",
"BUDULAJ UTYRCHATIJ))U MENYA SUKA SPINNOGO MOZGA BOL`SHE CHEM U TEBYA I TVOEJ MAMASHKI GOLOVNOGO,KRETINY SUKA))",
"CHTO TY OBORMOTISCE POZARILSYA NA MOYU MAMU ? ))NU VED` ONA MNE I TAK IZO DNYA RASSKAZYVAET KAK TEBYA TARABANIT V ZAD BIVNEM SLONOV`IM))",
" TY ZALUPU MOYU SEBE NA US NAMOTAL,HITROJOPIJ SLIPEZDREN`)) NO ETO NE DELAET TEBYA UMNYM - TY VSYO TAKOJ JE TUPORYLIJ DEGRDANTISCE))))",
"NA MOEM HUYU SIDYA,TY KAKUYU RELIGIYU ISPOVEDUESH`?))) POCHEMU GUS` - ETO IDOL I TY POKLONYAESH`SYA EMUUU ? )))",
"TY SCA DER`MO S JOPY SVOEJ MAMUEN`KI NANASOSAVSHEJSYA MOEGO HUYA BUDESH` SLIZYVAT`,A NU JIVO PRISTUPIL YAZYCHKOM ZA RABOTU!!!SLIZNYAK TY EBANIJ!!!!!!",
"TY JE POMNISH` TOT DEN`,KOGDA YA NACHAL TEBYA RASSTRELIVAT` IZ SVOEJ PUSHKI - SVOEGO CHLENA)) TY PROSTO KIBERSOSALKA))",
"TY HULI PIZDENKOJ SVOEJ TUT SVETISH`,TY NEZABUDKA KAK BUDUN PERHUNA NE KANAESH` UJE)) YA SVOJ CHLEN TEBE POKAJU TY PARNOKOPYTNOE SVOI KOPYTA OTBROSISH`)))YA SEGODNYA KSTATI USTRAIVAYU ROMANTIK S TVOEJ MAMOJ A POTOM TRAHNU EYO POD LUNOJ.)",
"TY OSTANESH`SYA V MOEJ PAMYATI KAK VERNIJ PES S MOEJ SPERMOJ NA GUBAH) A VOOBSCE TY HUJLO EBANOE BLYADKA ZA KALYADKI OTDALAS` TY,FU POZORISH` VES` ROD MUJSKOJ))))",
"YA SVOIM HUEM ZADNEPRIVODNOE TY OTROD`E PROVEDU ELEKTRICHESTVO DO TVOEJ MAMOCHKI)) SUCHKA ONA TOL`KO VYSOKOVOL`TNYM DAET,A YA ELEKTRO)))",
"HAHAH,NU TY OPEZDOL SLIVOCHNIJ)) TVOYA MAMAN UJE S GEMATOMAMI NA GUBAH OT MOEGO HUYA))",
"TY HULI PIZDABLOSHKA PIZDELYAKAESH` TUT ? ) TY JE PIZDABRATSKIJ PIZDOPLYAS))))",
"TY BLYAT` UJE OT NEDOSTATKA HUEV VO RTU TVOVOE NACHAL ZANIMAT`SYA HUEPECHATANIEM))))VAHAHAH)))",
"TY MUDAK SOSIHUJSKIJ YA J RASPIZDOSHU TVOYU MAT` POLOUMNUYU))",
"TY KAK GERKULES,TOL`KO HUELES))SILEN V SOSAINII TOL`KO,HUEGLOTISCE BLT)))",
"AH TY J EBANIJ STOS!!TVOYA MAT` - EBAL`NIJ STANOK))",
"INTENSIVNOST` TVOEGO NETEMATICHESKOGO PIZDEJA OBRATNA PROPORCIONAL`NA JIZNI TVOEJ MAMOCHKI NA MOEM CHLENE)))))",
"DA TY BLYAT` ALAPEZDYR`))TVOYA MAMKA VYTVORYAET AKROBATICHESKIE TRYUKI SVOIM RTOM NA MOEM HUYU)))",
"ANUSNAYA KAPEL`NICA - TVOYA MAMUL`KA)))))",
"TY ARHIPIZDRIT IPANIJ SCA V PIZDAK SVOEJ MAMY POPLYVESH`)))YA CELUYU REKU TEBE NASSU) VODOKANAL BLEAT`)))",
"HULI NEGRY TVOYU MAT` V POZICII VALET KRYSHUYUT ? ))))",
"TY VAFLIST ))I MAMASHU YA TVOYU VZEBU NA SVOJ ELDAK:3",
"EEE, VOS`MIKRYLIJ SEMIHUJ ,YA SCA TVOEJ MATERI VPERDOLYU PO SAMYE POMIDORY))))",
"SLYSH TY GANDURAS,YA TEBE SCA GANDOSHKU NA OCHKO NATYANU))BUDESH` LIZAT` GASHETKU SVOEJ MAMAN)))",
"TY GOVNOPIDOR NE PROCH` POEST` SOBACHIH FEKALIJ)))A TVOYA MAMA GOVNOROJAYA HUEPLYOTKA))))",
"TVOYA MAMANYA DERGANNAYA PIZDA)))ONA ESCE I DVUJOPAYA MUTANTKA)))",
"TY DIBILOS NAHUJ))A TVOYA MAMA - PIZDOPROEBOCHKA S 10 DNEVNOJ SCETINOJ))))",
"TY DOSKAEB)))TVOYA MAMKA TAKAYA PLOSKAYA((MRAZINA NEBROSKAYA))))",
"SCA TVOYU MAMU BUDEM DRAT` S POCANAMI V 4 SMYCHKA))) ONA JE U NAS MUZYKANTSHA))))",
"TY DROCHUL`EN)))KAJDIJ DEN` DROKAESH` NA TO KAK YA TVOYU MAMU V TRI PIZDY EBU I NE MOJESH` NICHO SKAZAT`, SSYKUCHEE TY VESCESTVO)))",
"TY SDRYSNESH` SCA SUCHKA))) MOE DULO TVOJ ROT PRODULO)))",
"TVOJ ANAL - DYMOHOD KAK VULKAN IZVERGAETSYA TOL`KO TOL`KO VYLETAET OTTUDA JIDKOST` TVOEJ MAMY KOGDA ONA SKVIRTIT))))",
"SCA TEBE EBAVKA PRILETIT V EBLO V FORME MOEGO HUYA)))",
"YA ZNAYU CHTO TY STRUCHOK SSANIJ OCHEN` JDESH` JAREVO MOEGO HUYA I TVOEJ MAMY KAK KITAJSKOJ PASKHI)))",
"TY JIBLIK DRANIJ A TVOYA MAT` JIVOGLOTKA)))GLOTAET HUI CHOT))) TO EST` JIVUYU PLOT`))",
"TY JIROBAS EBANIJ U TEBYA UJE KORNEVISCA OT HUYA VO RTU)) TY SUKA STRASHILISCE A TVOYA MAMA JIRTRESTOCHKA JOPALIZOCHKA)))",
"EE,JOPOGOLOVIJ))TY JOPITO,U TEBYA POLOJENIE BEZVYHODNOE))EDINSTVENNOE CHTO YA TEBE RAZRESHAYU,MOJ SLUGA,TAK ETO BYT` PIDOROCHKOM))))",
"TVOYA MAMULECHKA JOSYA)))TY TAK JE OCH AKTIVNO PROMYSHLYAESH` PASSIVNOJ PIDORASNEJ,FU))",
"TY CHO ORESH`,CHTO LYUBISH` JURNYU ? ))JURNYA - ETO KOGDA TEBYA MERTVEC VSEM DAYUSCIJ PRUT V ZAD,A YA ESCE VIDEL NEDAVNO KAK TY V RAVNYH OBEMAH SO SVOEJ MAMOJ VZAIMODEJSTVUESH` RTOM S HUEM MOIM?))))",
"YA SCA TEBYA SUKA JUCHIT` BUDU)))VOZ`MU ZDOROVENNIJ ELDACHELLO I PUST` TVOJ ANAL`NIJ SFINKTER POKOITSYA S MIROM))))",
"DA TVOYA MAMOCHKA ZASFAL`TIROVANNAYA,ZABLYADOVALAS` ONA))SCA POIGRAYU S NEJ V HOKKEJ)))BUDU ZABIVAT` V NEYO SHAJBY SVOIM HUEM)))",
"SCA YA TVOYU MAMU DURU ZAGONYU POD SHKURU))DA EYO LYUBOJ ZAGOVNIT)))",
"TVOYA MAT` ZADNICA S USHAMI)))A TY GLUPEN`KIJ ZADRISTYSH`))",
"DA MAMOCHKA TVOYA - KAPTYORKA OGOROJENNAYA MOIM HUEM)) A TY EYO VERNIJ POMOSHNIK - MINETCHIK)))",
"DA TY SUKA EBANIJ DRAKULA)) TOL`KO ON KROV` SOSET,A TY SPERMU VYSASYVAESH` IZ SVOEJ ZAEBANNOJ KURICY MAMY)))))",
"DA TY UJE IVAN TOSKUN ? )) U TEBYA BOLI V JELUDKE OT MOEJ SPERMOJ)))",
"YA IGRAYU NA ZAMANKU S TVOEJ MAMOJ))) VTYANU EYO NA SVOJ HUJ PONIMAESH`?))",
"TY TUT PANT B`ESH`,PONYATNO)) A NA DELE TY PORVANNAYA KURTIZANKA))))",
"POSADIT` TEBYA NA KOL ZNACHIT DOSTAVIT` TEBE UDOVOL`STVIE!",
"YA BY POMOLCHAL, NO TVOJ UROVEN` IQ NE POZVOLYAET MNE OSTAVIT` TVOYU RECH` BEZ KOMENTARIEV.",
"TVOYA SMELOST` DOSTOJNA UVAJENIYA,  TVOYA POPKA DOSTOJNA VOSKHISHENIYA!",
"MNE KAJETSYA TY MASTURBIRUESH` NA MOJ GOLOS, NAVERNO POETOMU TY MOLCHISH`!",
"TY SEJCHAS NE ODOLJENIE MNE SDELAL, TY VYPOLNIL MOJ PRIKAZ.",
"TVOYA MAMKA NE MOJET TEBE UDELIT` VNIMANIYA I OTVESTI TEBYA K PSIHIATRU, NU YA MOGU TEBE POMOCH`, UEBAT` TEBYA BITOJ PO GOLOVE, BOLEZN` YA NE VYLECHU, NO MIR OT TEBYA SPASU!",
"TVOJ MOZG VYDAET OSHIBKU, KOGDA YA POSYLAYU V NEGO NOVUYU INFORMACIYU, A VSE POTOMU CHTO V NEM NE DOSTATOCHNO OPERATIVNOJ PAMYATI.",
"TVOI DOSTOINSTVA ETO SOSAT` HUJ, RAZDAVAT` JOPU, LIZAT` GOKOLADNIJ GLAZ U DRUZEJ!",
"PO MOEJ MILOSTI TY ESCE GOVORISH`, PO MOEJ PRIHOTI TY STOISH` NA KOLENYAH, PO MOEMU PRIKAZU TY SPRYGNESH` S KRYSHI!",
"YA NAUCHU TEBYA KUN - FU, PROSTO VYRUBLYU TEBYA S VERTUSHKI.",
"TY PROSTO MUTNAYA SUBSTANCIYA IN`, KOTORAYA PRITVORYAETSYA V ZEMLE"
};

void Misc::aimware_fix_mouse_dx(CUserCmd* cmd)
{
	static ConVar* m_yaw = m_yaw = g_CVar->FindVar("m_yaw");
	static ConVar* m_pitch = m_pitch = g_CVar->FindVar("m_pitch");
	static ConVar* sensitivity = sensitivity = g_CVar->FindVar("sensitivity");

	static Vector m_angOldViewangles = g_ClientState->viewangles;

	float delta_x = std::remainderf(cmd->viewangles.pitch - m_angOldViewangles.x, 360.0f);
	float delta_y = std::remainderf(cmd->viewangles.yaw - m_angOldViewangles.y, 360.0f);

	if (delta_x != 0.0f) {
		float mouse_y = -((delta_x / m_pitch->GetFloat()) / sensitivity->GetFloat());
		short mousedy;
		if (mouse_y <= 32767.0f) {
			if (mouse_y >= -32768.0f) {
				if (mouse_y >= 1.0f || mouse_y < 0.0f) {
					if (mouse_y <= -1.0f || mouse_y > 0.0f)
						mousedy = static_cast<short>(mouse_y);
					else
						mousedy = -1;
				}
				else {
					mousedy = 1;
				}
			}
			else {
				mousedy = 0x8000u;
			}
		}
		else {
			mousedy = 0x7FFF;
		}

		cmd->mousedy = mousedy;
	}

	if (delta_y != 0.0f) {
		float mouse_x = -((delta_y / m_yaw->GetFloat()) / sensitivity->GetFloat());
		short mousedx;
		if (mouse_x <= 32767.0f) {
			if (mouse_x >= -32768.0f) {
				if (mouse_x >= 1.0f || mouse_x < 0.0f) {
					if (mouse_x <= -1.0f || mouse_x > 0.0f)
						mousedx = static_cast<short>(mouse_x);
					else
						mousedx = -1;
				}
				else {
					mousedx = 1;
				}
			}
			else {
				mousedx = 0x8000u;
			}
		}
		else {
			mousedx = 0x7FFF;
		}

		cmd->mousedx = mousedx;
	}
}

void Misc::OnCreateMove(CUserCmd* cmd)
{
	if (g_Config.misc_name_stealer) Steal(cmd);

	if (g_Config.misc_knifebot) KnifeBot(cmd);

	if (g_Config.misc_automatic_weapons) AutoWeapons(cmd);

	if (!g_LocalPlayer || !g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive()) return;

	if (g_Config.misc_airstuck && InputSys::Get().IsKeyDown(g_Config.misc_airstuck_key)) Airstuck(cmd);

}

void Misc::LegitResolver()
{
	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(i);
		if (pPlayer && pPlayer->IsPlayer() && pPlayer != g_LocalPlayer)
		{
			pPlayer->m_angEyeAngles().yaw = pPlayer->m_flLowerBodyYawTarget();
			pPlayer->m_angEyeAngles().Normalize();
		}
	}

}

void Misc::OnFrameStageNotify(ClientFrameStage_t stage)
{
	switch (stage)
	{
	case FRAME_UNDEFINED:
		break;
	case FRAME_START:

		static ConVar* PostProcVar = g_CVar->FindVar("mat_postprocess_enable");
		if (!g_Config.vis_misc_disable_postprocessing || g_Unload)
			PostProcVar->SetValue(0);
		else
			PostProcVar->SetValue(1);

		static ConVar* zoom_sensitivity_ratio_mouse = g_CVar->FindVar("zoom_sensitivity_ratio_mouse");
		if (!g_Config.vis_misc_disable_scope_zoom || g_Unload)
			zoom_sensitivity_ratio_mouse->SetValue(1);
		else
			zoom_sensitivity_ratio_mouse->SetValue(0);

		static ConVar* mat_fullbright = g_CVar->FindVar("mat_fullbright");
		if (!g_Config.vis_misc_fullbright || g_Unload)
			mat_fullbright->SetValue(0);
		else
			mat_fullbright->SetValue(1);

		static ConVar* weapon_debug_spread_show = g_CVar->FindVar("weapon_debug_spread_show");
		if (g_Config.vis_misc_force_crosshair && !g_LocalPlayer->m_bIsScoped() && !g_Unload)
			weapon_debug_spread_show->SetValue(3);
		else
			weapon_debug_spread_show->SetValue(0);

		if (g_LocalPlayer && g_EngineClient->IsInGame() && !g_Unload)
		{
			if (g_Config.misc_chat_spammer > 0) ChatSpammer();
		}
		break;
	case FRAME_NET_UPDATE_START:
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		if (g_Config.misc_legit_resolver && InputSys::Get().IsKeyDown(g_Config.misc_legit_resolver_key)) LegitResolver();
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		break;
	case FRAME_NET_UPDATE_END:
		break;
	case FRAME_RENDER_START:
		if (!g_Unload) NoFlash();
		break;
	case FRAME_RENDER_END:
		if (g_Config.misc_cash_hack || g_Unload)
		{
			if (!g_LocalPlayer || !g_EngineClient->IsInGame()) return;
			g_LocalPlayer->m_iAccount() += g_Config.misc_cash_hack_val;
		}
		break;
	}
}

void Misc::AntiKick(Hooks::tDispatchUserMessage oDispatchUserMessage, PVOID ecx, int type, unsigned int a3, unsigned int length, const void* msg_data)
{
	//leet

	if (type == CS_UM_VoteStart)
	{
		if (!g_Config.misc_anti_kick) return;
		bf_read read = bf_read(reinterpret_cast<uintptr_t>(msg_data));
		read.SetOffset(6);
		read.Skip(29); //30 ?
		std::string message = read.ReadString();

		g_Logger.Log("ANTIKICK", Utils::Format("Detected message: '%s'", std::string(message)));

		player_info_t local_info;
		if (!g_EngineClient->GetPlayerInfo(g_EngineClient->GetLocalPlayer(), &local_info))
			return;

		if (message.find(local_info.szName) != std::string::npos)
		{
			switch (g_Saver.AntiKickMethod)
			{
			case 0:
				g_Saver.AntiKickMethod = 1;
				g_EngineClient->ExecuteClientCmd("callvote swapteams");
				g_Logger.Log("ANTIKICK", Utils::Format("Detected '%s' in message, trying swapteams method.", local_info.szName));
				break;
			case 1:
				g_Saver.AntiKickMethod = 0;
				g_EngineClient->ExecuteClientCmd("callvote scrambleteams");
				g_Logger.Log("ANTIKICK", Utils::Format("Detected '%s' in message, trying scrambleteams method.", local_info.szName));
				break;
			}
		}
	}
}

void Misc::NoFlash()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !g_Config.vis_misc_noflash)
	{
		return;
	}

	g_LocalPlayer->m_flFlashDuration() = 0.f;
}

void Misc::Kick(int userid)
{
	char command[1024]; sprintf(command, "callvote kick %d", userid);

	g_EngineClient->ExecuteClientCmd(command);
}

void Misc::LinearExtrapolationsrun()
{
	//why
	if (g_LocalPlayer && g_LocalPlayer->IsAlive()) {
		for (int i = 1; i < g_GlobalVars->maxClients; i++)
		{
			C_BasePlayer* m_entity = C_BasePlayer::GetPlayerByIndex(i);

			if (!m_entity || m_entity == g_LocalPlayer || m_entity->GetClientClass()->m_ClassID != CCSPlayer || !m_entity->IsAlive())
				continue;

			float simtime_delta = m_entity->m_flSimulationTime() - m_entity->m_flOldSimulationTime();
			int choked_ticks = Math::clamp(TIME_TO_TICKS(simtime_delta), 1, 15);
			Vector lastOrig;

			if (lastOrig.Length() != m_entity->m_vecOrigin().Length())
				lastOrig = m_entity->m_vecOrigin();

			float delta_distance = (m_entity->m_vecOrigin() - lastOrig).LengthSqr();
			if (delta_distance > 4096.f)
			{
				Vector velocity_per_tick = m_entity->m_vecVelocity() * g_GlobalVars->interval_per_tick;
				auto new_origin = m_entity->m_vecOrigin() + (velocity_per_tick * choked_ticks);
				m_entity->SetAbsOriginal(new_origin);
			}
		}
	}
}

void Misc::Steal(CUserCmd* cmd)
{
	static DWORD delay = 0;

	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
	{
		const auto pLocalEntity = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
		static auto start_t = clock();
		const auto timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
		if (timeSoFar < .5)
			return;
		std::vector<std::string> allName;

		if (pLocalEntity)
		{
			for (auto i = 1; i < g_EngineClient->GetMaxClients(); i++)
			{
				const auto pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
				if (!pBaseEntity) continue;
				player_info_t pInfo;
				g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
				if (pInfo.ishltv) continue;
				allName.emplace_back(pInfo.szName);
			}

			const int random_index = rand() % allName.size();
			char buffer[128];
			sprintf_s(buffer, "%s ", allName[random_index].c_str());
			const char* result = buffer;


			Utils::SetName(result);
			start_t = clock();

			if (GetTickCount() - delay > 4000)
			{
				delay = GetTickCount();
				Utils::SetName("\n\xAD\xAD\xAD");
			}
		}
	}
}

void Misc::ChatSpammer() //ebat ELITNO suuuka
{
	if (g_Config.misc_chat_spammer == 1)
	{
		static DWORD g_dTimer = 0;
		if (GetTickCount() > g_dTimer + 850)
		{
			g_EngineClient->ExecuteClientCmd(u8"say ﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽");
			g_dTimer = GetTickCount();
		}
	}

	if (g_Config.misc_chat_spammer == 2)
	{
		long curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		static long timestamp = curTime;

		if ((curTime - timestamp) < 850)
			return;

		if (g_Config.misc_chat_spammer)
		{
			if (msgs.empty())
				return;

			std::srand(time(NULL));

			std::string str;

			std::rotate(msgs.begin(), msgs.begin() + 1, msgs.end());

			str.append("say ");
			str.append(msgs[0].data());

			g_EngineClient->ExecuteClientCmd(str.c_str());
		}
		timestamp = curTime;
	}

}

void Misc::AutoWeapons(CUserCmd* cmd)
{
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) return;

	if (weapon->m_iClip1() <= 0 && !weapon->IsKnife() && cmd->buttons & IN_ATTACK)
	{
		auto type = g_LocalPlayer->m_hActiveWeapon();

		if ((weapon->m_iItemDefinitionIndex() == WEAPONTYPE_MACHINEGUN || weapon->m_iItemDefinitionIndex() == WEAPONTYPE_RIFLE || weapon->m_iItemDefinitionIndex() == WEAPONTYPE_SHOTGUN ||
			weapon->m_iItemDefinitionIndex() == WEAPONTYPE_SNIPER_RIFLE || weapon->m_iItemDefinitionIndex() == WEAPONTYPE_SUBMACHINEGUN) && !didSwitch) {
			g_EngineClient->ExecuteClientCmd("slot2");
			didSwitch = true;
		}
	}
	else
		didSwitch = false;
}

void Misc::Airstuck(CUserCmd* cmd)
{
	//bruh
	cmd->command_number = INT_MAX;
	cmd->tick_count = INT_MAX;
}

void Misc::KnifeBot(CUserCmd* cmd)
{
	//shit
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_EngineClient->IsInGame())
		return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) return;

	if (g_LocalPlayer->m_hActiveWeapon()->IsKnife())
	{
		for (auto i = 1; i < g_GlobalVars->maxClients; ++i)
		{
			auto pPlayer = C_BasePlayer::GetPlayerByIndex(i);

			if (pPlayer)
			{
				if (pPlayer->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
					continue;

				Vector vHead = pPlayer->GetHitboxPos(HITBOX_HEAD);
				Vector vOrigin = pPlayer->m_angAbsOrigin();
				Vector out1;
				Vector out2;

				if (Math::WorldToScreen(vOrigin, out1) && Math::WorldToScreen(vHead, out2))
				{
					bool bOriginScreen = (out1.x > 0 && out1.y > 0);
					bool bHitBoxScreen = (vHead.x > 0 && vHead.y > 0);

					if (bOriginScreen || bHitBoxScreen)
					{
						int MinDistance = g_Config.misc_knifebot_dist;
						if (MinDistance <= 0) return;
						int a = int(g_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin()));
						int DistanceToPlayer = a;

						if (DistanceToPlayer > MinDistance)
							continue;

						if (DistanceToPlayer > 64)
							cmd->buttons |= IN_ATTACK;

						else
							cmd->buttons |= IN_ATTACK2;
					}
				}
			}
		}
	}
}

void Misc::SlowWalkRun(CUserCmd* cmd)
{
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (weapon)
	{
		if (weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 || weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1)
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)	Misc::Get().SlowWalk(cmd, 30);

		if (weapon->m_iItemDefinitionIndex() == WEAPON_AWP)
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)	Misc::Get().SlowWalk(cmd, 33);

		if (weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)	Misc::Get().SlowWalk(cmd, 70);

		if (!weapon->IsSniper())
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)	Misc::Get().SlowWalk(cmd, 34);
	}
}

void Misc::SlowWalk(CUserCmd* cmd, float speed)
{
	if (g_LocalPlayer->m_nMoveType() == MoveType_t::MOVETYPE_NOCLIP) return;

	if (!g_Config.rbot_slowwalk || !InputSys::Get().IsKeyDown(g_Config.rbot_slowwalk_key))
		return;

	if (speed <= 0.f)
		return;

	float min_speed = (float)(Math::FASTSQRT((cmd->forwardmove) * (cmd->forwardmove) + (cmd->sidemove) * (cmd->sidemove) + (cmd->upmove) * (cmd->upmove)));
	if (min_speed <= 0.f)
		return;

	if (cmd->buttons & IN_DUCK)
		speed *= 2.94117647f;

	if (min_speed <= speed)
		return;

	float finalSpeed = (speed / min_speed) * g_Config.rbot_slowwalk_mod;

	cmd->forwardmove *= finalSpeed;
	cmd->sidemove *= finalSpeed;
	cmd->upmove *= finalSpeed;
}

void Misc::SlowWalk(CUserCmd* cmd)
{
	if (g_Config.rbot_slowwalk && InputSys::Get().IsKeyDown(g_Config.rbot_slowwalk_key))
	{
		float speed = g_Config.rbot_slowwalk_mod * 0.01f;
		auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
		if (weapon)
		{
			auto weapon_data = weapon->GetCSWeaponData();
			if (weapon_data)
			{
				float max_speed = weapon->m_weaponMode() == 0 ? weapon_data->flMaxPlayerSpeed : weapon_data->flMaxPlayerSpeedAlt;
				float ratio = max_speed / 250.0f;
				speed *= ratio;
			}
		}

		cmd->forwardmove *= speed;
		cmd->sidemove *= speed;
	}
}