#include <iostream>
#include <cstring>

#ifdef _WIN32

#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment (lib, "ws2_32.lib")

#else
#include <arpa/inet.h>
typedef int SOCKET;
#endif // _WIN32

#define QUESTIONS_COUNT 4
#define ANSWERS_COUNT 6

unsigned char buf[1024], *name;
SOCKET in, DNSout;

#ifdef _WIN32
bool CtrlHandler(DWORD fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT) {
		closesocket(in);
		closesocket(DNSout);
		WSACleanup();
	}
	return false;
}
#endif

void printDomainName() {
	while (true) {
		if (name[0] >= 0xC0)
			name = buf + htons((short&)name[0] ^ 0xC0);
		for (char j = name++[0]; j; --j)
			std::cout << name++[0];
		if (!name[0])
			break;
		std::cout << '.';
	}
}

int main() {
#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

	WSADATA data;
	if (WSAStartup(0x0202, &data)) {
		std::cout << "Can't start Winsock!\n";
		return 1;
	}

#endif
	in = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in DNServerHint;
#ifdef _WIN32
	DNServerHint.sin_addr.S_un.S_addr = ADDR_ANY;
#else
	DNServerHint.sin_addr.s_addr = INADDR_ANY;
#endif
	DNServerHint.sin_family = AF_INET;
	DNServerHint.sin_port = htons(53);

	if (bind(in, (sockaddr*)&DNServerHint, sizeof(DNServerHint)) < 0)
	{
		std::cout << "Can't bind socket!" << '\n';
		return 1;
	}

	sockaddr_in DNServer;
	DNServer.sin_family = AF_INET;
	DNServer.sin_port = htons(53);
	inet_pton(AF_INET, "8.8.8.8", &DNServer.sin_addr);	// Here you can change DNS ip address

	DNSout = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in client;
#ifdef _WIN32
	int clientLength;
#else
	unsigned int clientLength;
#endif
	clientLength = sizeof(client);
	while (true)
	{
		int bytesIn = recvfrom(in, (char*)buf, 1024, 0, (sockaddr*)&client, &clientLength);
		sendto(DNSout, (char*)buf, bytesIn, 0, (sockaddr*)&DNServer, clientLength);
		bytesIn = recvfrom(DNSout, (char*)buf, 1024, 0, (sockaddr*)&DNServer, &clientLength);
		short count = htons((short&)buf[QUESTIONS_COUNT]);
		unsigned char* i = &buf[12];
		while (count--) {
			while (i[0])
				if (i[0] >= 0xC0) {
					++i;
					break;
				}
				else
					i += i[0] + 1;
			i += 5;
		}
		count = htons((short&)buf[ANSWERS_COUNT]);
		//Here you can edit DNS response
		//Example (change google.com response address to 127.0.0.1, undocument lines below)
		while (count--) {
			if (i[0] & 0xC0)
				name = buf + htons((short&)i[0] ^ 0xC0);
			else
				name = i;
			printDomainName();
			std::cout << " -> ";
			if (i[0] & 0xC0)
				++i;
			else
				i = name;
			i += 9;
//			if (!strcmp((char*)buf + 12, "\006google\003com")) {
//				std::cout << "changed to ";
//				(int&)i[2] = 0x0100007F;
//			}
			if ((short&)i[-8] == 0x0100)
				std::cout << +i[2] << '.' << +i[3] << '.' << +i[4] << '.' << +i[5];
			else if ((short&)i[-8] == 0x0500) {
				name = i + 2;
				printDomainName();
			}
			else
				std::cout << "Unsupported response format";
			i += htons((short&)i[0]) + 2;
			std::cout << '\n';
		}
		sendto(in, (char*)buf, bytesIn, 0, (sockaddr*)&client, clientLength);
	}
}
