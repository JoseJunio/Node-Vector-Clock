//
// Created by MarioJ on 25/06/15.
//

#ifndef NODE_SERVER_H
#define NODE_SERVER_H

#include "header.h"
#include <queue>

using namespace std;

class Server {
    
private:
    list<string>* nodes;
    map<string, int>* vc;
    list<string> messages_queue;
    
public:
    Server(list<string>*, map<string, int>*);
    void start(string);
    static void* handler_request(void*);
    void send(int, string);
    void end_message(int);
    string receive(int);
    void *get_in_addr(struct sockaddr*);
    int get_in_port(struct sockaddr*);
    void process(string, int);
    string vector_clock_to_string();
    void add_new_node(string);
    void deliver_message(string);
    string* format_addr(string);
    string* parse_vector_clock(string);
    string* format_connit_message(string);
    bool is_vector_clock_sync(string, string);
    bool is_vector_clock_lq(string);
    list<string> check_queue_messages();
    void print_messages(list<string>);
};

typedef struct {
    Server* server;
    int socket;
    string addr;
    int port;
} Helper;

#endif
