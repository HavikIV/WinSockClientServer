#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN				// Marco to prevent the windows.h header file from loading the Winsock.h header file that's now out of date
#endif WIN32_LEAN_AND_MEAN

#include <Windows.h>					// Generally not needed for WinSock applications as WinSock2.h internally includes core elements of Windows.h, but if needed it must be included before WinSock2.h
#include <WinSock2.h>					// Has the necessary functions, structures, and definitions needed to work with sockets
#include <WS2tcpip.h>					// Definitions from the WinSock 2 Protocol-Specific Annex Document for TCP/IP; newer functions and structures used to retrieve IP addresses
#include <IPHlpApi.h>					// Header file that has the IP Helper APIs, if this header file is needed then it must be included after WinSock2.h
#include <stdlib.h>
#include <stdio.h>						// Standard input and output; used for printf() function

#pragma comment(lib, "Ws2_32.lib")		// lets the linker know that the Ws2_32.lib is needed; WinSock applications need Ws2_32.lib
#pragma comment(lib, "Mswsock.lib")		// Lets the linker know that the Mswsock.lib is needed
#pragma comment(lib, "AdvApi32.lib")	// Lets the linker know that the AdvApi.lib is needed

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1986"

int __cdecl main(int argc, char** argv)	// C type main function, will allow me to pass in the name of the server the client should connect to in the command line
{
	WSADATA wsaData;				// WSADATA object; WSADATA structure contains information about the Windows Sockets implementation
									/* typedef struct WSAData {
											WORD				wVersion;								// Version of Windows Sockets specification that Ws2_32.dll expects the caller to use. High-order byte specifies the minor version number and the low-order byte specifies the major version number
											WORD				wHighVersion;							// Highest version of the Windows Sockets specification that Ws2_32.dll can support. High-order byte specifies the minor version number and the low-order byte specifies the major version number
											char				szDescription[WSADEsCRIPTION_LEN+1];	// NULL-terminated ASCII string into which Ws2_32.dll copies a description of the Windows Sockets implementation
											char				szSystemStatus{WSASYS_STATUS_LEN+1];	// NULL-terminated ASCII string into which Ws2_32.dll copies relevant status or configuration information
											unsigned short		iMaxSockets;							// The maximum number of sockets that may be opened. Ignored for Windows Sockets version 2 or later
											unsigned short		iMaxUdpDg;								// The maximum datagram message size. Ignored for Windows Sockets version 2 or later
											char FAR			*lpVendorInfo;							// A pointer to vendor-specific information. Ignored for Windows Sockets version 2 or later
									   } WSADATA, *LPWSADATA;
									*/

	// Create a SOCKET object
	SOCKET ConnectSocket = INVALID_SOCKET;			// Sets the SOCKET to ~0 (Two's Compliment of -1)
	// Declare an addrinfo object	
	struct addrinfo *result = NULL, *ptr = NULL, hints;			/* typedef struct addrinfo {
																int				ai_flags;		// Options used in the getaddrinfo function
																int				ai_family;		// Address family
																int				ai_socktype;	// Socket type
																int				ai_protocol;	// Protocol type
																size_t			ai_addrlen;		// The length, in bytes, of the buffer pointed to by the ai_addr
																char			*ai_canonname;	// The canonical name for the host
																struct sockaddr	*ai_addr;		// Pointer to a sockaddr structure
																struct addrinfo	*ai_next;		// Pointer to the next structure in a linked list
																} ADDRINFOA, *PADDINFOA;
																*/
	char* sendbuf = "This is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;					// Used to check if any of the function calls failed

	// Validate the parameters
	if (argc != 2)
	{
		printf("Usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize WinSock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);	// WSAStartup function initiates use of the WinSock DLL by a process
	// Check for errors
	if (iResult != 0)
	{
		printf("WSAStartup failed; %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));				// ZeroMemory -> Fills a block of memory with zeros; Used to initialize all members of the hint structure to 0;
	hints.ai_family = AF_UNSPEC;					// The address family is unspecified; change to AF_INET for IPv4 or AF_INET6 for IPv6
	hints.ai_socktype = SOCK_STREAM;				// Provides sequeced, reliable, two-way, connection-based byte streams with an OOB data transmission mechanism. Uses TCP
	hints.ai_protocol = IPPROTO_TCP;				// Uses Transmission Control Protocol (TCP)

	// getaddrinfo function provides protocol-independent translation from an ANSI host name to an address
	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	// Check for errors
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();								// Terminates the use of the Winsock 2 DLL (Ws2_32.dll)
		return 1;
	}

	// Attempt to connect to an address returned by the call to getaddrinfo
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to the server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		// Check for errors
		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("Error at socket(): %ld\n", WSAGetLastError());
			WSACleanup();								// Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
			return 1;
		}

		// Connect to the server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		// Check for errors
		if (iResult == SOCKET_ERROR)
		{
			// Close the socket
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);							// Frees address information that the getaddrinfo function dynamically allocates in addrinfo structures

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to the server!\n");
		WSACleanup();								// Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	// Send an initial buffer to the server
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	// Check for errors
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		// Close the socket
		closesocket(ConnectSocket);
		WSACleanup();								// Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// Shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	// Check for errors
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		// Close the socket
		closesocket(ConnectSocket);
		WSACleanup();								// Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
		return 1;
	}

	// Receiving data until the server closes the connection
	do
	{
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// Cleanup
	closesocket(ConnectSocket);
	WSACleanup();									// Terminates the use of the WinSock 2 DLL (Ws2_32.sll)

	return 0;
}