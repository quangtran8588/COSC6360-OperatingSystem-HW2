/************************************************************************************
* COSC 6360 - Advanced Operating System						    *
* Homework Assignment 2 - Simulate a Client-Server TCP Socket Connection in Linux   *
* Client creates TCP Socket connection to Server                                    *
* Client sends a Semaphore request message to Server                                *
* and waits for Server to reply back                                                *
************************************************************************************/
#pragma once
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
using namespace std;

#define MAXHOSTNAME 1024
#define MAXBUFFERSIZE 1024

int write_data(int sockConn, char *buff, int n);
int read_data(int sockConn, char *buff, int n);
int connect(string message);
int sem_create();
int sem_P(int sem_id);
int sem_V(int sem_id);
int sem_destroy(int sem_id);

// Main function is provided by Professor for testing and grading Assignment
// This function is one of a provided testing main function
// Abitrary functions will be used by Professor/TA for grading this assignment
// Main functions will invoke multiple functions, created by Student, to perform a specific operation
int main() {
	int mutex1, mutex2; //semaphores
    int pid; // process ID
    if ((mutex1 = sem_create()) < 0) {
         printf("Cannot create semaphore mutex1.\n");
         _exit(1);
    }  //if
    if ((mutex2 = sem_create()) < 0) {
         _exit(2);
         printf("Cannot create semaphore mutex2.\n");
    }  // if
    // basic checks
    sem_P(mutex2);
    printf("Completed a P() operation on mutex2.\n");
    sem_V(mutex2);
    sem_V(mutex2);
    printf("Completed two V() operations on mutex2.\n");
    
    if ((pid = fork()) == 0) {
        // child process
        printf("Child process requests mutex1.\n");
        sem_P(mutex1);
        printf("Child process got mutex1.\n");
        sleep(10);
        printf("Child process is ready to release mutex1.\n");
        sem_V(mutex1);
        printf("Child process released mutex1.\n");
        _exit(0);
    } // if
    sleep(3);
    printf("Parent process requests mutex1.\n");
    sem_P(mutex1);
    printf("Parent process got mutex1.\n");
    sleep(8);
    printf("Parent process is ready to release mutex1.\n");
    sem_V(mutex1);
    printf("Parent process released mutex1.\n");

    // Ending
    sem_destroy(mutex1);
    if (sem_P(mutex1) >= 0) {
        printf("OOPS! Semaphore mutex1 was not properly destroyed!\n");
    } // if
    sem_destroy(mutex2);
    if (sem_P(mutex2) >= 0) {
        printf("OOPS! Semaphore mutex2 was not properly destroyed!\n");
    } //if
    if (sem_P(mutex1) >= 0) {
        printf("OOPS! Server accepted a call to a non-existent semaphore!\n");
    }
	return 0;
}

// function to push data into socket and send to Server
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

// function to read data which was sent from Server into TCP socket
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
// Function creates a string message of request "Create Semaphore"
int sem_create() {
	string message = "Create_Sem";
	return connect(message);
}
// Function creates a string message of request "P_Operation Semaphore" along with Semaphore ID
int sem_P(int sem_id) {
	string message = "P_Operation " + to_string(sem_id);
	return connect(message);
}
// Function creates a string message of request "V_Operation Semaphore" along with Semaphore ID
int sem_V(int sem_id) {
	string message = "V_Operation " + to_string(sem_id);
	return connect(message);
}
// Function creates a string message of request "Destroy Semaphore" along with Semaphore ID to be destroyed
int sem_destroy(int sem_id) {
	string message = "Destroy_Sem " + to_string(sem_id);
	return connect(message);
}
// Function creates a client-server TCP connection
// and sends a request message to server
int connect(string message) {
	// Create and initialize variables
	int successful;
	int sock, connection;
	int portnum;
	char hostname[MAXHOSTNAME + 1];
	char recvBuff[MAXBUFFERSIZE] = "";
	char sendBuff[MAXBUFFERSIZE] = "Hello! This is client connection.";
	struct sockaddr_in address;
	struct hostent *host;
	successful = 0;
	portnum = sock = connection = 0;

	memset(&address, 0, sizeof(address));
	gethostname(hostname, MAXHOSTNAME);

	if ((host = gethostbyname(hostname)) == NULL) {
		cout << "Cannot get host name." << endl;
		return -1;
	}
	portnum = 5000;
	// Create TCP socket to connect to Server
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "Cannot create socket!" << endl;
		exit(1);
	}
	address.sin_addr = (*(struct in_addr*) host->h_addr);
	address.sin_family = AF_INET;
	address.sin_port = htons(portnum);
	// Client creates connection to a server
	// If failed, return -1 and exit program
	if ((connection = connect(sock,(struct sockaddr*)&address, sizeof(address))) < 0)
	{
		cout << "Cannot create connection to Server. Please check your port number!" << endl;
		return -1;
	}
	// Client connected to a server, and starts sending a hello message to server
	// and waits for server to reply back a welcome message
	// Then, client sends a request message to server and waits for server to reply back a client's request message
	else
	{
		if ((successful = write_data(sock, sendBuff, sizeof(sendBuff))) < 0) {
			cout << "Fail to say Hello to server!" << endl;
			close(connection);
			close(sock);
			exit(1);
		}
		if ((successful = read_data(sock, recvBuff, sizeof(recvBuff))) < 0) {
			cout << "Fail to receive message! Server might be down!" << endl;
			close(connection);
			close(sock);
			exit(1);
		}
		strcpy(sendBuff, message.c_str());
		if ((successful = write_data(sock, sendBuff, sizeof(sendBuff))) < 0) {
			cout << "Fail to send request to server!" << endl;
			close(connection);
			close(sock);
			exit(1);
		}
		if ((successful = read_data(sock, recvBuff, sizeof(recvBuff))) < 0) {
			cout << "Fail to receive message! Server might be down!" << endl;
			close(connection);
			close(sock);
			exit(1);
		}
		return stoi(string(recvBuff));
	}
	return 0;
}
