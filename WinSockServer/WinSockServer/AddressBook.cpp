#include "AddressBook.h"

//	Default Constructor
AddressBook::AddressBook()
{
	setupFDSets(INVALID_SOCKET);
}

//	Destructor
AddressBook::~AddressBook()
{
	//	Delete all of the contacts in the address book
	contacts.clear();
}

//	Add the given contact to the list of contacts
void AddressBook::addContact(Contact con)
{
	contacts.push_back(con);
}

//	Remove the given contact from the list
void AddressBook::removeContact(string no)
{
	if (isContactInAddressBook(no))
	{
		contacts.erase(getContact(no));
	}
}

//	Update the socket of the contact
//	The contact reconnected to the server so need to update
//	the socket for the contact
void AddressBook::updateContactSocket(string no, SOCKET newSocket)
{
	if (isContactInAddressBook(no))
	{
		getContact(no)->setSocket(newSocket);
	}
}

//	Check to see if the given contact is within the list of contacts
bool AddressBook::isContactInAddressBook(string no)
{
	for (contactList::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		string number = it->getNumber();
		if (number == no)
		{
			return true;
		}
	}
	return false;
}

//	Check how many contacts are in the address book
int AddressBook::getSizeOfAddressBook()
{
	return contacts.size();
}

//	Grab the readFDSet
FD_SET& AddressBook::getReadFDSet()
{
	return readFDs;
}

//	Grab the writeFDSet
FD_SET& AddressBook::getWriteFDSet()
{
	return writeFDs;
}

//	Grab the exceptFDSet
FD_SET& AddressBook::getExceptFDSet()
{
	return exceptFDs;
}

contactList& AddressBook::getContactList()
{
	return contacts;
}
//	Setup the fd_sets
void AddressBook::setupFDSets(SOCKET listeningSocket = INVALID_SOCKET)
{
	//	FD_ZERO(*set) is a macro that initializes the set to the null set
	FD_ZERO(&readFDs);
	FD_ZERO(&writeFDs);
	FD_ZERO(&exceptFDs);

	//	Add the listenSocket to read and except fd_sets
	if (listeningSocket != INVALID_SOCKET)
	{
		FD_SET(listeningSocket, &readFDs);
		FD_SET(listeningSocket, &exceptFDs);
	}

	//	Add the client connections to the three sets
	contactList::iterator it = contacts.begin();	//	Create an iterator to be used to iterate through the list of contacts

	while (it != contacts.end())
	{
		//	Check to see if there is space in the read buffer, if there is, then pay attention for incoming data
		if (it->getMsgSize() < defaultBufferLen)
		{
			FD_SET(it->getSocket(), &readFDs);
		}

		//	Check to see if there's any data in the buffer that still needs to be sent out.
		if (it->getMsgSize() > 0)
		{
			FD_SET(it->getSocket(), &writeFDs);
		}

		//	if the client isn't ready to read incoming data or sent data then add it to the except set
		FD_SET(it->getSocket(), &exceptFDs);

		//	Increment the iterator
		it++;
	}
}

//	Receive a message from the given contact
//	This function will return true if the message was received
//	without any problems, otherwise it will return false when
//	something goes wrong. If the error is WSAEWOULDBLOCK isn't considered a failure.
bool AddressBook::recvMsg(Contact& con)
{
	int bytes = recv(con.getSocket(), &con.getBuffer(), defaultBufferLen - con.getMsgSize(), 0);

	//	Check for errors
	if (bytes == 0)
	{
		//	Connection was closed
		return false;
	}
	else if (bytes == SOCKET_ERROR)
	{
		//	Something went wrong
		printf("recv failed: %d\n", bytes);
		int err;
		int errLen = sizeof(err);
		//	getsockopt retrieves a socket option
		//	getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
		//	level is asking what level the option is defined at. SOL_SOCKET is one such level
		//	optname is the socket option for which the value is to be retrieved. Must be defined in the provided socket level
		//	optval is a pointer to a buffer in which the value for the requested option is to be returned
		//	optlen is a pointer to the size, in bytes, of the optval buffer.
		getsockopt(con.getSocket(), SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
		return (err == WSAEWOULDBLOCK);
	}

	if (bytes == 13)
	{
		//	The client sent their phone number so update the contact's phone number entry
		con.setNumber(&con.getBuffer());
		return true;
	}
	//	Successfully read the incoming data so need to record how bytes were read
	printf("recv() was successful and recieved %d bytes\n", bytes);
	con.setMsgSize(bytes);

	return true;
}

//	Send a message to the given contact
//	This function will return true if the message was sent
//	without any problems, otherwise it will return false when
//	something goes wrong. If the error is WSAEWOULDBLOCK isn't considered a failure.
bool AddressBook::sendMsg(Contact& sender, Contact& receiver)
{
	//	Send everything in the connection's buffer
	int bytes = send(receiver.getSocket(), &sender.getBuffer(), sender.getMsgSize(), 0);

	//	Check for errors
	if (bytes == SOCKET_ERROR)
	{
		int err;
		int errLen = sizeof(err);
		getsockopt(receiver.getSocket(), SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
		return (err == WSAEWOULDBLOCK);
	}

	if (bytes == sender.getMsgSize())
	{
		//	Everything the buffer was sent out, so clear the buffer, and reset the sizeOfMsgInBuffer back to 0
		sender.clearBuffer();
		sender.setMsgSize(0);
	}
	else
	{
		//	Only some of the data in the buffer was sent out, so remove the part that was sent out from the buffer
		//	for the next time that send() is called on this connection
		sender.setMsgSize(sender.getMsgSize() - bytes);
		memmove(&sender.getBuffer(), &sender.getBuffer() + bytes, sender.getMsgSize());
	}

	return true;
}

//	Find the contact
contactList::iterator AddressBook::getContact(string no)
{
	//	check to see of the Contact is even in the list
	if (isContactInAddressBook(no))
	{
		for (contactList::iterator it = contacts.begin(); it != contacts.end(); it++)
		{
			if (it->getNumber() == no)
			{
				return it;
			}
		}
	}
}

//	Returns an iterator that points to the beginning of the contactList
contactList::iterator AddressBook::begin()
{
	return contacts.begin();
}

//	Returns an iterator that points to end of the contactList
contactList::iterator AddressBook::end()
{
	return contacts.end();
}
