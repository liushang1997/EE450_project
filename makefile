.PHONY: serverA serverB

all: 
	g++ -std=c++11 serverA.cpp -o serverA
	g++ -std=c++11 serverB.cpp -o serverB
	g++ -std=c++11 servermain.cpp -o servermain
	g++ -std=c++11 client.cpp -o client 

serverA: 
	./serverA

serverB: 
	./serverB

mainserver: 
	./servermain
