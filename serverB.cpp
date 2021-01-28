#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string> 
#include <string.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <set> 
#include <vector>
#include <regex>
#include <sys/types.h> 
#include <arpa/inet.h> 

using namespace std;

#define HOST "127.0.0.1"
#define MAX_SIZE 1024
#define UDP_BACKA_PORT 30762
#define UDP_BACKB_PORT 31762
#define UDP_MAIN_PORT 32762
#define TCP_MAIN_PORT 33762

unordered_map<string, unordered_map<int, set<int>>> relation;

void loadData() {
    string line, country;
    ifstream input("data2.txt");
    while (getline(input, line)) {
        if (isalpha(line[0])) {
            country = line;
        } else {
            regex regex{R"([\s]+)"};
            sregex_token_iterator it{line.begin(), line.end(), regex, -1};
            vector<string> ids{it, {}};            
            for (size_t i = 1; i < ids.size(); i++) {
                relation[country][stoi(ids[0])].insert(stoi(ids[i]));
            }
        }
    }
    input.close();
}

int compute(string country, int id) {
    if (!relation[country].count(id)) {
        cout << "User" << id <<  " does not show up in " << country << '\n';
        return -1;  // id not found in the country
    }
    if (relation[country].size() == relation[country][id].size()+1) {
        return -3;  // recommend NULL, since it has connected to all countries
    }
    cout << "The server B is searching possible friends for User" << id << " ...\n";
    int res = 0, maxVal = -1;
    for (auto i : relation[country]) {
        if (i.first != id && !((i.second).count(id))) {
            set<int> intersect;
            set_intersection(i.second.begin(), i.second.end(),
                relation[country][id].begin(), relation[country][id].end(), inserter(intersect,intersect.begin()));
            if ((int)intersect.size() > maxVal || (intersect.size() == maxVal && i.first < res)) {
                maxVal = intersect.size();
                res = i.first;
            }
        }
    }
    return res;
}

void UDPFromMainServer() {
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family = AF_INET;
    inet_aton(HOST, &servaddr.sin_addr); 
    servaddr.sin_port = htons(UDP_BACKB_PORT); 
      
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    while (1) {
        // if (!fork()) {
            char reqBuf[MAX_SIZE] = {0}, resBuf[MAX_SIZE] = {0};
            unsigned int len = sizeof(cliaddr);
            recvfrom(sockfd, (char *)reqBuf, MAX_SIZE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);   
            string request = reqBuf;
            if (request == "askmap") {
                for (auto i : relation) {
                    string toAppend = i.first + " ";
                    strcat(resBuf, toAppend.c_str());
                }
                resBuf[strlen(resBuf)-1] = '\0';
                sendto(sockfd, (const char *)resBuf, strlen(resBuf), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                cout << "The server B has sent a country list to Main Server\n";
            } else {
                int idx = request.find(" ");
                string country = request.substr(0, idx);
                int id = stoi(request.substr(idx+1));
                cout << "The server B has received request for finding possible friends of User" << id << " in " << country << '\n';
                string res = to_string(compute(country, id));
                strcpy(resBuf, res.c_str());
                sendto(sockfd, (const char *)resBuf, strlen(resBuf), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                if (res == "-1") {
                    cout << "The server B has sent \"User" << id <<  " not found\" to MainServer\n";
                } else {
                    cout << "The server B has sent the result to Main Server\n";
                }
            }
        // }
    }
}

int main() { 
    loadData();
    cout << "The server B is up and running using UDP on port " << UDP_BACKB_PORT << '\n';
    UDPFromMainServer();
} 