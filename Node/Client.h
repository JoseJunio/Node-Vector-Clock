//
// Created by MarioJ on 25/06/15.
//

#ifndef NODE_CLIENT_H
#define NODE_CLIENT_H

#include "header.cpp"
#include <random>

#define NODES "-nodes"

using namespace std;

typedef struct {
    string tracker_host, node_host;
    int tracker_port, node_port;
    
} Addresses;

class Client {
    
private:
    Addresses* addresses;
    int socket;
    list<string>* nodes;
    map<string, int>* vc;
    
public:
    Client(Addresses*, list<string>*, map<string, int>*);
    void conect(string, string);
    void send(string);
    void end_message();
    string receive();
    void close();
    void *get_in_addr(struct sockaddr*);
    void new_node();
    void sync_vector_clock();
    void broadcast_new_node();
    void send_message(string);
    string make_connit(string, string);
    string get_nodes();
    string format_vector_clock();
    int menu();
    string* random_addr();
    void update_vc(string);
    string* format_addr(string);
};


#endif
