#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>


#define PORT 6435
#define SERVER_IP "127.0.0.1"
#define MAX_DATA_SIZE 65536000  // Maximum data size to be received

int main() {
    int sock;
    struct sockaddr_in server_addr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    // Convert server IP address from text to binary form
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) ;



    // Connect to server
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) ;
    
    // Memory mapping for zero-copy data reception
    char *recv_data = mmap(NULL, MAX_DATA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);


    // Directly read data from the socket into the mapped memory region
    ssize_t bytes_received = recv(sock, recv_data, MAX_DATA_SIZE, 0);




    // Print the received data directly from the memory-mapped region
    printf("Received data:\n%.*s\n", (int)bytes_received, recv_data);

    // Cleanup
    munmap(recv_data, MAX_DATA_SIZE);
    close(sock);


    return 0;
}
