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
	INPUT_ACK = 5, // Server acknowledges input
};

struct Player
{
	uint32_t id;
	float x, y; // Player position
	int32_t health;
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
		InputAckPacket inputAck;
		StatePacket state;
		uint32_t playerId;
	};

	void serialize(char* buffer) const;
	void deserialize(const char* buffer);
};

struct InputAckPacket
{
	uint32_t playerId;
	uint32_t sequence; // Tracking the sequence number of the input being acknowledged

	void serialize(char* buffer) const;
	void deserialize(const char* buffer);
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
    uint32_t sequence; // Sequence number for input ordering

	void serialize(char* buffer) const;
	void deserialize(const char* buffer);
};

// Game state structure (server to clients)
struct StatePacket
{
	uint32_t playerCount;
	Player players[MAX_PLAYERS];
	uint32_t timestamp;

	void serialize(char* buffer) const;
	void deserialize(const char* buffer);
};


