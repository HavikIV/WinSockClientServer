#pragma once

#include "libs.h"
#include "Contact.h"

using namespace std;

typedef vector<Contact> contactList;

class AddressBook
{
public:
	//	Default Constructor
	AddressBook();

	//	Destructor
	~AddressBook();

	//	Add the given contact to the list of contacts
	void addContact(Contact con);

	//	Remove the given contact from the list
	void removeContact(string no);

	//	Update the socket of the contact
	//	The contact reconnected to the server so need to update
	//	the socket for the contact
	void updateContactSocket(string no, SOCKET newSocket);

	//	Check to see if the given contact is within the list of contacts
	bool isContactInAddressBook(string no);

	//	Check how many contacts are in the address book
	int getSizeOfAddressBook();

	//	Grab the readFDSet
	FD_SET& getReadFDSet();

	//	Grab the writeFDSet
	FD_SET& getWriteFDSet();

	//	Grab the exceptFDSet
	FD_SET& getExceptFDSet();

	contactList& getContactList();

	//	Setup the fd_sets
	void setupFDSets(SOCKET listenSocket);

	//	Receive a message from the given contact
	//	This function will return true if the message was received
	//	without any problems, otherwise it will return false when
	//	something goes wrong. If the error is WSAEWOULDBLOCK isn't considered a failure.
	bool recvMsg(Contact& con);

	//	Send a message to the given contact
	//	This function will return true if the message was sent
	//	without any problems, otherwise it will return false when
	//	something goes wrong. If the error is WSAEWOULDBLOCK isn't considered a failure.
	bool sendMsg(Contact& sender, Contact& receiver);

	//	Find the contact
	contactList::iterator getContact(string no);

	//	Returns an iterator that points to the beginning of the contactList
	contactList::iterator begin();

	//	Returns an iterator that points to end of the contactList
	contactList::iterator end();

private:
	contactList contacts;						//	List of Contacts
	fd_set readFDs, writeFDs, exceptFDs;		//	Fd_sets for reading, writing, and exception
};