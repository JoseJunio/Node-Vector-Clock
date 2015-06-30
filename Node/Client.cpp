//
// Created by MarioJ on 25/06/15.
//

#include "Client.h"

Client::Client(Addresses* _addresses, list<string>* _nodes, map<string, int>* _vc) {
    addresses = _addresses;
    nodes = _nodes;
    vc = _vc;
    
    string my_addr("");
    my_addr.append(addresses->node_host).append(" ").append(to_string(addresses->node_port));
    
    (*vc).insert(pair<string, int>(my_addr, 0));
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
    cout << "> Try connecting at " << host << ":" << port << endl;
    
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
    
    if (!(*nodes).empty()) {
        
        list<string>::iterator it;
        
        for (it = (*nodes).begin(); it != (*nodes).end(); it++) {
            nodes_str.append(*it).append(",");
        }
        
        nodes_str = nodes_str.substr(0, nodes_str.length() - 1);
        
    } else {
        nodes_str = "Nodes no active right now.";
    }
    
    return nodes_str;
    
}

void Client::new_node() {
    
    string request(NEW_NODE);
    request.append(" ").append(addresses->node_host).append(" ").append(to_string(addresses->node_port)).append(NET_EOM);
    
    // connect on tracker
    conect(addresses->tracker_host, to_string(addresses->tracker_port));
    
    // send request
    send(request);
    
    // get response with nodes
    string nodes_str = receive();
    
    // close connect
    close();
    
    // remove prefix function
    nodes_str = nodes_str.substr(strlen(NODES) + 1, nodes_str.length() - strlen(NET_EOM));
    
    // check if there's nodes in tracker
    if (!nodes_str.empty()) {
        
        // my addr
        string my_addr("");
        my_addr.append(addresses->node_host).append(" ").append(to_string(addresses->node_port));
        
        // tokenizer by separator
        StringTokenizer tokenizer(nodes_str, LIST_SEPARATOR);
        int count_tokens = tokenizer.countTokens();
        
        // loop throught the tokens
        for (int i = 0; i < count_tokens; i++) {
            
            // get next addr
            string addr = tokenizer.nextToken();
            
            // check if addr is same the my addr, otherwise add to nodes
            if (!addr.compare(my_addr)) {
                cout << "> My address " << addr << endl;
            } else {
                (*nodes).push_back(addr);
            }
            
        }
        
    } else {
        cout << "> No nodes active in tracker." << endl;
    }
    
    cout.flush();
}

/**
 Syncronize local vector clock with some node in net
 */
void Client::sync_vector_clock() {
    
    cout << endl;
    
    // mount expression to send server
    string expression(VC);
    
    // append final message
    expression.append(NET_EOM);
    
    // get randomic node
    string* addr = random_addr();
    
    if (addr == NULL)
        return;
    
    // connect to node and get vector clock as string
    conect(addr[0], addr[1]);
    send(expression);
    string list_str_vc = receive();
    close();
    
    cout << "> Syncronizing vector clock local" << endl << endl;
    
    // update list vector clock's string
    update_vc(list_str_vc);
    
}

/**
 Send broadcast message for all nodes in tracker to add this node in their list
 */
void Client::broadcast_new_node() {
    
    for (list<string>::iterator it = (*nodes).begin(); it != (*nodes).end(); it++) {
        
        string node_addr = (*it);
        
        // get host and port of node
        string* tokens = format_addr(node_addr);
        
        // make expression to send server node
        string expression(NEW_NODE);
        expression.append(" ").append(addresses->node_host).append(" ").append(to_string(addresses->node_port)).append(NET_EOM);
        
        // connect node and send address of node
        conect(tokens[0], tokens[1]);
        send(expression);
        close();
        
        cout << "> Sending broadcast address" << endl << endl;
    }
    
}

void Client::send_message(string connit) {
    
    if ((*nodes).empty()) {
        cout << "> No nodes activity in tracker" << endl;
        return;
    }
    
    string node_addr;
    node_addr.assign(addresses->node_host).append(" ").append(to_string(addresses->node_port));
    
    // increment vector clock at node sender position
    (*vc)[node_addr]++;
    
    string message(MESSAGE);
    message.append(" ").append(addresses->node_host).append(" ").append(to_string(addresses->node_port)).append(MESSAGE_SEPARATOR_ARG).append(format_vector_clock()).append(MESSAGE_SEPARATOR_ARG).append(connit).append(NET_EOM);
    
    cout << endl << "> " << message << endl;
    
    list<string>::iterator it;
    
    for (it = (*nodes).begin(); it != (*nodes).end(); it++) {
        
        string node_addr = *it;
        
        StringTokenizer tokenizer(node_addr, " ");
        string host = tokenizer.nextToken(), port = tokenizer.nextToken();
        
        conect(host, port);
        send(message);
        close();
        
    }
    
}

string Client::format_vector_clock() {
    
    if ((*vc).empty()) {
        cout << "No vector clock " << endl;
        return NULL;
    }
    
    string vc_str("");
    
    for (map<string, int>::iterator it = (*vc).begin(); it != (*vc).end(); it++)
        vc_str.append((*it).first).append(VECTOR_CLOCK_KEY_VALUE_SEPARATOR).append(to_string((*it).second)).append(LIST_SEPARATOR);
    
    return vc_str.substr(0, vc_str.size() - strlen(LIST_SEPARATOR));
}

int Client::menu() {
    
    int o;
    
    cout << endl << "(1) Print nodes" << endl;
    cout << "(2) New message" << endl;
    cout << "(3) Print Vector Clock" << endl;
    cout.flush();
    
    cin >> o;
    
    return o;
}

string* Client::random_addr() {
    
    cout << endl;
    
    if ((*nodes).empty()) {
        cout << "> No nodes active in tracker" << endl;
        return NULL;
    }
    
    static bool first = true;
    long nodes_size = (*nodes).size();
    
    if (first) {
        srand((unsigned int)time(NULL));
        first = !first;
    }
    
    int index = rand() % nodes_size;
    
    list<string>::iterator it = (*nodes).begin();
    advance(it, index);
    
    string addr = (*it);
    
    if (addr.empty())
        return NULL;
    
    return format_addr(addr);
}

void Client::update_vc(string list_vc_str) {
    
    StringTokenizer tokenizer(list_vc_str, LIST_SEPARATOR);
    int count = tokenizer.countTokens();
    
    for (int i = 0; i < count; i++) {
        
        // get next vector clock
        string str_vc = tokenizer.nextToken();
        
        StringTokenizer tokenizer_vc(str_vc, VECTOR_CLOCK_KEY_VALUE_SEPARATOR);
        string node = tokenizer_vc.nextToken();
        int vc_value = tokenizer_vc.nextIntToken();
        
        // put into vector clock map
        (*vc).insert(make_pair(node, vc_value));
    }
    
}

string* Client::format_addr(string addr) {
    
    string* tokens = new string[2];
    
    StringTokenizer tokenizer(addr, " ");
    
    tokens[0] = tokenizer.nextToken();
    tokens[1] = tokenizer.nextToken();
    
    return tokens;
    
}
