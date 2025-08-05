#include "GameState.h"
#include <WS2tcpip.h>
#include <iostream>

GameState::GameState() : nextPlayerId(1)
{
	state.playerCount = 0;
	state.timestamp = 0;
}

bool GameState::addPlayer(const sockaddr_in& clientAddr, uint32_t& playerId)
{
	if (players.size() >= MAX_PLAYERS)
	{
		return false;
	}

	// Create a unique key for the client based on IP and port
	char clientIp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
	std::string clientKey = std::string(clientIp) + ":" + std::to_string(ntohs(clientAddr.sin_port));

	// Create a new player add starting position and health and save as a player and client with its address
	Player newPlayer;
	newPlayer.id = nextPlayerId++;
	newPlayer.x = 5.0f; // Starting position
	newPlayer.y = 5.0f;
	newPlayer.health = 100;
	players.push_back(newPlayer); // Add the player to the players vector
	clients[clientKey] = clientAddr; // Store the client address in the map that stores clients and their addresses

	
	// Update the state with the new player
	state.playerCount = static_cast<uint32_t>(players.size());
	for (size_t i = 0; i < players.size() && i < MAX_PLAYERS; ++i)
	{
		state.players[i] = players[i];
	}
	playerId = newPlayer.id;
	return true;
}

void GameState::processInput(const InputPacket& input)
{
	for (auto& player : players)
	{
		// Search the player that sent the input and update its position based on the input
		if (player.id == input.playerId)
		{
			float speed = 0.1f; // Movement speed in grid units (1.0f = 1 grid unit)
			if (input.moveUp) player.y -= speed;
			if (input.moveDown) player.y += speed;
			if (input.moveLeft) player.x -= speed;
			if (input.moveRight) player.x += speed;
			break;
		}
	}

	// Update state
	state.playerCount = static_cast<uint32_t>(players.size());
	for (size_t i = 0; i < players.size() && i < MAX_PLAYERS; ++i)
	{
		state.players[i] = players[i];
	}
	state.timestamp = static_cast<uint32_t>(time(nullptr)); // Update timestamp for ordering inputs and synchronization
}

void GameState::update()
{
	for (auto& player : players)
	{
		// Ensure players stay within bounds of the game area (0 to 10 in this case)
		if (player.x < 0) player.x = 0;
		if (player.y < 0) player.y = 0;
		if (player.x > 10) player.x = 10;
		if (player.y > 10) player.y = 10;
	}

	// Update state with the current player positions and timestamp
	state.playerCount = static_cast<uint32_t>(players.size());
	for (size_t i = 0; i < players.size() && i < MAX_PLAYERS; ++i)
	{
		state.players[i] = players[i];
	}
	state.timestamp = static_cast<uint32_t>(time(nullptr));
}

const StatePacket& GameState::getState() const
{
	return state;
}

const std::map<std::string, sockaddr_in>& GameState::getClients() const
{
	return clients;
}


