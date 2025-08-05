#pragma once
#include "GameProtocol.h"
#include <vector>
#include <map>
#include <string>
#include <WS2tcpip.h>


class GameState
{
public:
	GameState();
	bool addPlayer(const sockaddr_in& clientAddr, uint32_t& playerId);
	void processInput(const InputPacket& inputPacket);
	void update();
	const StatePacket& getState() const;
	const std::map<std::string, sockaddr_in>& getClients() const;

private:
	std::vector<Player> players;
	std::map<std::string, sockaddr_in> clients;
	StatePacket state;
	uint32_t nextPlayerId;
};