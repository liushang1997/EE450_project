#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h>
#include <stdlib.h>  
#include <string.h>
#include <iostream>
#include <string>

#define HOST "127.0.0.1"
#define MAX_SIZE 1024
#define UDP_BACKA_PORT 30762
#define UDP_BACKB_PORT 31762
#define UDP_MAIN_PORT 32762
#define TCP_MAIN_PORT 33762 

using namespace std;

void TCPToMainServer(string country, string id) {
    string res;
    int sock, valread; 
    struct sockaddr_in servaddr; 
    string request = country + " " + id;
    char reqBuf[request.length()+1] = {0}; 
    char resBuf[MAX_SIZE] = {0};

    memset(&servaddr, 0, sizeof(servaddr)); 

    strcpy(reqBuf, request.c_str()); 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("\n Socket creation error \n"); 
    } 
   
    servaddr.sin_family = AF_INET; 
    inet_aton(HOST, &servaddr.sin_addr);
    servaddr.sin_port = htons(TCP_MAIN_PORT); 
       
    if (inet_pton(AF_INET, HOST, &servaddr.sin_addr) <= 0) { 
        printf("\nInvalid address/ Address not supported \n"); 
        exit(EXIT_FAILURE); 
    } 
   
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
        printf("\nConnection Failed \n"); 
        exit(EXIT_FAILURE);
    } 

    int n = 0;
    n = send(sock, reqBuf, strlen(reqBuf), 0); 
    cout << "Client has sent User" << id << " and " << country 
         << " to Main Server using TCP\n";
    valread = read(sock, resBuf, MAX_SIZE); 

    res = resBuf;
    if (res == "-3") {
        cout << "Client has received results from Main Server: "
             << "User None" << " is possible friend of User" 
             << id << " in " << country <<'\n';        
    } else if (res == "-2") {
        cout << country << " not found\n";
    } else if (res == "-1") {
        cout << "User" << id << " not found\n";
    } else {
        cout << "Client has received results from Main Server: "
             << "User" << res << " is possible friend of User" 
             << id << " in " << country <<'\n';
    }
}

int main() { 
    cout << "Client is up and running\n";
    while (1) {
        string res, country, id;
        cout << "Enter country name: ";
        cin >> country;
        cout << "Enter user ID: ";
        cin >> id;
        TCPToMainServer(country, id);
    }
}