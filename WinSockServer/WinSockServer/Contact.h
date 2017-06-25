#pragma once

#include "libs.h"

#define defaultBufferLen 512

using namespace std;

class Contact
{
public:
	//	Default Constructor
	//	Initialize number to "000-000-0000", socket and buffer to NULL, and sizeOfMsgInBuffer to 0
	Contact();

	//	Another Constructor
	//	Initialize the contact's phone number and socket to the data provided
	Contact(string no, SOCKET sock);

	//	Destructor
	~Contact();

	//	Set the phone of the contact
	void setNumber(string no);

	//	Set the socket for the contact
	void setSocket(SOCKET sock);

	//	Update the variable sizeOfMsgInBuffer by the size of
	//	the message that was added to the buffer
	void setMsgSize(int incSize);

	//	Clear the buffer
	void clearBuffer();

	//	Grab the phone number of the contact
	string getNumber();

	//	Grab the socket for the contact
	//	This function will return a pointer to the contact's socket
	//	so that it can be used as needed
	SOCKET& getSocket();

	//	Grab the message in the buffer
	//	This function will return a pointer to the contact's buffer
	//	that can then be used to send the message to it's destination
	char& getBuffer();

	//	Grab the size, in bytes, of the message
	int getMsgSize();

private:
	string number;						//	The contact's phone number
	SOCKET socket;						//	Socket that connects to the client
	char buffer[defaultBufferLen];		//	Buffer for the contact
	int sizeOfMsgInBuffer;				//	Size, in bytes, of the message stored in the buffer
};