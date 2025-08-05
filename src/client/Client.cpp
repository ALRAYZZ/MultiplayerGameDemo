#include "Client.h"
#include <conio.h>
#include <iostream>

Client::Client(const std::string& serverIp, uint16_t serverPort) : playerId(0), connected(false)
{
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
}

Client::~Client()
{
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
	}
}

bool Client::initialize()
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

	Packet connectPacket;
	connectPacket.type = PacketType::CONNECT;
	connectPacket.playerId = 0; // Player ID will be assigned by the server
	char buffer[MAX_PACKET_SIZE];
	connectPacket.serialize(buffer);
	if (!network.sendPacket(socket, serverAddr, buffer, MAX_PACKET_SIZE))
	{
		closesocket(socket);
		socket = INVALID_SOCKET;
		return false;
	}
	return true;
}

void Client::handleInput()
{
	InputPacket input;
	input.playerId = playerId;
	input.moveUp = false;
	input.moveDown = false;
	input.moveLeft = false;
	input.moveRight = false;
	input.timestamp = static_cast<uint32_t>(time(nullptr));

	if (_kbhit())
	{
		char ch = _getch();
		if (ch == 'w') input.moveUp = true;
		else if (ch == 's') input.moveDown = true;
		else if (ch == 'a') input.moveLeft = true;
		else if (ch == 'd') input.moveRight = true;
	}

	if (input.moveUp || input.moveDown || input.moveLeft || input.moveRight)
	{
		// If input is made, send it to the server
		Packet inputPacket;
		inputPacket.type = PacketType::INPUT;
		inputPacket.input = input;
		char buffer[MAX_PACKET_SIZE];
		inputPacket.serialize(buffer);
		network.sendPacket(socket, serverAddr, buffer, MAX_PACKET_SIZE);
	}
}

void Client::handlePacket(const Packet& packet)
{
	if (packet.type == PacketType::CONNECT_ACK && !connected)
	{
		playerId = packet.playerId;
		connected = true;
		std::cout << "Connected as Player " << playerId << std::endl;
	}
	else if (packet.type == PacketType::STATE && connected)
	{
		renderer.render(packet.state);
	}
}

void Client::run()
{
	while (true)
	{
		handleInput();

		char buffer[MAX_PACKET_SIZE];
		sockaddr_in fromAddr;
		int fromAddrSize = sizeof(fromAddr);

		while (network.receivePacket(socket, buffer, MAX_PACKET_SIZE, fromAddr, fromAddrSize))
		{
			Packet packet;
			packet.deserialize(buffer);
			handlePacket(packet);
		}

		Sleep(50);
	}
}

int main()
{
	Client client("127.0.0.1", 8888);
	if (!client.initialize())
	{
		return 1;
	}
	client.run();
	return 0;
}