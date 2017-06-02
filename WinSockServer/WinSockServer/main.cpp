/*	A basic WinSock server application
*	as demonstrated in a tutorial by Microsoft
*/

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>									//	Has the necessary functions, structures, and definitions needed for socket programming
#include <WS2tcpip.h>									//	Definitions for TCP/IP from WinSock 2 Protocol-Specific Annex Document; newer functions and structures used to retrieve IP addresses
#include <stdlib.h>
#include <stdio.h>										//	Standard input/output, specifically printf()

// Gotta let the linker that the application also needs Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "1986"
#define DEFAULT_BUFLEN 512


int __cdecl main(void)	
{

	//	Create a WSADARA object
	WSADATA wsaData;

	//	Create a SOCKET object that will be used to listen to clients that try to connect to the server
	SOCKET ListenSocket = INVALID_SOCKET;
	//	Create a temporary SOCKET object that will be used to connect to a client
	SOCKET ClientSocket = INVALID_SOCKET;

	//	Declaring an addrinfo object, hints used for providing hints for what Kind of IP address to get, result will be populated by getaddrinfo()
	struct addrinfo *result = NULL, hints;

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;										//	Used to store the returned values from the function calls to check they completed successfully
	int iSendResult;

	//	Initialize WinSock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);		//	MAKEWORD(2,2) requests WSAStartup for version 2.2 of WinSock on the system.
	//	Check for errors
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));					//	ZeroMemory -> fills a block a memory with zeros; Used to initialize all members of the hints structure to 0;
	hints.ai_family = AF_INET;							//	The address family is IPv4
	hints.ai_socktype = SOCK_STREAM;					//	SOCK_STREAM provides a sequenced, reliable, two-way, connection-based byte streams with an OOB data transmission mechanism. Uses TCP
	hints.ai_protocol = IPPROTO_TCP;					//	Specifies Transmission Control Protocol (TCP)
	hints.ai_flags = AI_PASSIVE;						//	AI_PASSIVE flag indicates that the caller intends to use the returned socket address structure in a call to the bind function.

	//	Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	//	Check for errors
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	//	Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//	Check for errors
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);							//	Frees address information that getaddrinfo() dynamically allocated into the addrinfo structure result
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	//	Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	//	Check for errors
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);							//	Frees address information that getaddrinfo() dynamically allocated into the addrinfo structure result
		closesocket(ListenSocket);
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	freeaddrinfo(result);								//	Frees address information that getaddrinfo() dynamically allocated into the addrinfo structure result

	//	Listen on a socket
	iResult = listen(ListenSocket, SOMAXCONN);			//	listen() takes in the created socket and the maximum length of a queue of pending connections to accept. SOMAXXONN is a special constant to allow a maximum reasonable number of pending connections
	//	Check for errors
	if (iResult == SOCKET_ERROR)
	{
		printf("Listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	//	Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	//	Check for errors
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	//	No longer need the server socket as the server is going to deal with client directly instead of passing it to different thread while listening for additional connection
	//	Next time, try to modify this application so the server does that instead
	closesocket(ListenSocket);

	//	Receive until the peer shuts down the connection
	do
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		//	Check for errors
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);

			//	Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			//	Check for errors
			if (iSendResult == SOCKET_ERROR)
			{
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();							//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
				return 1;
			}
			printf("Bytes sent; %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closin...\n");
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();								//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
			return 1;
		}
	} while (iResult > 0);

	//	Shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ClientSocket, SD_SEND);
	//	Check for errors
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	//	Cleanup
	closesocket(ClientSocket);
	WSACleanup();									//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)

	return 0;
}