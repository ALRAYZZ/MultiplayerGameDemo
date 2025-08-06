#pragma once
#include "GameProtocol.h"
#include <vector>
#include <map>
#include <string>
#include <WS2tcpip.h>
#include <queue>

class GameState
{
public:
	GameState();
	bool addPlayer(const sockaddr_in& clientAddr, uint32_t& playerId);
	void queueInput(const InputPacket& input);
	void processInput();
	void update();
	const StatePacket& getState() const;
	const std::map<std::string, sockaddr_in>& getClients() const;
	uint32_t getLastProcessedSequence(uint32_t playerId) const;

private:
	struct PlayerInput
	{
		InputPacket input;
		uint32_t sequence;
	};

	std::vector<Player> players;
	std::map<std::string, sockaddr_in> clients;
	std::map<uint32_t, std::queue<PlayerInput>> inputBuffers; // Per player input buffer with all inputs received
	std::map<uint32_t, uint32_t> lastProcessedSequences; // Track last processed sequence number
	StatePacket state;
	uint32_t nextPlayerId;
};