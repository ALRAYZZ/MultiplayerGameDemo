#include "GameState.h"
#include <WS2tcpip.h>
#include <iostream>
#include <algorithm>

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
	inputBuffers[newPlayer.id] = std::vector<PlayerInput>(); // Initialize input buffer for the player
	lastProcessedSequences[newPlayer.id] = 0; // Initialize last processed sequence for the player

	
	// Update the state with the new player
	state.playerCount = static_cast<uint32_t>(players.size());
	for (size_t i = 0; i < players.size() && i < MAX_PLAYERS; ++i)
	{
		state.players[i] = players[i];
	}
	playerId = newPlayer.id;
	return true;
}

void GameState::queueInput(const InputPacket& input)
{
	// If the input sequence is bigger than the last processed, send it to the input buffer
	if (input.sequence > lastProcessedSequences[input.playerId])
	{
		PlayerInput playerInput;
		playerInput.input = input;
		playerInput.sequence = input.sequence;
		playerInput.timestamp = input.timestamp;
		inputBuffers[input.playerId].push_back(playerInput);
		std::cout << "[SERVER] Player " << input.playerId << " sent input seq " << input.sequence << " (last processed = " << lastProcessedSequences[input.playerId] << ")" << std::endl;
	}
}

void GameState::processInput()
{
	for (auto& player : players)
	{
		auto& buffer = inputBuffers[player.id];

		// Sort inputs by timestamp to ensure chronological processing
		std::sort(buffer.begin(), buffer.end(), [](const PlayerInput& a, const PlayerInput& b)
			{
				return a.timestamp < b.timestamp;
			});

		while (!buffer.empty())
		{
			const PlayerInput& playerInput = buffer.front();
			if (playerInput.sequence <= lastProcessedSequences[player.id])
			{
				buffer.erase(buffer.begin()); // Discard outdate inputs
				continue;
			}

			const InputPacket& input = playerInput.input;
			std::cout << "[SERVER] Processing input seq=" << input.sequence	<< " for player " << player.id << std::endl;
			float moveSpeed = 1.0f; // Define a movement speed
			if (input.moveUp) player.y -= moveSpeed;
			if (input.moveDown) player.y += moveSpeed;
			if (input.moveLeft) player.x -= moveSpeed;
			if (input.moveRight) player.x += moveSpeed;

			lastProcessedSequences[player.id] = playerInput.sequence; // Update last processed sequence
			buffer.erase(buffer.begin()); // Remove the processed input
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

uint32_t GameState::getLastProcessedSequence(uint32_t playerId) const
{
	auto it = lastProcessedSequences.find(playerId);
	return it != lastProcessedSequences.end() ? it->second : 0;
}

