#include "Client.h"
#include "Server.h"

#define TRACKER_HOST "127.0.0.1"
#define TRACKER_PORT 65000

#define MENU_PRINT_NODES 1
#define MENU_NEW_MESSAGE 2
#define MENU_PRINT_VC 3

using namespace std;

list<string> nodes;
map<string, int> vc;

void* server_handler(void* o) {
    
    cout << "Thread Server [OK]" << endl << endl;
    
    Address* addr = (Address*) o;
    
    Server s(&nodes, &vc);
    s.start(to_string(addr->port));
    
    return NULL;
}

void* client_handler(void* o) {
    
    sleep(1);
    
    cout << "Thread Client [OK]" << endl << endl;
    
    cout << endl;
    
    Address* addr = (Address*) o;
    stringstream ss;
    
    Addresses addresses;
    addresses.tracker_host = TRACKER_HOST;
    addresses.tracker_port = TRACKER_PORT;
    addresses.node_host = addr->host;
    addresses.node_port = addr->port;
    
    // instace client
    Client c(&addresses, &nodes, &vc);
    
    // send new node from tracker to register it
    c.new_node();
    
    /**
     Syncronize current vector clock with other node at net
     */
    c.sync_vector_clock();
    
    /**
     Send broadcast message to all nodes in tracker to register this node at their list nodes
     */
    c.broadcast_new_node(addr->name.empty() ? "Anonimo" : addr->name);
    
    // username
    ss << "[" << (addr->name.empty() ? "Anonimo" : addr->name)  << "]: ";
    
    if (!DEBUG_NODE)
        cout << "############## CHAT ##############" << endl << endl;
    
    if (DEBUG_NODE) {
        
        while (1) {
            
            int o = c.menu();
            cin.ignore();
            
            if (o == MENU_PRINT_NODES) {
                cout << endl << c.get_nodes() << endl;
            } else if (o == MENU_NEW_MESSAGE) {
                
                string message, message_full;
                
                getline(cin, message);
                
                message_full.append(ss.str()).append(message);
                
                c.send_message(message_full);
                
            } else if (o == MENU_PRINT_VC) {
                cout << endl << c.format_vector_clock() << endl;
            }
            
            cout.flush();
        }
        
    } else {
        
        while (1) {
            
            string message, message_full;
            
            getline(cin, message);
            
            message_full.append(ss.str()).append(message);
            
            c.send_message(message_full);
            
            cout.flush();
        }
        
    }
    
    return NULL;
}

int main(int argc, const char * argv[]) {
    
    if (argc < 3) {
        cout << "Enter node host and port" << endl;
        exit(1);
    }
    
    Address address;
    address.host = argv[1];
    address.port = atoi(argv[2]);
    
    if (argv[3] != NULL)
        address.name = argv[3];
    
    pthread_t t_server, t_client;
    
    pthread_create(&t_server, NULL, server_handler, &address);
    pthread_create(&t_server, NULL, client_handler, &address);
    
    pthread_join(t_client, NULL);
    pthread_join(t_server, NULL);
    
    return 0;
}
