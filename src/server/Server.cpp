#include "Server.h"
#include <iostream>

// Constructor initializes the server with a specified port and sets the socket to an invalid state.
Server::Server(uint16_t port) : port(port), socket(INVALID_SOCKET), tickCount(0) {}

Server::~Server()
{
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
	}
}

bool Server::initialize()
{
	if (!network.initialize())
	{
		return false;
	}

	socket = network.createUdpSocket(true);
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	if (!network.bindSocket(socket, "0.0.0.0", port))
	{
		closesocket(socket);
		socket = INVALID_SOCKET;
		return false;
	}

	std::cout << "Server initialized on port " << port << "..." << std::endl;
	return true;
}

// Handles incoming packets based on their type.
void Server::handlePacket(const Packet& packet, const sockaddr_in& clientAddr)
{
	if (packet.type == PacketType::CONNECT)
	{
		uint32_t playerId;
		if (gameState.addPlayer(clientAddr, playerId))
		{
			Packet ack;
			ack.type = PacketType::CONNECT_ACK	;
			ack.playerId = playerId;
			char buffer[MAX_PACKET_SIZE];
			ack.serialize(buffer);
			network.sendPacket(socket, clientAddr, buffer, MAX_PACKET_SIZE);
			std::cout << "Player " << playerId << " connected." << std::endl;
		}
	}
	else if (packet.type == PacketType::INPUT)
	{
		gameState.queueInput(packet.input);
		// Acknowledge the input received
		Packet ack;
		ack.type = PacketType::INPUT_ACK;
		ack.inputAck.playerId = packet.input.playerId;
		ack.inputAck.sequence = packet.input.sequence;
		char buffer[MAX_PACKET_SIZE];
		ack.serialize(buffer);
		network.sendPacket(socket, clientAddr, buffer, MAX_PACKET_SIZE);
	}
}

// Processes incoming packets, updates the game state, and broadcasts the current state to all clients.
void Server::tick()
{
	char buffer[MAX_PACKET_SIZE];
	sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);

	// Receive packets from clients and send them for handling
	while (network.receivePacket(socket, buffer, MAX_PACKET_SIZE, clientAddr, clientAddrSize))
	{
		Packet packet;
		packet.deserialize(buffer);
		handlePacket(packet, clientAddr);
	}

	// Process buffered inputs
	gameState.processInput();

	// Update game state
	gameState.update();

	// Broadcast state
	const StatePacket& state = gameState.getState();
	Packet statePacket;
	statePacket.type = PacketType::STATE;
	statePacket.state = state;
	char sendBuffer[MAX_PACKET_SIZE];
	statePacket.serialize(sendBuffer);

	for (const auto& [key, addr] : gameState.getClients())
	{
		network.sendPacket(socket, addr, sendBuffer, MAX_PACKET_SIZE);
	}

	tickCount++;
}

void Server::run()
{
	const int TICK_RATE = 20;
	const int TICK_TIME_MS = 1000 / TICK_RATE;

	while (true)
	{
		auto start = GetTickCount64();
		// Run the server tick
		tick();


		// Maintain tick rate
		auto elapsed = GetTickCount64() - start;
		if (elapsed < TICK_TIME_MS)
		{
			Sleep(TICK_TIME_MS - elapsed);
		}
	}
}

int main()
{
	Server server(8888);
	if (!server.initialize())
	{
		return 1;
	}
	server.run();
	return 0;
}