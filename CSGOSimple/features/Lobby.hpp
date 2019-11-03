#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"

#pragma once
class Lobby : public Singleton<Lobby>
{
public:
	auto meme_inviter(bool get_collection_size)->uint16_t;
};

