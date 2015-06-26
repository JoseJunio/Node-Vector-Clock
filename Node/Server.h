//
// Created by MarioJ on 25/06/15.
//

#ifndef NODE_SERVER_H
#define NODE_SERVER_H

#include "header.cpp"

using namespace std;

class Server {

public:
    void start(string, string);
    static void* handler_request(void*);
    void send(int, string);
    void end_message(int);
    string receive(int);
    void *get_in_addr(struct sockaddr*);
    int get_in_port(struct sockaddr*);
    void process(string);
    void add(string);
};

typedef struct {
    Server* server;
    int socket;
    string addr;
} Helper;

#endif
