#include "Client.h"
#include "Server.h"

#define TRACKER_HOST "127.0.0.1"
#define TRACKER_PORT "65000"

using namespace std;

void* server_handler(void* o) {
    
    cout << endl;
    
    Address* addr = (Address*) o;
    
    Server s;
    s.start(addr->host, addr->port);
    
    return NULL;
}

void* client_handler(void* o) {
    
    sleep(1);
    
    cout << endl;
    
    Address* addr = (Address*) o;
    Client c(TRACKER_HOST, TRACKER_PORT);
    
    // send new node from tracker
    c.new_node(addr->host, addr->port);
    
    while (1) {
        
        int o = c.menu();
        cin.ignore();
        
        if (o == MENU_PRINT_NODES) {
            cout << c.get_nodes() << endl;
        } else if (o == MENU_ADD) {
            
            string key, value;
            
            cout << "key: ";
            getline(cin, key);
            
            cout << "value: ";
            getline(cin, value);
            
            string connit = c.make_connit(key, value);
            c.add(connit);
            
        } else if (o == MENU_DELETE) {
            
        }
        
    }
    
    return NULL;
}

int main(int argc, const char * argv[]) {
    
    if (argc != 3) {
        cout << "Enter node host and port" << endl;
        exit(1);
    }
    
    Address address;
    address.host = argv[1];
    address.port = argv[2];
    
    pthread_t t_server, t_client;
    
    pthread_create(&t_server, NULL, server_handler, &address);
    pthread_create(&t_server, NULL, client_handler, &address);
    
    pthread_join(t_client, NULL);
    pthread_join(t_server, NULL);
    
    return 0;
}
