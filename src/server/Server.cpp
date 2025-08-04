#include <iostream>
#include <map>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <GameProtocol.h>

struct Client
{
	sockaddr_in addr;
	uint32_t lastPacketTime; // So we can track when the last packet was received
};

int main()
{
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed." << WSAGetLastError() << std::endl;
		return 1;
	}

	// Create UDP Socket
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	// Set socket to non-blocking
	u_long mode = 1; // 1 to enable non-blocking
	if (ioctlsocket(serverSocket, FIONBIO, &mode) != 0)
	{
		std::cerr << "Failed to set non-blocking: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// Binding socket to server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	inet_pton(AF_INET, "0.0.0.0", &serverAddr.sin_addr);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server is running and listening on port 8888..." << std::endl;

	// Game state
	std::vector<Player> players;
	std::map<std::string, Client> clients; // Clients address as string key
	uint32_t nextPlayerId = 1; // Start player IDs from 1. Iterate over this to assign unique IDs to players

	// Main loop
	const int TICK_RATE = 20;
	const int TICK_TIME_MS = 1000 / TICK_RATE; // 20 ticks per second

	while (true)
	{
		auto start = GetTickCount64();

		// Receive packets
		char buffer[MAX_PACKET_SIZE];
		sockaddr_in clientAddr;
		int clientAddrSize = sizeof(clientAddr);

		while (true)
		{
			int bytesReceived = recvfrom(serverSocket, buffer, MAX_PACKET_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
			if (bytesReceived <= 0)
			{
				if (bytesReceived == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
				{
					std::cerr << "Receive error: " << WSAGetLastError() << std::endl;
				}
				break;
			}

			// Convert client address to string for map key
			char clientIp[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
			std::string clientKey = std::string(clientIp) + ":" + std::to_string(ntohs(clientAddr.sin_port));

			Packet packet;
			packet.deserialize(buffer);

			if (packet.type == PacketType::CONNECT)
			{
				if (players.size() < MAX_PLAYERS)
				{
					// Add new player
					Player newPlayer;
					newPlayer.id = nextPlayerId++;
					newPlayer.x = 5.0f;
					newPlayer.y = 5.0f;
					newPlayer.health = 100;
					players.push_back(newPlayer);

					// Adding client to map
					clients[clientKey] = { clientAddr, static_cast<uint32_t>(time(nullptr)) }; // Client struct with address and current time

					// Send connection acknowledgment
					Packet ack;
					ack.type = PacketType::CONNECT_ACK;
					ack.playerId = newPlayer.id;
					char sendBuffer[MAX_PACKET_SIZE];
					ack.serialize(sendBuffer);
					sendto(serverSocket, sendBuffer, MAX_PACKET_SIZE, 0, (sockaddr*)&clientAddr, clientAddrSize);

					std::cout << "Player " << newPlayer.id << " connected from " << clientKey << std::endl;
				}
			}
			else if (packet.type == PacketType::INPUT)
			{
				clients[clientKey].lastPacketTime = static_cast<uint32_t>(time(nullptr)); // Update last packet time
				for (auto& player : players)
				{
					if (player.id == packet.input.playerId)
					{
						float speed = 0.1f; // Movement speed
						if (packet.input.moveUp) player.y -= speed;	
						if (packet.input.moveDown) player.y += speed;
						if (packet.input.moveLeft) player.x -= speed;
						if (packet.input.moveRight) player.x += speed;
						break;
					}
				}
			}
		}

		// Update game state (simple boundary check)
		for (auto& player : players)
		{
			if (player.x < 0) player.x = 0;
			if (player.x > 10) player.x = 10;
			if (player.y < 0) player.y = 0;
			if (player.y > 10) player.y = 10;
		}

		// Broadcast game state
		Packet statePacket;
		statePacket.type = PacketType::STATE;
		statePacket.state.playerCount = static_cast<uint32_t>(players.size());
		for (size_t i = 0; i < players.size(); i++)
		{
			statePacket.state.players[i] = players[i];
		}
		statePacket.state.timestamp = static_cast<uint32_t>(time(nullptr));

		char sendBuffer[MAX_PACKET_SIZE];
		statePacket.serialize(sendBuffer);

		for (const auto& [key, client] : clients)
		{
			sendto(serverSocket, sendBuffer, MAX_PACKET_SIZE, 0, (sockaddr*)&client.addr, sizeof(client.addr));
		}

		// Maintain tick rate
		auto elapsed = GetTickCount64() - start;
		if (elapsed < TICK_TIME_MS)
		{
			Sleep(TICK_TIME_MS - elapsed);
		}
	}

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}