#pragma once
#include <cstdint>
#include <string>

const uint32_t MAX_PACKET_SIZE = 512; // Max size of UDP packet

// Packet types for different messages
enum class PacketType : uint8_t
{
	MESSAGE = 0, // Regular message
	// Add more types later
};

// Packet structure for messages
struct Packet
{
	PacketType type;
	uint32_t timestamp; // For ordering messages
	char data[256]; // Data payload

	// Serialize packet to buffer for sending
	void serialize(char* buffer) const
	{
		size_t offset = 0;
		memcpy(buffer + offset, &type, sizeof(type));
		offset += sizeof(type);
		memcpy(buffer + offset, &timestamp, sizeof(timestamp));
		offset += sizeof(timestamp);
		memcpy(buffer + offset, data, sizeof(data));
	}


	// Deserialize packet from buffer received
	void deserialize(const char* buffer)
	{
		size_t offset = 0;
		memcpy(&type, buffer + offset, sizeof(type));
		offset += sizeof(type);
		memcpy(&timestamp, buffer + offset, sizeof(timestamp));
		offset += sizeof(timestamp);
		memcpy(data, buffer + offset, sizeof(data));
	}
};
