//
// Created by MarioJ on 25/06/15.
//

#ifndef NODE_CLIENT_H
#define NODE_CLIENT_H

#include "header.cpp"

#define NEW_NODE "-new"
#define NODES "-nodes"

#define MENU_PRINT_NODES 1
#define MENU_ADD 2
#define MENU_DELETE 3

using namespace std;

class Client {
    
private:
    string host_tracker, port_tracker;
    int socket;
    list<string> nodes;
    
public:
    Client(string, string);
    void conect(string, string);
    void send(string);
    void end_message();
    string receive();
    void close();
    void *get_in_addr(struct sockaddr*);
    void new_node(string, string);
    void add(string);
    string make_connit(string, string);
    string get_nodes();
    int menu();
};


#endif
