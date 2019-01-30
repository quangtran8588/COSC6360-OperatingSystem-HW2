/************************************************************************************
* COSC 6360 - Advanced Operating System						    *
* Homework Assignment 2 - Simulate a Client-Server TCP Socket Connection in Linux   *
* Server creates TCP Socket connections                                             *
* Server waits for Client to send Semaphore request message                         *
* and replies back an appropriate message                                           *
************************************************************************************/
#pragma once
#include <iostream>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <algorithm>
#include <queue>
using namespace std;

#define MAXHOSTNAME 1024
#define MAXBUFFERSIZE 1024
#define MAXROW 128
#define MAXCOL 2

int write_data(int sockConn, char *buff, int n);
int read_data(int sockConn, char *buff, int n);
int establish();
int parse_request(string *request,int *sem_id);
void split(string input, string *left, string *right, char delimiter);
int parse_request(string *request, int *sem_id);
void initialize_list(int(&list)[MAXROW][MAXCOL]);
int sem_create(int (&list)[MAXROW][MAXCOL]);
int sem_destroy(int (&list)[MAXROW][MAXCOL], int sem_id);
int sem_V(int (&list)[MAXROW][MAXCOL], int sem_id);
int sem_P(int (&list)[MAXROW][MAXCOL], int sem_id);
bool isExisted(int list[MAXROW][MAXCOL], int sem_id);
bool isLocked(int list[MAXROW][MAXCOL], int sem_id);
void free_up_list(int(&list)[MAXROW][MAXCOL], int &available_slot);

struct queueList {
	int sem_id;
	int sock_conn;
	queueList(int id, int sock) : sem_id(id), sock_conn(sock) {}
};

int main() {
	int s;
	s = 0;
	if ((s = establish() < 0))
		return -1;
	return 0;
}

// function to push data into socket and send to Client
int write_data(int sockConn, char *buff, int n)
{
	int bCount, bWrite;
	bCount = bWrite = 0;
	while (bCount < n)
	{
		if ((bWrite = send(sockConn, &buff[bWrite], n - bCount, 0)) > 0) {
			bCount += bWrite;
		}
		else if (bWrite < 0)
			return -1;
		else
			return 0;
	}
	return bCount;
}

// function to retrieve data from socket
int read_data(int sockConn, char *buff, int n)
{
	int bCount, bRead;
	bCount = bRead = 0;
	while (bCount < n)
	{
		if ((bRead = recv(sockConn, &buff[bRead], n - bCount, 0)) > 0) {
			bCount += bRead;
		}
		else if (bRead < 0)
			return -1;
		else
			return 0;
	}
	return bCount;
}

//	Function will split string into two parts - request, and semaphore id
void split(string input, string *left, string *right, char delimiter) {
	stringstream ss(input);
	getline(ss, *left, delimiter);
	getline(ss, *right);
}

//	Function will identify the client's request
//	1. Create Semaphore request, 2. Destroy Semaphore request, 3. V Semaphore request, 4. P Semaphore request
//	After identifying specific request type, split client's message request into two parts (only for 2,3,and 4) - request and semaphore id
int parse_request(string *request, int *sem_id) {
	string temp = "";
	size_t found = (*request).find("Create_Sem");
	if (found != string::npos) {
		return 1;
	}
	found = (*request).find("Destroy_Sem");
	if (found != string::npos) {
		split(*request, request, &temp, ' ');
		*sem_id = stoi(temp);
		return 2;
	}
	found = (*request).find("V_Operation");
	if (found != string::npos) {
		split(*request, request, &temp, ' ');
		*sem_id = stoi(temp);
		return 3;
	}
	found = (*request).find("P_Operation");
	if (found != string::npos) {
		split(*request, request, &temp, ' ');
		*sem_id = stoi(temp);
		return 4;
	}
	return -1;
}

//	Function initialize the 2D array contains Semaphore's ID and Semaphore's value
void initialize_list(int (&list)[MAXROW][MAXCOL]) {
	for (int i = 0; i < MAXROW; i++) {
		for (int j = 0; j < MAXCOL; j++) {
			if (j == 0)
				list[i][j] = i;
			else
				list[i][j] = -1;
		}
	}
}

//	Function will create new Semaphore ID by checking if value is equal to -1
//	If yes, then initialize value to 1
//	If list is full, return -1 to indicate "list is full"
int sem_create(int (&list)[MAXROW][MAXCOL]) {
	for (int i = 0; i < MAXROW; i++) {
		if (list[i][1] == -1) {
			list[i][1] = 1;
			return list[i][0];
		}
	}
	return -1;
}

//	Function will delete Semaphore ID when client request
//	Checking if Semaphore ID is existed by checking Semaphore value
//	If value -1 which mean Semaphore does not exist, then return -1
//	Otherwise, set value to -2 to indicate Semaphore's deletion
int sem_destroy(int (&list)[MAXROW][MAXCOL], int sem_id) {
	if (list[sem_id][1] < 0)
		return -1;
	else 
		list[sem_id][1] = -2;
	return 0;
}

//	Function will release Semaphore 
//	If semaphore's value is not non-negative, set it back to 1
//	If semaphore's value is -1 or -2 which means Semaphore's ID does not exist (not created, or already destroyed)
//	then return -1
int sem_V(int (&list)[MAXROW][MAXCOL], int sem_id) {
	if (list[sem_id][1] < 0)
		return -1;
	else {
		list[sem_id][1] = 1;
	}
	return 0;
}

int sem_P(int (&list)[MAXROW][MAXCOL], int sem_id) {
	if (list[sem_id][1] < 0)
		return -1;
	list[sem_id][1] = 0;
	return 0;
}

bool isLocked (int list[MAXROW][MAXCOL], int sem_id){	
	if (list[sem_id][1] == 0)
		return true;
	return false;
}

void free_up_list(int (&list)[MAXROW][MAXCOL], int &available_slot) {
	for (int i = 0; i < MAXROW; i++) {
		if (list[i][1] == -2) {
			list[i][1] = -1;
			available_slot += 1;
		}
	}
}

//	Function will create Server socket and wait for client's connection
int establish()
{
	// Create and Initialize variables to be used
	long successful;
	int connection, sock, c;
	int portnum;
	int available_semaphore_id = MAXROW;
	successful = 0;
	c = portnum = connection = sock = 0;
	struct sockaddr_in address;
	struct hostent *hp;
	char sendBuff[MAXBUFFERSIZE] = "";
	char recvBuff[MAXBUFFERSIZE] = "";
	char myname[MAXHOSTNAME + 1];
	queue<queueList> queue_client_conn[MAXROW];
	

	int list[MAXROW][MAXCOL];
	initialize_list(list);
	
	// Get hostname and Port number of Socket Server	
	memset(&address, 0, sizeof(address));
	gethostname(myname, MAXHOSTNAME);
	portnum = 5000;
	
	c = sizeof(struct sockaddr_in);
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_family = AF_INET;
	address.sin_port = htons(portnum);
	// Create Socket Server
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "Fail to create a socket" << endl;
		return -1;
	}

	// Bind socket and listen Client connection
	// Maximum of 100 clients can connect at the same time
	if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
		cout << "Fail to bind a socket" << endl;
		close(sock);
		return -1;
	}
	cout << "Waiting for client connection....." << endl;
	listen(sock, 100);
	
	for (;;)
	{
		if ((connection = accept(sock, (struct sockaddr*)&address, (socklen_t*)&c)) < 0) {
			cout << "Error to connect" << endl;
			return -1;
		}
		else
		{
			// Receive hello message from Client when it connects to Server
			cout << "\nConnection was found!" << endl;
			if ((successful = read_data(connection, recvBuff, sizeof(recvBuff))) < 0) {
				cout << "Fail to receive response from Client" << endl;
				close(connection);
				close(sock);
				exit(1);
			}
			cout << "Message from Client: " << recvBuff << endl;
			// Reply wellcome message to Client
			char wellcome[1024] = "Wellcome. You have been connected to the Server";
			if ((successful = write_data(connection, wellcome, sizeof(wellcome))) < 0) {
				cout << "Fail to Wellcome message" << endl;
				close(connection);
				close(sock);
				exit(1);
			}
			if ((successful = read_data(connection, recvBuff, sizeof(recvBuff))) < 0) {
				cout << "Fail to receive request from Client" << endl;
				close(connection);
				close(sock);
				exit(1);
			}
			cout << "Request from Client: " << recvBuff << endl;

			//	Server receives Client's request. Call parse_request to identify request
			//	and split message request into appropriate fields
			string request = string(recvBuff);
			int id = 0;
			int ret = parse_request(&request, &id);

			//	Client requests Semaphore Create
			//	If list is full, Server reply -1. Otherwise, reply Semaphore ID to Client
			//	and initialize Semaphore's value to 1
			if (ret == 1) {
				if (available_semaphore_id == 0)
					free_up_list(list, available_semaphore_id);
				int sem_id = sem_create(list);
				strcpy(sendBuff, to_string(sem_id).c_str());
				if ((successful = write_data(connection, sendBuff, sizeof(sendBuff))) < 0) {
					cout << "Fail to reply the client's request" << endl;
					close(connection);
					close(sock);
					exit(1);
				}
				if (sem_id >= 0)
					available_semaphore_id -= 1;
			}
			
			//	Client requests Semaphore Destroy
			//	Check whether Semaphore's ID is existed, then reply 0 to Client and set Semaphore's value to -2 (indicate deletion)
			//	Otherwise, reply -1
			else if (ret == 2) {
				int errcode = sem_destroy(list, id);
				strcpy(sendBuff, to_string(errcode).c_str());
				if ((successful = write_data(connection, sendBuff, sizeof(sendBuff))) < 0) {
					cout << "Fail to reply the client's request" << endl;
					close(connection);
					close(sock);
					exit(1);
				}
			}

			//	Client requests V_Operation Semaphore
			//	If Semaphore's ID is existed, Server replies 0 to Client and set Semaphore's value back to 1
			//	Otherwise, Server replies -1 
			//	After setting Semaphore's value back to 1, Server check if any Client is still waiting for P_Operation in queue list
			//	If Yes, then proceed P_Operation of next Client
			else if (ret == 3) {
				int errcode = sem_V(list, id);
				strcpy(sendBuff, to_string(errcode).c_str());
				if ((successful = write_data(connection, sendBuff, sizeof(sendBuff))) < 0) {
					cout << "Fail to reply the client's request" << endl;
					close(connection);
					close(sock);
					exit(1);
				}
				if (errcode != -1 && !queue_client_conn[id].empty()) {
					queueList item = queue_client_conn[id].front();
					queue_client_conn[id].pop();
					errcode = sem_P(list, item.sem_id);
					strcpy(sendBuff, to_string(errcode).c_str());
					if ((successful = write_data(item.sock_conn, sendBuff, sizeof(sendBuff))) < 0) {
						cout << "Fail to reply the client's request" << endl;
						close(item.sock_conn);
						close(sock);
						exit(1);
					}
				}
			}

			//	Client requests P_Operation Semaphore
			//	Check whether any Semaphore is locked, then push Client into queue
			//	Otherwise, checking whether Semaphore's ID is valid, then reply 0 to Client, and set Semaphore's value to 0
			//	If Semaphore's ID is not existed, reply -1
			else if (ret == 4) {
				if (isLocked(list, id)) {
					queue_client_conn[id].push(queueList(id, connection));
				}
				else {
					int errcode = sem_P(list, id);
					strcpy(sendBuff, to_string(errcode).c_str());
					if ((successful = write_data(connection, sendBuff, sizeof(sendBuff))) < 0) {
						cout << "Fail to reply the client's request" << endl;
						close(connection);
						close(sock);
						exit(1);
					}
				}
			}
		}
	}
	return sock;
}
