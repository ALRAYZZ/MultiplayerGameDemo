#include <GameProtocol.h>

void InputAckPacket::serialize(char* buffer) const
{
    size_t offset = 0;
    memcpy(buffer + offset, &playerId, sizeof(playerId));
	offset += sizeof(playerId);
    memcpy(buffer + offset, &sequence, sizeof(sequence));
}

void InputAckPacket::deserialize(const char* buffer)
{
    size_t offset = 0;
    memcpy(&playerId, buffer + offset, sizeof(playerId));
    offset += sizeof(playerId);
    memcpy(&sequence, buffer + offset, sizeof(sequence));
}

void InputPacket::serialize(char* buffer) const
{
    size_t offset = 0;
    memcpy(buffer + offset, &playerId, sizeof(playerId));
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
	offset += sizeof(timestamp);
	memcpy(buffer + offset, &sequence, sizeof(sequence));
}

void InputPacket::deserialize(const char* buffer)
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
	offset += sizeof(timestamp);
	memcpy(&sequence, buffer + offset, sizeof(sequence));
}

void StatePacket::serialize(char* buffer) const
{
    size_t offset = 0;
    memcpy(buffer + offset, &playerCount, sizeof(playerCount));
    offset += sizeof(playerCount);
    memcpy(buffer + offset, players, sizeof(players));
    offset += sizeof(players);
    memcpy(buffer + offset, &timestamp, sizeof(timestamp));
}

void StatePacket::deserialize(const char* buffer)
{
    size_t offset = 0;
    memcpy(&playerCount, buffer + offset, sizeof(playerCount));
    offset += sizeof(playerCount);
    memcpy(players, buffer + offset, sizeof(players));
    offset += sizeof(players);
    memcpy(&timestamp, buffer + offset, sizeof(timestamp));
}

void Packet::serialize(char* buffer) const
{
    size_t offset = 0;
    memcpy(buffer + offset, &type, sizeof(type));
    offset += sizeof(type);

    if (type == PacketType::MESSAGE)
    {
        memcpy(buffer + offset, message, sizeof(message));
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
    else if (type == PacketType::INPUT_ACK)
    {
		inputAck.serialize(buffer + offset);
    }
}

void Packet::deserialize(const char* buffer)
{
    size_t offset = 0;
    memcpy(&type, buffer + offset, sizeof(type));
    offset += sizeof(type);

    if (type == PacketType::MESSAGE)
    {
        memcpy(message, buffer + offset, sizeof(message));
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
    else if (type == PacketType::INPUT_ACK)
    {
		inputAck.deserialize(buffer + offset);
    }
}
