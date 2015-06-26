#ifndef HEADER_CLIENT_H
#define HEADER_CLIENT_H

#include <iostream>
#include <list>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "StringTokenizer.h"

#define MAX_DATA_SIZE 1024
#define NET_EOM "\r\n\r\n"

#define CONNIT_SEPARATOR "\n"
#define LIST_SEPARATOR ","

#define ADD "-add"
#define DELETE "-delete"

typedef struct {
    std::string host, port;
} Address;

#endif