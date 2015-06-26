//
// Created by MarioJ on 25/06/15.
//

#include "Server.h"

void Server::start(string host, string port) {
    
    pthread_t thread_handler_request;
    int sockfd = 0, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }
    
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, 20) == -1) {
        perror("listen");
        exit(1);
    }
    
    while (1) {  // main accept() loop
        
        sin_size = sizeof their_addr;
        
        cout << "> Waiting connection" << endl;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        
        Helper helper;
        helper.server = this;
        helper.socket = new_fd;
        helper.addr = s;
        
        pthread_create(&thread_handler_request, NULL, &Server::handler_request, &helper);
        
    }
    
}

void *Server::handler_request(void *o) {
    
    Helper* helper = (Helper*) o;
    
    Server* server = helper->server;
    int socket = helper->socket;
    
    // get data from client and process it
    server->process(server->receive(socket));
    
    // close socket client, work done !
    close(helper->socket);
    
    return NULL;
}

void Server::send(int socket, string message) {
    
    if (::send(socket, message.c_str(), message.length(), 0) == -1) {
        cout << "error to send " << message << endl << endl;
    }
}

void Server::end_message(int socket) {
    send(socket, NET_EOM);
}

string Server::receive(int socket) {
    
    string message("");
    long numbytes;
    char buffer[MAX_DATA_SIZE];
    
    while ((numbytes = recv(socket, buffer, MAX_DATA_SIZE - 1, 0)) > 0) {
        
        message.append(buffer, numbytes);
        
        if (message.find(NET_EOM) != string::npos)
            break;
        
    }
    
    return message.substr(0, message.length() - strlen(NET_EOM));
    
}

void* Server::get_in_addr(struct sockaddr *sa) {
    
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int Server::get_in_port(struct sockaddr *sa) {
    
    if (sa->sa_family == AF_INET) {
        return ntohs((((struct sockaddr_in*)sa)->sin_port));
    }
    
    return ntohs((((struct sockaddr_in6*)sa)->sin6_port));
}

void Server::process(string data) {
    
    if (data.empty()) {
        return;
    }
    
    cout << "> [client]: " << data;
    
    if (data.find(ADD) != string::npos) {
        
    } else if (data.find(DELETE) != string::npos) {
        
    }
    
    StringTokenizer tokenizer(data, CONNIT_SEPARATOR);
    
    if (tokenizer.countTokens() != 2)
        return;
    
    string key = tokenizer.nextToken(), value = tokenizer.nextToken();
    
    cout << "> [" << key << " : " << value << "]" << endl;
    
}
