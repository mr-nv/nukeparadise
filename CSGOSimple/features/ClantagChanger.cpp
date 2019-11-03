#include "ClantagChanger.h"
#include "../options.hpp"

std::string custom_clantag;

void ClantagChanger::DynamicClantag()
{
	static bool TriggerSwitch = false;
	static bool SwitchTag = false;

	static float LastChangeTime = 0.f;
	static float TimeToChangeTag = 0.f;

	if (g_GlobalVars->realtime - LastChangeTime < 0.5f)
		return;

	LastChangeTime = g_GlobalVars->realtime;
	int ServerTime = (float)g_GlobalVars->interval_per_tick * g_LocalPlayer->m_nTickBase() * 1.8;
	int value = ServerTime % 200;
	switch (value) { //$$$$$$$$$
	case 1: SwitchTag = false; break;
	case 11: SwitchTag = false; break;
	case 21: SwitchTag = false; break;
	case 31: SwitchTag = false; break;
	case 41: SwitchTag = false; break;
	case 51: SwitchTag = false; break;
	case 61: SwitchTag = false; break;
	case 71: SwitchTag = false; break;
	case 81: SwitchTag = false; break;
	case 91: SwitchTag = false; break;
	case 101: SwitchTag = true; break;
	case 111: SwitchTag = true; break;
	case 121: SwitchTag = true; break;
	case 131: SwitchTag = true; break;
	case 141: SwitchTag = true; break;
	case 151: SwitchTag = true; break;
	case 161: SwitchTag = true; break;
	case 171: SwitchTag = true; break;
	case 181: SwitchTag = true; break;
	case 191: SwitchTag = true; break;
	}
		if (SwitchTag)
		{
			int ServerTime = (float)g_GlobalVars->interval_per_tick * g_LocalPlayer->m_nTickBase() * 1.8;
			int value = ServerTime % 12;
			switch (value) {
			case 0: Utils::SetClantag((" NUK3PARAD1S3 ")); break;
			case 1: Utils::SetClantag((" UK3PARAD1S3  ")); break;
			case 2: Utils::SetClantag((" K3PARAD1S3 N ")); break;
			case 3: Utils::SetClantag((" 3PARAD1S3 NU ")); break;
			case 4: Utils::SetClantag((" PARAD1S3 NUK ")); break;
			case 5: Utils::SetClantag((" ARAD1S3 NUK3P ")); break;
			case 6: Utils::SetClantag((" RAD1S3 NUK3PA ")); break;
			case 7: Utils::SetClantag((" AD1S3 NUK3PAR ")); break;
			case 8: Utils::SetClantag((" D1S3 NUK3PARA ")); break;
			case 9: Utils::SetClantag((" 1S3 NUK3PARAD ")); break;
			case 10:Utils::SetClantag((" S3 NUK3PARAD1 ")); break;
			case 11:Utils::SetClantag((" 3 NUK3PARAD1S ")); break;
			case 12:Utils::SetClantag(("  NUK3PARAD1S3 ")); break;
			}
		}
		else
		{
			int ServerTime = (float)g_GlobalVars->interval_per_tick * g_LocalPlayer->m_nTickBase() * 1.8;
			int value = ServerTime % 98;
			switch (value) {
			case 0: Utils::SetClantag(("OFICIALNYJ")); break;
			case 1: Utils::SetClantag(("VYEBATOR")); break;
			case 2: Utils::SetClantag(("TVOEJ")); break;
			case 3: Utils::SetClantag(("MAMKI")); break;
			case 4: Utils::SetClantag(("RAZORVAL")); break;
			case 5: Utils::SetClantag(("PIZDAK")); break;
			case 6: Utils::SetClantag(("TVOEJ")); break;
			case 7: Utils::SetClantag(("MAMKI")); break;
			case 8: Utils::SetClantag(("MAT IBU")); break;
			case 9: Utils::SetClantag(("KAK")); break;
			case 10: Utils::SetClantag(("KONCHINUYU")); break;
			case 11: Utils::SetClantag(("HUEM SBIL")); break;
			case 12: Utils::SetClantag(("TVOYU MAT")); break;
			case 13: Utils::SetClantag(("MOJ HUJ VHODIL")); break;
			case 14: Utils::SetClantag(("V PIZDU")); break;
			case 15: Utils::SetClantag(("TVOEJ MAMKI")); break;
			case 16: Utils::SetClantag(("MAT TVOYA")); break;
			case 17: Utils::SetClantag(("SASALA JOSTKO")); break;
			case 18: Utils::SetClantag(("TY ZACHEM MNE")); break;
			case 19: Utils::SetClantag(("HUJ SASAL")); break;
			case 20: Utils::SetClantag(("EBAL TVOYU")); break;
			case 21: Utils::SetClantag(("MALENKUY PIZDU")); break;
			case 22: Utils::SetClantag(("NA MOGILKE")); break;
			case 23: Utils::SetClantag(("TVOEJ MAMKI")); break;
			case 24: Utils::SetClantag(("MOJ HUJ KAK")); break;
			case 25: Utils::SetClantag(("LOH SASESH")); break;
			case 26: Utils::SetClantag(("CHET PIZDU")); break;
			case 27: Utils::SetClantag(("TVOYU POIMEL")); break;
			case 28: Utils::SetClantag(("O DA SASESH")); break;
			case 29: Utils::SetClantag(("TY KLASSNO")); break;
			case 30: Utils::SetClantag(("EBAL TYA TAK")); break;
			case 31: Utils::SetClantag(("DOLGO CHTO")); break;
			case 32: Utils::SetClantag(("TVOYA PIZDA")); break;
			case 33: Utils::SetClantag(("STALA KAK")); break;
			case 34: Utils::SetClantag(("CHERNAYA DYRA")); break;
			case 35: Utils::SetClantag(("MOJ HUJ")); break;
			case 36: Utils::SetClantag(("TVOYA IKONA")); break;
			case 37: Utils::SetClantag(("NA KOLENI")); break;
			case 38: Utils::SetClantag(("ZHESTKA POIBAL")); break;
			case 39: Utils::SetClantag(("TYA V NATURE")); break;
			case 40: Utils::SetClantag(("TVOYA MAT")); break;
			case 41: Utils::SetClantag(("ATSASYVALA MNE")); break;
			case 42: Utils::SetClantag(("HEHE CHET")); break;
			case 43: Utils::SetClantag(("TVOYA MAMKA")); break;
			case 44: Utils::SetClantag(("NA IZE SASET")); break;
			case 45: Utils::SetClantag(("TY PONIMAESH")); break;
			case 46: Utils::SetClantag(("CHTO PIZDA")); break;
			case 47: Utils::SetClantag(("TVOJ MAMKI")); break;
			case 48: Utils::SetClantag(("PRITYAGIVAET")); break;
			case 49: Utils::SetClantag(("MOJ HUJ KAK")); break;
			case 50: Utils::SetClantag(("MAGNIT")); break;
			case 51: Utils::SetClantag(("TVOYA MAT")); break;
			case 52: Utils::SetClantag(("KAK OBEZYANA")); break;
			case 53: Utils::SetClantag(("CYPLYAETSYA")); break;
			case 54: Utils::SetClantag(("ZA MOJ HUJ")); break;
			case 55: Utils::SetClantag(("TY AGNOSTIK")); break;
			case 56: Utils::SetClantag(("MOEGO HUYA")); break;
			case 57: Utils::SetClantag(("SLYSH EBANNIJ")); break;
			case 58: Utils::SetClantag(("LOSHOK")); break;
			case 59: Utils::SetClantag(("HUJ MOJ LOVI")); break;
			case 60: Utils::SetClantag(("SVYATOJ OTEC")); break;
			case 61: Utils::SetClantag(("TVOEJ PIZDY")); break;
			case 62: Utils::SetClantag(("SKULISH")); break;
			case 63: Utils::SetClantag(("NA MOEM HUE")); break;
			case 64: Utils::SetClantag(("PES PRYGAESH")); break;
			case 65: Utils::SetClantag(("KAK BROHA")); break;
			case 66: Utils::SetClantag(("EBASHU TVOE")); break;
			case 67: Utils::SetClantag(("OCHKO KAK")); break;
			case 68: Utils::SetClantag(("TERMINATOR")); break;
			case 69: Utils::SetClantag(("HUEM TYA")); break;
			case 70: Utils::SetClantag(("GONYAL KAK")); break;
			case 71: Utils::SetClantag(("SOLDAT V ARMII")); break;
			case 72: Utils::SetClantag(("TY EBLAN")); break;
			case 73: Utils::SetClantag(("PONIMAESH")); break;
			case 74: Utils::SetClantag(("CHTO TVOYA MAT")); break;
			case 75: Utils::SetClantag(("KAK SHURYGINA")); break;
			case 76: Utils::SetClantag(("VSEM DAET")); break;
			case 77: Utils::SetClantag(("TVOYU PIZDU")); break;
			case 78: Utils::SetClantag(("POHITELI")); break;
			case 79: Utils::SetClantag(("INOPLANITYANE")); break;
			case 80: Utils::SetClantag(("CHTOBY VYEBAT")); break;
			case 81: Utils::SetClantag(("TVOYA BABULYA")); break;
			case 82: Utils::SetClantag(("RASSKAZYVALA")); break;
			case 83: Utils::SetClantag(("KAK V NEE")); break;
			case 84: Utils::SetClantag(("POSILILIS")); break;
			case 85: Utils::SetClantag(("ENOTY I EBALI")); break;
			case 86: Utils::SetClantag(("EE PO NOCHAM")); break;
			case 87: Utils::SetClantag(("PODSOS")); break;
			case 88: Utils::SetClantag(("CARSKOGO HUYA")); break;
			case 89: Utils::SetClantag(("YA TYA SCAS")); break;
			case 90: Utils::SetClantag(("V NATURE")); break;
			case 91: Utils::SetClantag(("POEBAL DURU")); break;
			case 92: Utils::SetClantag(("EBU TYA")); break;
			case 93: Utils::SetClantag(("V BASSEJNE")); break;
			case 94: Utils::SetClantag(("S SINIMI")); break;
			case 95: Utils::SetClantag(("KITAMI")); break;
			case 96: Utils::SetClantag(("POSASYVAESH")); break;
			case 97: Utils::SetClantag(("CHET TY ZHUTKO")); break;
			case 98: Utils::SetClantag(("AHAHAHAHAHAHA")); break;
			}
		}
	
}

void ClantagChanger::CustomSilde()
{
	static float LastChangeTime = 0.f;
	static float TimeToChangeTag = 0.f;

	if (g_GlobalVars->realtime - LastChangeTime < 0.5f)
		return;

	LastChangeTime = g_GlobalVars->realtime;
	Marquee(custom_clantag);

	Utils::SetClantag(custom_clantag.data());
}

void ClantagChanger::Marquee(std::string& clantag)
{
	std::string temp = clantag;
	clantag.erase(0, 1);
	clantag += temp[0];
}

void ClantagChanger::OnCreateMove()
{
	if (!g_LocalPlayer)
		return;

	switch (g_Config.misc_clantagchanger_type)
	{
	case 0:
		return;
	case 1:
		DynamicClantag();
		break;
	case 3:
		CustomSilde();
		break;
	default:
		break;
	}
}

void ClantagChanger::SetCustomSlide(std::string clantag)
{
	custom_clantag = clantag;
}
