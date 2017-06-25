#include "Contact.h"

Contact::Contact()
{
	number = "000-000-0000";
	socket = INVALID_SOCKET;
	//	memset -> sets the first num bytes of the block of memory pointed by ptr to the specified value (interpreted as an unsigned char)
	//	memset(void* ptr, int value, size_t num)
	//	Set the buffer to all NULL character \0
	memset(buffer, '\0', sizeof(char)* defaultBufferLen);
	sizeOfMsgInBuffer = 0;
}

Contact::Contact(string no, SOCKET sock)
{
	setNumber(no);
	setSocket(sock);
	memset(buffer, '\0', sizeof(char)* defaultBufferLen);
	sizeOfMsgInBuffer = 0;
}

Contact::~Contact()
{
	//	Close the socket
	//closesocket(socket);
}

//	Update or add the phone number of the contact
void Contact::setNumber(string no)
{
	number = no;
}

//	Update or add the Socket for the contact
void Contact::setSocket(SOCKET sock)
{
	socket = sock;
}

//	Update the variable sizeOfMsgInBuffer by the size of
//	the message that was added to the buffer
void Contact::setMsgSize(int incSize)
{
	sizeOfMsgInBuffer = incSize;
}

//	Clear the message from the buffer by reseting the buffer to all '\0''s
void Contact::clearBuffer()
{
	//	Clear the whole msg from the buffer
	memset(buffer, '\0', sizeof(char)* sizeOfMsgInBuffer);
}

//	Grab the phone number of the contact
string Contact::getNumber()
{
	return number;
}

//	Grab the socket for the contact
//	This function will return a pointer to the contact's socket
//	so that it can be used as needed
SOCKET& Contact::getSocket()
{
	return socket;
}

//	Grab the message in the buffer
//	This function will return a pointer to the contact's buffer
//	that can then be used to send the message to it's destination
char& Contact::getBuffer()
{
	return *buffer;
}

//	Grab the size, in bytes, of the message
int Contact::getMsgSize()
{
	return sizeOfMsgInBuffer;
}