#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cout << "WSAStartup failed: " << result << std::endl;
		return 1;
	}
	std::cout << "Winsock initialized. Server running..." << std::endl;
	WSACleanup();
	return 0;
}