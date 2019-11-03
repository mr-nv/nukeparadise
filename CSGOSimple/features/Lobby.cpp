#include "../helpers/math.hpp"
#include "Lobby.hpp"
#include "../MovementFix.h"
#include "../options.hpp"
#include "../helpers/input.hpp"

auto Lobby::meme_inviter(bool get_collection_size) -> uint16_t {
#pragma pack(push, 1)
	struct item_t {
		uint16_t idx_next_0, unk_idx_2, idx_prev_4, unk_val_6;
		uint64_t steam_id_8;
		uint32_t* unk_ptr_16;
	}; //Size: 0x0014
#pragma pack(pop)

	static const auto collection = *(uint32_t**)(Utils::PatternScan(("client_panorama.dll"), "8B 35 ? ? ? ? 66 3B D0 74 07") + 2);

	if (*collection) {
		auto invite_to_lobby = [](uint64_t id) {
			class IMatchFramework; // GetMatchSession:13
			class ISteamMatchmaking; // InviteUserToLobby:16

			using GetLobbyId = uint64_t(__thiscall*)(void*);
			using GetMatchSession = uintptr_t * (__thiscall*)(IMatchFramework*);
			using InviteUserToLobby = bool(__thiscall*)(ISteamMatchmaking*, uint64_t, uint64_t);

			static const auto match_framework = **reinterpret_cast<IMatchFramework***>(Utils::PatternScan(("client_panorama.dll"), "8B 0D ? ? ? ? 8B 01 FF 50 2C 8D 4B 18") + 0x2);
			static const auto steam_matchmaking = **reinterpret_cast<ISteamMatchmaking***>(Utils::PatternScan(("client_panorama.dll"), "8B 3D ? ? ? ? 83 EC 08 8B 4D 08 8B C4") + 0x2);

			const auto match_session = CallVFunction<GetMatchSession>(match_framework, 13)(match_framework);
			if (match_session) {
				const uint64_t my_lobby_id = CallVFunction<GetLobbyId>(match_session, 4)(match_session);
				CallVFunction<InviteUserToLobby>(steam_matchmaking, 16)(steam_matchmaking, my_lobby_id, id);
				//Utils::CallVFunction<InviteUserToLobby>(steam_matchmaking, 16)(steam_matchmaking, my_lobby_id, id);
			}
		};

		auto max_index = ((uint16_t*)collection)[9];

		if (get_collection_size)
			return max_index;

		for (uint16_t i = 0; i <= max_index; ++i) {
			auto item = &((item_t*)*collection)[i];
			invite_to_lobby(item->steam_id_8);
		}

		return max_index;
	}

	return 0;
}