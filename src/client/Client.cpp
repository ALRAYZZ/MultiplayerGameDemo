#include "Client.h"
#include <conio.h>
#include <iostream>

Client::Client(const std::string& serverIp, uint16_t serverPort) : playerId(0), connected(false), socket(INVALID_SOCKET), inputSequence(0), lastStateTimestamp(0)
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

	// Doing a while instead of if: While captures all key presses until the buffer is empty
	while (_kbhit())
	{
		char ch = _getch();
		if (ch == 'w') input.moveUp = true;
		else if (ch == 's') input.moveDown = true;
		else if (ch == 'a') input.moveLeft = true;
		else if (ch == 'd') input.moveRight = true;
	}

	if (input.moveUp || input.moveDown || input.moveLeft || input.moveRight)
	{
		input.timestamp = static_cast<uint32_t>(time(nullptr));
		input.sequence = inputSequence++;
		std::cout << "[CLIENT] Using inputSequence = " << input.sequence << std::endl;
		std::cout << "[CLIENT] Raw inputSequence = " << inputSequence << std::endl;
		unacknowledgedInputs.push(input);
		Packet inputPacket;
		inputPacket.type = PacketType::INPUT;
		inputPacket.input = input;
		char buffer[MAX_PACKET_SIZE];
		inputPacket.serialize(buffer);
		network.sendPacket(socket, serverAddr, buffer, MAX_PACKET_SIZE);
		std::cout << "Sent input sequence " << input.sequence << std::endl;
	}
}

// Resending Input packets that have not been acknowledged by the server
void Client::resendUnacknowledgedInputs()
{
	auto tempQueue = unacknowledgedInputs; // Copy the queue to avoid modifying while iterating
	unacknowledgedInputs = std::queue<InputPacket>();
	while (!tempQueue.empty())
	{
		InputPacket input = tempQueue.front();
		tempQueue.pop();
		if (static_cast<uint32_t>(time(nullptr)) - input.timestamp < 5)
		{
			unacknowledgedInputs.push(input);
			Packet inputPacket;
			inputPacket.type = PacketType::INPUT;
			inputPacket.input = input;
			char buffer[MAX_PACKET_SIZE];
			inputPacket.serialize(buffer);
			network.sendPacket(socket, serverAddr, buffer, MAX_PACKET_SIZE);
			std::cout << "Resent input sequence " << input.sequence << std::endl;
		}
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
	else if (packet.type == PacketType::STATE && connected && packet.isNewerThan(lastStateTimestamp))
	{
		lastStateTimestamp = packet.state.timestamp;
		renderer.render(packet.state);
		std::cout << "Applied state with timestamp " << packet.state.timestamp << std::endl;
	}
	else if (packet.type == PacketType::INPUT_ACK && connected)
	{
		// Remove acknowledged inputs from the queue
		auto tempQueue = unacknowledgedInputs;
		unacknowledgedInputs = std::queue<InputPacket>();
		while (!tempQueue.empty())
		{
			InputPacket input = tempQueue.front();
			tempQueue.pop();
			if (input.sequence > packet.inputAck.sequence)
			{
				unacknowledgedInputs.push(input);
				std::cout << "[CLIENT] Got ACK for seq " << packet.inputAck.sequence << std::endl;
			}
		}
	}
}

void Client::tick()
{
	handleInput();
	resendUnacknowledgedInputs();

	char buffer[MAX_PACKET_SIZE];
	sockaddr_in fromAddr;
	int fromAddrSize = sizeof(fromAddr);

	while (network.receivePacket(socket, buffer, MAX_PACKET_SIZE, fromAddr, fromAddrSize))
	{
		Packet packet;
		packet.deserialize(buffer);
		handlePacket(packet);
	}
}

void Client::run()
{
	const int TICK_RATE = 20;
	const int TICK_TIME_MS = 1000 / TICK_RATE;

	while (true)
	{
		auto start = GetTickCount64();
		tick();

		auto elapsed = GetTickCount64() - start;
		if (elapsed < TICK_TIME_MS)
		{
			Sleep(TICK_TIME_MS - elapsed);
		}
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