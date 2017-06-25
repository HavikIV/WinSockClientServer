/*	A basic WinSock server application
*	as demonstrated in a tutorial by Microsoft
*/

#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "libs.h"
#include "AddressBook.h"

#define DEFAULT_PORT "1986"
#define DEFAULT_BUFLEN 512

using namespace std;									//	So I won't have to specify the namespace when I create a vector for the different client connections

struct Connection
{
	SOCKET sock;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen;

	Connection(SOCKET s) : sock(s), recvbuflen(0) {}		//	Constructor with an initialization list for the struct, sets sock to s and recvbuflen to DEFAUL_BUFLEN
};

//typedef vector<Connection> connectionList;
//typedef vector<Contact> contactList;
//
//connectionList connections;
//contactList myContacts;

//	Create an Address Book
AddressBook ab;

//void setupFDSETS(fd_set& readFDs, fd_set& writeFDs, fd_set& exceptFDs, SOCKET listeningSocket);
//
//bool readData(Contact& conn);
//
//bool writeData(Contact& conn);

string getNumberFromBuffer(Contact& sender);

void acceptConnections(SOCKET listeningSocket);

//unsigned __stdcall ClientSession(void *data);


int __cdecl main(void)	
{

	//	Create a WSADARA object
	WSADATA wsaData;

	//	Create a SOCKET object that will be used to listen to clients that try to connect to the server
	SOCKET ListenSocket = INVALID_SOCKET;
	//	Create a temporary SOCKET object that will be used to connect to a client
	//SOCKET ClientSocket = INVALID_SOCKET;
	
	//	Declaring an addrinfo object, hints used for providing hints for what Kind of IP address to get, result will be populated by getaddrinfo()
	struct addrinfo *result = NULL, hints;

	int iResult;										//	Used to store the returned values from the function calls to check they completed successfully
	//u_long iMode = 1;									//	Will use this to change the socket to be nonblocking

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
	iResult = getaddrinfo("192.168.1.4", DEFAULT_PORT, &hints, &result);
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
	while (1)
	{
		acceptConnections(ListenSocket);
	}
	//while (ClientSocket = accept(ListenSocket, NULL, NULL))
	//{
	//	//	Check for errors
	//	if (ClientSocket == INVALID_SOCKET)
	//	{
	//		printf("accept failed with error: %d\n", WSAGetLastError());
	//		closesocket(ListenSocket);
	//		WSACleanup();								//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
	//		return 1;
	//	}

	//	////	Change the I/O mode of the socket to be nonblocking
	//	//iResult = ioctlsocket(ClientSocket, FIONBIO, &iMode);
	//	////	Check for errors
	//	//if (iResult != NO_ERROR)
	//	//{
	//	//	printf("ioctlsocket failed with error: %d", iResult);
	//	//}

	//	//	Create a new thread to handle the client
	//	//unsigned threadID;
	//	//HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)ClientSocket, 0, &threadID);
	//}

	//	Cleanup
	closesocket(ListenSocket);
	//closesocket(ClientSocket);
	WSACleanup();										//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)

	return 0;
}

////	Setup the three FD sets that are used with select() with the sockets in the connection list.
////	Also add one for the listening socket, if one was passed in to the function
//void setupFDSETS(fd_set& readFDs, fd_set& writeFDs, fd_set& exceptFDs, SOCKET listeningSocket = INVALID_SOCKET)
//{
//	//	FD_ZERO(*set) is a macro that initializes the set to the null set
//	FD_ZERO(&readFDs);
//	FD_ZERO(&writeFDs);
//	FD_ZERO(&exceptFDs);
//
//	//	If a listening socket was passed to the function, add it to the read and except FD sets
//	if (listeningSocket != INVALID_SOCKET)
//	{
//		//	FD_SET(s, *set) is a macro that is used to add socket s to the set provided
//		FD_SET(listeningSocket, &readFDs);
//		FD_SET(listeningSocket, &exceptFDs);
//	}
//
//	//	Add the client connections to the three sets
//
//	//connectionList::iterator it = connections.begin();	//	Create an iterator to be used to iterate through the list(vector) of client connections
//	contactList::iterator it = myContacts.begin();
//
//	while (it != myContacts.end())
//	{
//		//	Check to see if there is space in the read buffer, if there is, then pay attention for incoming data
//		if (it->getMsgSize() < defaultBufferLen)
//		{
//			FD_SET(it->socket, &readFDs);
//		}
//
//		//	Check to see if there's any data in the buffer that still needs to be sent out.
//		if (it->getMsgSize() > 0)
//		{
//			FD_SET(it->socket, &writeFDs);
//		}
//
//		//	if the client isn't ready to read incoming data or sent data then add it to the except set
//		FD_SET(it->socket, &exceptFDs);
//
//		//	Increment the iterator
//		it++;
//	}
//}
//
////	This function will read the data that is coming in the client connection.
////	It will return FALSE on failure to read or when the client closes its half
////	of the connection.
//bool readData(Contact& conn)
//{
//	//	Read the incoming data and store the number of bytes received in the variable bytes
//	//	recvbuf + recvbuflen = where to start adding the incoming data in the buffer
//	//	DEFAULT_BUFLEN - recvbuflen = how much space is left in the buffer
//	//	Remember: recv(socket s, char* buf, int len, int flags); len is the size, in bytes, of the buffer,
//	//	and buf is the pointer to the buffer
//	int bytes = recv(conn.socket, &conn.getBuffer() + conn.getMsgSize(), DEFAULT_BUFLEN - conn.getMsgSize(), 0);
//
//	//	Check for errors
//	if (bytes == 0)
//	{
//		//	Connection was closed
//		return false;
//	}
//	else if (bytes == SOCKET_ERROR)
//	{
//		//	Something went wrong
//		printf("recv failed: %d\n", bytes);
//		int err;
//		int errLen = sizeof(err);
//		//	getsockopt retrieves a socket option
//		//	getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
//		//	level is asking what level the option is defined at. SOL_SOCKET is one such level
//		//	optname is the socket option for which the value is to be retrieved. Must be defined in the provided socket level
//		//	optval is a pointer to a buffer in which the value for the requested option is to be returned
//		//	optlen is a pointer to the size, in bytes, of the optval buffer.
//		getsockopt(conn.socket, SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
//		return (err == WSAEWOULDBLOCK);
//	}
//
//	//	Successfully read the incoming data so need to record how bytes were read
//	printf("recv() was successful and recieved %d bytes\n", bytes);
//	conn.setMsgSize(bytes);// += bytes;
//
//	return true;
//}
//
////	The client connection provided is ready to send out data.
////	Returns FALSE on failure.
//bool writeData(Contact& conn)
//{
//	//	Send everything in the connection's buffer
//	int bytes;
//
//	if (myContacts.size() > 1)
//	{
//		//	Instead of echoing the message, send it to all other clients
//		//connectionList::iterator it = connections.begin();
//		contactList::iterator it = myContacts.begin();
//
//		while (it != myContacts.end())
//		{
//			if (it->socket != conn.socket)
//			{
//				bytes = send(it->socket, &conn.getBuffer(), conn.getMsgSize(), 0);
//
//				//	Check for errors
//				if (bytes == SOCKET_ERROR)
//				{
//					int err;
//					int errLen = sizeof(err);
//					getsockopt(conn.socket, SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
//					return (err == WSAEWOULDBLOCK);
//				}
//
//				if (bytes == conn.getMsgSize())
//				{
//					//	Everything got sent out on the connection, so reset the recvbuflen to 0
//					//	to prevent that the buffer was cleared.
//					conn.setMsgSize(0);// = 0;
//				}
//				else
//				{
//					//	Only some of the data in the buffer was sent out, so remove the part that was sent out from the buffer
//					//	for the next time that send() is called on this connection
//					conn.setMsgSize(-bytes);// -= bytes;
//					//	memmove(void *dest, const void *src, size_t count)
//					//	dest -> destination object; recvbuf
//					//	src -> source object; recvbuf + bytes
//					//	count -> number of bytes to copy to the destination
//					//	moves the part that wasn't sent out to the front of the buffer for the next time send()is called
//					memmove(&conn.getBuffer(), &conn.getBuffer() + bytes, conn.getMsgSize());
//				}
//			}
//
//			it++;
//		}
//	}
//	else
//	{
//		bytes = send(conn.socket, &conn.getBuffer(), conn.getMsgSize(), 0);	//	Echo the message back
//		if (bytes == conn.getMsgSize())
//		{
//			//	Everything got sent out on the connection, so reset the recvbuflen to 0
//			//	to prevent that the buffer was cleared.
//			conn.setMsgSize(0);// = 0;
//		}
//		else
//		{
//			//	Only some of the data in the buffer was sent out, so remove the part that was sent out from the buffer
//			//	for the next time that send() is called on this connection
//			conn.setMsgSize(-bytes);// -= bytes;
//			//	memmove(void *dest, const void *src, size_t count)
//			//	dest -> destination object; recvbuf
//			//	src -> source object; recvbuf + bytes
//			//	count -> number of bytes to copy to the destination
//			//	moves the part that wasn't sent out to the front of the buffer for the next time send()is called
//			memmove(&conn.getBuffer(), &conn.getBuffer() + bytes, conn.getMsgSize());
//		}
//	}
//
//	return true;
//}

//	This function contains an infinite loop that will accept connections, but if
//	something breaks, the function will return control back to the caller.
void acceptConnections(SOCKET listeningSocket)
{
	sockaddr_in sinRemote;								//	Used to determine where connection is coming from
	int addrSize = sizeof(sinRemote);

	while (1)
	{
		Contact con;
		fd_set readFDs, writeFDs, exceptFDs;
		//setupFDSETS(readFDs, writeFDs, exceptFDs, listeningSocket);
		ab.setupFDSets(listeningSocket);

		//	References to the FD_SETS in the address book
		readFDs = ab.getReadFDSet();
		writeFDs = ab.getWriteFDSet();
		exceptFDs = ab.getExceptFDSet();

		if (select(0, &readFDs, &writeFDs, &exceptFDs, 0) > 0)
		{
			//	Something happened on one of the sockets

			//	Check to see if it was the listening socket, if so accept the connection
			//	FD_ISSET(s, *set) is a macro that is used to check if the socket s is in the set provided, nonzero if in set, else zero if not in set
			if (FD_ISSET(listeningSocket, &readFDs))
			{
				SOCKET sock = accept(listeningSocket, (sockaddr*)&sinRemote, &addrSize);

				//	Check for errors
				if (sock == INVALID_SOCKET)
				{
					printf("Failed to accept client connection.\n");
					return;
				}
				else
				{
					//Connection temp(sock);
					//recv(sock, temp.recvbuf, temp.recvbuflen, 0);	// the number of the client here
					//printf("%s\n", temp.recvbuf);
					//	accept() was successful, inform the user where the connection was from
					printf("Accepted connection from %s:%d, socket %d.\n", inet_ntoa(sinRemote.sin_addr), ntohs(sinRemote.sin_port), sock);

					con.setSocket(sock);

					//	Add the connection to the vector of connections
					//connections.push_back(sock);
					ab.addContact(con);
					//myContacts.push_back(con);

					//	If there are more than 64 sockets, there might be some unexpected problems
					/*	What are the “64 sockets” limitations?
					 *	There are two 64-socket limitations:
					 *	The Windows event mechanism (e.g. WaitForMultipleObjects()) can only wait on 64 event objects at a time. Winsock 2
					 *	provides the WSAEventSelect() function which lets you use Windows’ event mechanism to wait for events on sockets.
					 *	Because it uses Windows’ event mechanism, you can only wait for events on 64 sockets at a time. If you want to wait
					 *	on more than 64 Winsock event objects at a time, you need to use multiple threads, each waiting on no more than 64 of the sockets.
					 *	The select() function is also limited in certain situations to waiting on 64 sockets at a time. The FD_SETSIZE constant
					 *	defined in the Winsock header determines the size of the fd_set structures you pass to select(). The default value is 64,
					 *	but if you define this constant to a different value before including the Winsock header, it accepts that value instead:
					 *	#define FD_SETSIZE 1024
					 *	#include <wsock32.h>
					 *	The problem is that modern network stacks are complex, with many parts coming from various sources, including third parties
					 *	via things like Layered Service Providers. When you change this constant, you’re depending on all these components to play by
					 *	the new rules. They’re supposed to, but not all do. The typical symptom is that they ignore sockets beyond the 64th in larger
					 *	fd_set structures. You can get around this limitation with threads, just as in the event object case.	
					 */
					//	Warn the user of the there being more the 64 sockets, counting the listening socket
					if ((ab.getSizeOfAddressBook() + 1) > 64)
					{
						printf("WARNING: More than 63 client connections accepted, so there might be some unexcepted problems arising.\n");
					}

					//	Make the client socket to be non-blocking just in case to avoid any problems
					u_long iMode = 1;
					ioctlsocket(sock, FIONBIO, &iMode);
				}
			}
			else if (FD_ISSET(listeningSocket, &exceptFDs))
			{
				int err;
				int errLen = sizeof(err);
				getsockopt(listeningSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
				printf("Exception on listening socket: %d\n", err);
				return;
			}
			else
			{
				//	Check if it was one of the client connections
				//connectionList::iterator it = connections.begin();
				contactList::iterator it = ab.begin();
				//contactList::iterator it = myContacts.begin();

				while (it != ab.end())
				{
					bool ok = true;
					const char* errorType = 0;

					//	See if this socket's flag is set in any of the FD sets.
					if (FD_ISSET(it->getSocket(), &exceptFDs))
					{
						ok = false;
						errorType = "General socket error";
						//	FD_CLR(s, *set) is a macro that is used to remove socket s from the set provided
						FD_CLR(it->getSocket(), &exceptFDs);
					}
					else
					{
						if (FD_ISSET(it->getSocket(), &readFDs))
						{
							//ok = readData(*it);
							ok = ab.recvMsg(*it);
							errorType = "Read error";
							FD_CLR(it->getSocket(), &readFDs);
						}
						if (FD_ISSET(it->getSocket(), &writeFDs))
						{
							//ok = writeData(*it);
							string number = getNumberFromBuffer(*it);
							ok = ab.sendMsg(*it, *ab.getContact(number));	// echo it back to itself
							errorType = "Write error";
							FD_CLR(it->getSocket(), &writeFDs);
						}
					}

					if (!ok)
					{
						//	Something went wrong on the client socket or the client closed half of it's connection.
						//	Shutdown the connection and remove it from the list of connections
						int err;
						int errLen = sizeof(err);
						getsockopt(it->getSocket(), SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
						if (err != NO_ERROR)
						{
							printf("%ld\n", WSAGetLastError());
						}
						else
						{
							printf("Closed connection from %s:%d, socket %d.\n", inet_ntoa(sinRemote.sin_addr), ntohs(sinRemote.sin_port), it->getSocket());
						}
						closesocket(it->getSocket());
						/*connections.erase(it);
						it = connections.begin();*/
						ab.removeContact(it->getNumber());
						it = ab.begin();
					}
					else
					{
						//	Move onto the next connection in the list
						it++;
					}
				}
			}			
		}
		else
		{
			printf("select() failed: %ld\n", WSAGetLastError());
			return;
		}
	}
}

string getNumberFromBuffer(Contact& sender)
{
	//	Get the number from the buffer
	char str[26];
	memset(str, 0, 14);
	strncpy_s(str, &(const char)sender.getBuffer(), 12);

	//	Remove the number from the buffer
	memmove(&sender.getBuffer(), &sender.getBuffer() + 13, sender.getMsgSize() - 13);
	sender.setMsgSize(sender.getMsgSize() - 13);

	//	Return the number
	return string(str);
}

////	Function that is passed to a different thread for each accepted client connections
////	This function will handle reading and sending data associated to the client socket
////	that is passed to the function when the child thread is created.
//unsigned __stdcall ClientSession(void *data)
//{
//	SOCKET clientSocket = (SOCKET)data;
//
//	int iResult, iSendResult;
//	char recvbuf[DEFAULT_BUFLEN];
//	int recvbuflen = DEFAULT_BUFLEN;
//
//	//process the client
//	//	Receive until the peer shuts down the connection
//	do
//	{
//		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
//		//	Check for errors
//		if (iResult > 0)
//		{
//			printf("Bytes received: %d\n", iResult);
//
//			//	Echo the buffer back to the sender
//			iSendResult = send(clientSocket, recvbuf, iResult, 0);
//			//	Check for errors
//			if (iSendResult == SOCKET_ERROR)
//			{
//				printf("send failed: %d\n", WSAGetLastError());
//				closesocket(clientSocket);
//				WSACleanup();							//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
//				return 1;
//			}
//			printf("Bytes sent; %d\n", iSendResult);
//		}
//		else if (iResult == 0)
//			printf("Connection closin...\n");
//		else
//		{
//			printf("recv failed: %d\n", WSAGetLastError());
//			closesocket(clientSocket);
//			WSACleanup();								//	Terminates the use of the WinSock 2 DLL (Ws2_32.sll)
//			return 1;
//		}
//	} while (iResult > 0);
//}