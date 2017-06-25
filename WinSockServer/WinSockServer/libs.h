#pragma once

//	Place all of the needed libraries here
#include <WinSock2.h>									//	Has the necessary functions, structures, and definitions needed for socket programming
#include <Windows.h>
#include <WS2tcpip.h>									//	Definitions for TCP/IP from WinSock 2 Protocol-Specific Annex Document; newer functions and structures used to retrieve IP addresses
#include <stdlib.h>
#include <stdio.h>										//	Standard input/output, specifically printf()
#include <process.h>									//	For multi threading, will make a new thread for each client that connects to the server
#include <vector>										//	Will use to keep track of the multiple clients that connect to the server
#include <string>

// Gotta let the linker that the application also needs Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")