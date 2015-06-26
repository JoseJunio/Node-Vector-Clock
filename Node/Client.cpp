//
// Created by MarioJ on 25/06/15.
//

#include "Client.h"

Client::Client(string _host_tracker, string _port_tracker) {
    host_tracker = _host_tracker;
    port_tracker = _port_tracker;
}

void Client::conect(string host, string port) {
    
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }
    
    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        
        if ((socket = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("> Socket");
            continue;
        }
        
        if (connect(socket, p->ai_addr, p->ai_addrlen) == -1) {
            ::close(socket);
            perror("> Connect");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "> Failed to connect\n");
        return;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
    cout << "> Try connecting at " << host_tracker << ":" << port_tracker << endl;
    
    freeaddrinfo(servinfo); // all done with this structure
}

void Client::send(string message) {
    
    if (::send(socket, message.c_str(), message.length(), 0) == -1)
        cout << "> Message not send: " << message << endl << endl;
}

void Client::end_message() {
    send(NET_EOM);
}

string Client::receive() {
    
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

void Client::close() {
    ::close(socket);
}

void* Client::get_in_addr(struct sockaddr *sa) {
    
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string Client::get_nodes() {
    
    string nodes_str("");
    
    if (!nodes.empty()) {
        
        list<string>::iterator it;
        
        for (it = nodes.begin(); it != nodes.end(); it++) {
            nodes_str.append(*it).append(",");
        }
        
        nodes_str = nodes_str.substr(0, nodes_str.length() - 1);
        
    } else {
        nodes_str = "Nodes no active right now.";
    }
    
    return nodes_str;
    
}

void Client::new_node(string host, string port) {
    
    string request(NEW_NODE);
    request.append(" ").append(host).append(" ").append(port).append(NET_EOM);
    
    // connect on tracker
    conect(host_tracker, port_tracker);
    
    // send request
    send(request);
    
    // get response with nodes
    string nodes_str = receive();
    
    // close connect
    close();
    
    // remove prefix function
    nodes_str = nodes_str.substr(strlen(NODES) + 1, nodes_str.length());
    
    // check if there's nodes in tracker
    if (!nodes_str.empty()) {
        
        // my addr
        string my_addr("");
        my_addr.append(host).append(" ").append(port);
        
        // tokenizer by separator
        StringTokenizer tokenizer(nodes_str, LIST_SEPARATOR);
        int count_tokens = tokenizer.countTokens();
        
        // loop throught the tokens
        for (int i = 0; i < count_tokens; i++) {
            
            // get next addr
            string addr = tokenizer.nextToken();
            
            // check if addr is same the my addr, otherwise add to nodes
            if (!addr.compare(my_addr)) {
                cout << "my addr " << addr << endl;
            } else {
                nodes.push_back(addr);
            }
            
        }
        
    } else {
        cout << "No nodes active on tracker." << endl;
    }
    
    cout.flush();
    
}

void Client::add(string connit) {
    
    if (nodes.empty()) {
        cout << "> no nodes active" << endl;
        return;
    }
    
    string add(ADD);
    add.append(" ").append(connit).append(NET_EOM);
    
    cout << "> " << add << endl;
    
    list<string>::iterator it;
    
    for (it = nodes.begin(); it != nodes.end(); it++) {
        
        string node_addr = *it;
        
        StringTokenizer tokenizer(node_addr, " ");
        
        string host = tokenizer.nextToken(), port = tokenizer.nextToken();
        
        cout << "> sending to " << host << ":" << port << endl;
        
        conect(host, port);
        send(add);
        close();
        
    }
    
}

string Client::make_connit(string key, string value) {
    return key.append(CONNIT_SEPARATOR).append(value);
}

int Client::menu() {
    
    int o;
    
    cout << endl << "(1) print nodes" << endl;
    cout << "(2) add" << endl;
    cout << "(3) delete" << endl;
    
    cout << endl << "> ";
    cout.flush();
    
    cin >> o;
    
    return o;
    
}
