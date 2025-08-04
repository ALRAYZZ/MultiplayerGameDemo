#pragma once
#include <cstdint>
#include <string>

const uint32_t MAX_PACKET_SIZE = 512; // Max size of UDP packet
const uint32_t MAX_PLAYERS = 4;

// Packet types for different messages
enum class PacketType : uint8_t
{
	MESSAGE = 0, // Regular message
	INPUT = 1, // Client sends input
	STATE = 2, // Server sends game state
	CONNECT = 3, // Clients requests to connect
	CONNECT_ACK = 4, // Server acknowledges connection with player ID
};

struct Player
{
	uint32_t id;
	float x, y; // Player position
	int32_t health;
};

// Input structure
struct InputPacket
{
	uint32_t playerId;
	bool moveUp;
	bool moveDown;
	bool moveLeft;
	bool moveRight;
	uint32_t timestamp; // For ordering inputs

	void serialize(char* buffer) const
	{
		size_t offset = 0;
		memcpy(buffer + offset, &playerId, sizeof(playerId)); // Student note (We doing pointer arithmetics since the buffer is a char array)
		offset += sizeof(playerId);
		memcpy(buffer + offset, &moveUp, sizeof(moveUp));
		offset += sizeof(moveUp);
		memcpy(buffer + offset, &moveDown, sizeof(moveDown));
		offset += sizeof(moveDown);
		memcpy(buffer + offset, &moveLeft, sizeof(moveLeft));
		offset += sizeof(moveLeft);
		memcpy(buffer + offset, &moveRight, sizeof(moveRight));
		offset += sizeof(moveRight);
		memcpy(buffer + offset, &timestamp, sizeof(timestamp));
	}

	void deserialize(const char* buffer)
	{
		size_t offset = 0;
		memcpy(&playerId, buffer + offset, sizeof(playerId));
		offset += sizeof(playerId);
		memcpy(&moveUp, buffer + offset, sizeof(moveUp));
		offset += sizeof(moveUp);
		memcpy(&moveDown, buffer + offset, sizeof(moveDown));
		offset += sizeof(moveDown);
		memcpy(&moveLeft, buffer + offset, sizeof(moveLeft));
		offset += sizeof(moveLeft);
		memcpy(&moveRight, buffer + offset, sizeof(moveRight));
		offset += sizeof(moveRight);
		memcpy(&timestamp, buffer + offset, sizeof(timestamp));
	}
};

// Game state structure (server to clients)
struct StatePacket
{
	uint32_t playerCount;
	Player players[MAX_PLAYERS];
	uint32_t timestamp;

	// Serialize packet to buffer for sending
	void serialize(char* buffer) const
	{
		size_t offset = 0;
		memcpy(buffer + offset, &playerCount, sizeof(playerCount));
		offset += sizeof(playerCount);
		memcpy(buffer + offset, &players, sizeof(players));
		offset += sizeof(players);
		memcpy(buffer + offset, &timestamp, sizeof(timestamp));
	}


	// Deserialize packet from buffer received
	void deserialize(const char* buffer) 
	{
		size_t offset = 0;
		memcpy(&playerCount, buffer + offset, sizeof(playerCount));
		offset += sizeof(playerCount);
		memcpy(&players, buffer + offset, sizeof(players));
		offset += sizeof(players);
		memcpy(&timestamp, buffer + offset, sizeof(timestamp));
	}
};

// This allows us to call packet.deserialize(buffer) and it will automatically call the correct deserialize function based on the packet type
struct Packet
{
	// Union allows us to save memory by using the same space for different types of data
	// Packet struct will only be one type at a time either message, input, state or connect...
	PacketType type;
	union 
	{
		char message[256];
		InputPacket input;
		StatePacket state;
		uint32_t playerId;
	};

	void serialize(char* buffer) const
	{
		uint32_t offset = 0;
		memcpy(buffer + offset, &type, sizeof(type));
		offset += sizeof(type);
		if (type == PacketType::MESSAGE)
		{
			memcpy(buffer + offset, &message, sizeof(message));
		}
		else if (type == PacketType::INPUT)
		{
			input.serialize(buffer + offset);
		}
		else if (type == PacketType::STATE)
		{
			state.serialize(buffer + offset);
		}
		else if (type == PacketType::CONNECT || type == PacketType::CONNECT_ACK)
		{
			memcpy(buffer + offset, &playerId, sizeof(playerId));
		}
	}
	
	void deserialize(const char* buffer)
	{
		uint32_t offset = 0;
		memcpy(&type, buffer + offset, sizeof(type));
		offset += sizeof(type);
		if (type == PacketType::MESSAGE)
		{
			memcpy(&message, buffer + offset, sizeof(message));
		}
		else if (type == PacketType::INPUT)
		{
			input.deserialize(buffer + offset);
		}
		else if (type == PacketType::STATE)
		{
			state.deserialize(buffer + offset);
		}
		else if (type == PacketType::CONNECT || type == PacketType::CONNECT_ACK)
		{
			memcpy(&playerId, buffer + offset, sizeof(playerId));
		}
	}
};
