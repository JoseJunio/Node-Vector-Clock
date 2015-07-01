//
// Created by MarioJ on 25/06/15.
//

#include "Server.h"

Server::Server(list<string>* _nodes, map<string, int>* _vc) {
    nodes = _nodes;
    vc = _vc;
}

void Server::start(string port) {
    
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
        
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        int port = get_in_port((struct sockaddr *) &their_addr);
        
        Helper helper;
        helper.server = this;
        helper.socket = new_fd;
        helper.addr = s;
        helper.port = port;
        
        pthread_create(&thread_handler_request, NULL, &Server::handler_request, &helper);
        
    }
    
}

void *Server::handler_request(void *o) {
    
    Helper* helper = (Helper*) o;
    
    Server* server = helper->server;
    int socket = helper->socket;
    
    // get data from client and process it
    server->process(server->receive(socket), socket);
    
    // close socket client, work done !
    close(helper->socket);
    
    return NULL;
}

void Server::process(string message, int socket) {
    
    if (message.empty())
        return;
    
    // print request message from client
    if (DEBUG_NODE) {
        cout << endl;
        cout << "> [new message]" << " " << message << endl;
    }
    
    if (message.find(VC) != string::npos) {
        send(socket, vector_clock_to_string().append(NET_EOM));
    } else if (message.find(NEW_NODE) != string::npos) {
        add_new_node(message.substr(strlen(NEW_NODE) + 1, message.length() - strlen(NEW_NODE) - 1));
    } else if (message.find(MESSAGE) != string::npos) {
        deliver_message(message.substr(strlen(MESSAGE) + 1, message.length()));
    }
    
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
        return (((struct sockaddr_in*)sa)->sin_port);
    }
    
    return (((struct sockaddr_in6*)sa)->sin6_port);
}

string Server::vector_clock_to_string() {
    
    if ((*vc).empty())
        return NULL;
    
    string vc_str("");
    
    for (map<string,int>::iterator it = (*vc).begin(); it != (*vc).end(); it++)
        vc_str.append((*it).first).append(VECTOR_CLOCK_KEY_VALUE_SEPARATOR).append(to_string((*it).second)).append(LIST_SEPARATOR);
    
    return vc_str.substr(0, vc_str.size() - strlen(LIST_SEPARATOR));
}

void Server::add_new_node(string message) {
    
    int index_node_arg = (int) message.find(MESSAGE_SEPARATOR_ARG);
    
    string node_str = message.substr(0, index_node_arg);
    string username = message.substr(index_node_arg + 1, message.length() - index_node_arg);
    
    bool node_exists = is_node_exists(node_str);
    
    if (!node_exists) {
        
        cout << "> " << username << " se conectou" << endl << endl;
        
        // add new node addr to list
        (*nodes).push_back(node_str);
        
    } else {
        cout << "> " << username << " se conectou novamente" << endl << endl;
    }
    
    // add new node to vector clock with default initialize
    (*vc).insert(make_pair(node_str, 0));
}

string* Server::format_connit_message(string connit) {
    
    int first_index_arg = (int)connit.find(MESSAGE_SEPARATOR_ARG);
    int second_index_arg = (int)connit.find(MESSAGE_SEPARATOR_ARG, first_index_arg + 1);
    
    if (first_index_arg != string::npos && second_index_arg != string::npos) {
        
        string* tokens = new string[3];
        
        string node_addr_sender = connit.substr(0, first_index_arg);
        string vc_list_sender = connit.substr(first_index_arg + 1, second_index_arg - first_index_arg - 1);
        string text_message = connit.substr(second_index_arg + 1);
        
        tokens[0] = node_addr_sender;
        tokens[1] = vc_list_sender;
        tokens[2] = text_message;
        
        return tokens;
    }
    
    return NULL;
}

/**
 Check if vector clock that node is correct with this node
 */
void Server::deliver_message(string connit) {
    
    string* message_tokens = format_connit_message(connit);
    
    if (message_tokens != NULL) {
        
        string node_addr_sender = message_tokens[0];
        string vc_list_sender = message_tokens[1];
        string text_message = message_tokens[2];
        
//        cout << "[node address sender]: " << node_addr_sender << "; [vc list sender]: " << vc_list_sender << endl;
        
        if (is_vector_clock_sync(vc_list_sender, node_addr_sender)) {
            
            // increment clock node sender
            (*vc)[node_addr_sender]++;
            
            // check whether queue has dont messages
            list<string> messages_poped = check_queue_messages();
            
            // print in view messages poped from
            print_messages(messages_poped);
            
            cout << "> " << text_message << endl;
            
        } else {
            messages_queue.push_back(connit);
            
            if (DEBUG_NODE)
                cout << "wait messages to sync vector clock [message pushed queue]" << endl;
        }
        
    } else
        cout << "Incorrect message format" << endl;
    
    
    cout.flush();
}

string* Server::format_addr(string addr) {
    
    string* addr_tokens = new string[2];
    StringTokenizer tokenizer(addr, " ");
    
    addr_tokens[0] = tokenizer.nextToken();
    addr_tokens[1] = tokenizer.nextToken();
    
    return addr_tokens;
}

string* Server::parse_vector_clock(string vc) {
    
    string* tokens = new string[2];
    StringTokenizer tokenizer(vc, VECTOR_CLOCK_KEY_VALUE_SEPARATOR);
    
    tokens[0] = tokenizer.nextToken();
    tokens[1] = tokenizer.nextToken();
    
    return tokens;
}

/**
 */
bool Server::is_vector_clock_sync(string list_vc_sender, string node_addr_sender) {
    
    bool is_sync = true;
    StringTokenizer tokenizer(list_vc_sender, LIST_SEPARATOR);
    
    for (int i = 0; i < tokenizer.countTokens(); i++) {
        
        string* vc_tokens = parse_vector_clock(tokenizer.nextToken());
        
        string node_addr = vc_tokens[0];
        int value = std::atoi(vc_tokens[1].c_str());
        
        if (node_addr_sender.compare(node_addr) != 0) {
            
            if (value > (*vc)[node_addr]) {
                cout << "is not equal nodes " << value << " is gt " << (*vc)[node_addr] << endl;
                is_sync = false;
                break;
            }
            
        } else {
            
            if (value != ((*vc)[node_addr] + 1)) {
                cout << "is equal nodes " << value << " <> " << (*vc)[node_addr] + 1 << endl;
                is_sync = false;
                break;
            }
            
        }
        
    }
    
    return is_sync;
}

bool Server::is_vector_clock_lq(string list_vc) {
    
    bool is_sync = true;
    StringTokenizer tokenizer(list_vc, LIST_SEPARATOR);
    
    for (int i = 0; i < tokenizer.countTokens(); i++) {
        
        string* vc_tokens = parse_vector_clock(tokenizer.nextToken());
        string node_addr = vc_tokens[0];
        int value = std::atoi(vc_tokens[1].c_str());
        
        if (value > (*vc)[node_addr]) {
            is_sync = false;
            break;
        }
    }
    
    return is_sync;
}

list<string> Server::check_queue_messages() {
    
    list<string> poped_messages;
    
    for (list<string>::iterator it = messages_queue.begin(); it != messages_queue.end(); it++) {
        
        string connit = *it;
        string* tokens = format_connit_message(connit);
        
        if (tokens != NULL) {
            
            string node_addr_sender = tokens[0];
            string vc_list_sender = tokens[1];
            string text_message = tokens[2];
            
            if (is_vector_clock_lq(vc_list_sender)) {
                poped_messages.push_back(text_message);
                messages_queue.erase(it);
            }
            
        }
        
    }
    
    return poped_messages;
}

void Server::print_messages(list<string> messages) {
    
    for (list<string>::iterator it = messages.begin(); it != messages.end(); it++) {
        cout << "> " << *it << endl;
    }
    
    cout << endl;
}

bool Server::is_node_exists(string node) {
    
    for (list<string>::iterator it = (*nodes).begin(); it != (*nodes).end(); it++) {
        
        if (!(*it).compare(node)) {
            return true;
        }
        
    }
    
    return false;
}