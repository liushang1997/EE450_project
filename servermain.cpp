#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
// #include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string> 
#include <string.h>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <regex>

#define HOST "127.0.0.1"
#define MAX_SIZE 1024
#define UDP_BACKA_PORT 30762
#define UDP_BACKB_PORT 31762
#define UDP_MAIN_PORT 32762
#define TCP_MAIN_PORT 33762

using namespace std;

unordered_map<string, int> countryServerMap;

string UDPToBackServer(int backPort, string request) {
    int sockfd; 
    char reqBuf[request.length() + 1] = {0}, resBuf[MAX_SIZE] = {0};
    struct sockaddr_in servaddr; 
  
    strcpy(reqBuf, request.c_str()); 

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 

    servaddr.sin_family = AF_INET; 
    inet_aton(HOST, &servaddr.sin_addr);
    servaddr.sin_port = htons(backPort); 
      
    unsigned int len = sizeof(servaddr); 
    sendto(sockfd, (const char *)reqBuf, strlen(reqBuf), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));        
    recvfrom(sockfd, (char *)resBuf, MAX_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
    close(sockfd);

    string response = resBuf;
    return response;
}

void TCPFromClient() {
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char reqBuf[MAX_SIZE] = {0}, resBuf[MAX_SIZE] = {0}; 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }  

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    } 

    address.sin_family = AF_INET; 
    inet_aton(HOST, &address.sin_addr);
    address.sin_port = htons(TCP_MAIN_PORT); 
       
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(server_fd, 2) < 0) { 
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    int cid = 0;

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        }

        if (!fork()) {
            close(server_fd);
            valread = read(new_socket, reqBuf, MAX_SIZE); 

            string response, request = reqBuf;
            int idx = request.find(" ");
            string country = request.substr(0, idx);
            string id = request.substr(idx+1);
            cout << "The Main server has received the request on User" << id << " in " << country
                 << " from client " << ++cid << " using TCP over port " << TCP_MAIN_PORT << '\n';

            // cout << "------------" << countryServerMap.count(country) << "-------------" << '\n';
            if (!countryServerMap.count(country)) {
                cout <<  country << " does not show up in server A&B\n";
                cout << "The Main Server has sent \"" << country << ": Not found\" to client" << cid
                     << " using TCP over port " << TCP_MAIN_PORT << "\n";
                response = "-2";    // country not found
            } else if (countryServerMap[country] == 0) {
                cout << country << " shows up in server A\n";
                cout << "The Main Server has sent request from User " << id << " to "
                     << "server A using UDP over port " << UDP_MAIN_PORT << '\n';
                response = UDPToBackServer(UDP_BACKA_PORT, request);
                if (response == "-1") {
                    cout << "The Main server has received \"User ID: Not found\" from "
                         << "server A\n";
                    cout << "The Main Server has sent error to client using TCP over "
                         << TCP_MAIN_PORT << '\n';
                } else {
                    cout << "The Main server has received searching result of User"
                         << id << " from server A\n";
                    cout << "The Main Server has sent searching result to client "
                         << "using TCP over port " << TCP_MAIN_PORT << "\n";
                }
            } else {
                cout << country << " shows up in server B\n";
                cout << "The Main Server has sent request from User " << id << " to "
                     << "server B using UDP over port " << UDP_MAIN_PORT << '\n';
                response = UDPToBackServer(UDP_BACKB_PORT, request);            
                if (response == "-1") {
                    cout << "The Main server has received \"User ID: Not found\" from "
                         << "server B\n";
                    cout << "The Main Server has sent error to client using TCP over "
                         << TCP_MAIN_PORT << '\n';
                } else {
                    cout << "The Main server has received searching result of User"
                         << id << " from server B\n";
                    cout << "The Main Server has sent searching result to client "
                         << "using TCP over port " << TCP_MAIN_PORT << "\n";
                }
            }

            strcpy(resBuf, response.c_str());
            if (send(new_socket, resBuf, strlen(resBuf), 0) < 0) {
                perror("send");
            }
            close(new_socket);
            exit(0);
        }
        close(new_socket);
    }
}

void loadData() {
    string request = "askmap";
    string responseA = UDPToBackServer(UDP_BACKA_PORT, request);
    cout << "The Main server has received the country list from server A using UDP over port " << UDP_MAIN_PORT << '\n';
    string responseB = UDPToBackServer(UDP_BACKB_PORT, request);
    cout << "The Main server has received the country list from server B using UDP over port " << UDP_MAIN_PORT << '\n';
    regex regex{R"(\s)"};
    sregex_token_iterator itA{responseA.begin(), responseA.end(), regex, -1};
    vector<string> countriesA{itA, {}};            
    for (size_t i = 0; i < countriesA.size(); i++) {
        countryServerMap[countriesA[i]] = 0;
    }
    sregex_token_iterator itB{responseB.begin(), responseB.end(), regex, -1};
    vector<string> countriesB{itB, {}};            
    for (size_t i = 0; i < countriesB.size(); i++) {
        countryServerMap[countriesB[i]] = 1;
    }
    cout << "Server A            | Server B            \n";
    for (size_t i = 0; i < max(countriesA.size(), countriesB.size()); i++) {
        if (i >= countriesA.size()) {
            string sa(20, ' ');
            string sb(20 - countriesB[i].length(), ' ');
            cout << sa << "| " << countriesB[i] << sb << '\n';
        } else if (i >= countriesB.size()) {
            string sa(20 - countriesA[i].length(), ' ');
            string sb(20, ' ');
            cout << countriesA[i] << sa  << "| " << sb << '\n';
        } else {
            string sa(20 - countriesA[i].length(), ' ');
            string sb(20 - countriesB[i].length(), ' ');                        
            cout << countriesA[i] << sa << "| " << countriesB[i] << sb << '\n';
        }
    }
}

int main() { 
    cout << "The Main server is up and running.\n";
    loadData();
    TCPFromClient(); 
} 