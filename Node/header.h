#ifndef HEADER_CLIENT_H
#define HEADER_CLIENT_H

#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sstream>
#include "StringTokenizer.h"

#define MAX_DATA_SIZE 1024
#define NET_EOM "\r\n\r\n"
#define DEBUG_NODE false

#define PRINT_SEPARATOR "\n"
#define MESSAGE_SEPARATOR_ARG "|"
#define VECTOR_CLOCK_KEY_VALUE_SEPARATOR ":"
#define LIST_SEPARATOR ","

#define NEW_NODE "-new"
#define VC "-vc"
#define MESSAGE "-message"

typedef struct {
    std::string host, name;
    int port;
} Address;

#endif