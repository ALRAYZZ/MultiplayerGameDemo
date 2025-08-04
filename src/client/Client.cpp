#include <iostream>
#include <winSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <GameProtocol.h>
#include <conio.h>


void renderGameState(const StatePacket& state)
{
	const int GRID_SIZE = 11; // 10x10 grid + 1 for the border
	char grid[GRID_SIZE][GRID_SIZE] = { {'.'} };

	// Initialize grid
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			grid[y][x] = '.';
		}
	}

	// Place players on the grid
	for (uint32_t i = 0; i < state.playerCount; i++)
	{
		int x = static_cast<int>(state.players[i].x);
		int y = static_cast<int>(state.players[i].y);
		if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
		{
			grid[y][x] = 'P'; // Mark player position
		}
	}

	// Render grid
	system("cls"); // Clear console (Windows specific)
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			std::cout << grid[y][x] << ' ';
		}
		std::cout << std::endl;
	}
}

int main()
{
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "WSAStartup failed: " << WSAGetLastError() << std::endl;
		return 1;
	}

	// Create UDP socket
	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	// Set socket to non-blocking mode
	u_long mode = 1;
	if (ioctlsocket(clientSocket, FIONBIO, &mode) != 0)
	{
		std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	// Server address info
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Localhost address

	// Connect to the server
	Packet connectPacket;
	connectPacket.type = PacketType::CONNECT;
	connectPacket.playerId = 0; // Player ID will be assigned by the server
	char sendBuffer[MAX_PACKET_SIZE];
	connectPacket.serialize(sendBuffer);
	sendto(clientSocket, sendBuffer, sizeof(connectPacket), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));


	uint32_t playerId = 0;
	bool connected = false;

	// Main loop
	while (true)
	{
		// Handle input
		InputPacket input;
		input.playerId = playerId;
		input.moveUp = _kbhit() && _getch() == 'w'; 
		input.moveDown = _kbhit() && _getch() == 's';
		input.moveLeft = _kbhit() && _getch() == 'a';
		input.moveRight = _kbhit() && _getch() == 'd';
		input.timestamp = static_cast<uint32_t>(time(nullptr));

		if (input.moveUp || input.moveDown || input.moveLeft || input.moveRight)
		{
			// If input is made, send it to the server
			Packet inputPacket;
			inputPacket.type = PacketType::INPUT;
			inputPacket.input = input;
			inputPacket.serialize(sendBuffer);
			sendto(clientSocket, sendBuffer, sizeof(inputPacket), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		}

		// Receive packets
		char recvBuffer[MAX_PACKET_SIZE];
		sockaddr_in fromAddr;
		int fromAddrSize = sizeof(fromAddr);

		while (true)
		{
			int bytesReceived = recvfrom(clientSocket, recvBuffer, MAX_PACKET_SIZE, 0, (sockaddr*)&fromAddr, &fromAddrSize);
			if (bytesReceived <= 0)
			{
				if (bytesReceived == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
				{
					std::cerr << "Receive error: " << WSAGetLastError() << std::endl;
				}
				break;
			}

			Packet packet;
			packet.deserialize(recvBuffer);
			if (packet.type == PacketType::CONNECT_ACK && !connected)
			{
				playerId = packet.playerId;
				connected = true;
				std::cout << "Connected as Player " << playerId << std::endl;
			}
			else if (packet.type == PacketType::STATE && connected)
			{
				renderGameState(packet.state);
			}
		}

		Sleep(50); // 20 Hz client update rate
	}

	closesocket(clientSocket);
	WSACleanup();
	return 0;
}