#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>        // For file descriptor operations
#include <sys/sendfile.h> // For sendfile()
#include <sys/mman.h>     // For memory mapping
#include <sys/syscall.h>  // For memfd_create()

#define PORT 6435 /* Port number for the server */
#define SHM_KEY 0x5678  /* Key for the shared memory */
#define MAX_BATCH_SIZE 65536000 /* Max data size in shared memory */

#define SIGNAL_READY 1 /* Signal ready for update */
#define SIGNAL_IDLE 0 /* Signal not ready for update */

char* dto_data = NULL; // Data Transfer Object (DTO) for shared memory data
pthread_mutex_t dto_lock = PTHREAD_MUTEX_INITIALIZER;  // Mutex for shared memory access

// Function to read from shared memory (runs in its own thread)
void* shared_memory_reader(void* arg) {
    // Access shared memory
    int shmid = shmget(SHM_KEY, sizeof(int) + MAX_BATCH_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    char* shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (char *)-1) {
        perror("shmat failed");
        exit(1);
    }
    dto_data = shm_addr + sizeof(int);  // Use shared memory data directly

    int *signal_mem = (int *)shm_addr;

    while (1) {
        if (*signal_mem == SIGNAL_READY) {
            // Lock the mutex to ensure exclusive access to shared memory
            pthread_mutex_lock(&dto_lock);

            // Print update confirmation to the terminal
            puts("Shared memory updated. Data available for transfer.");

            // Set the signal back to idle
            *signal_mem = SIGNAL_IDLE;

            // Unlock the mutex
            pthread_mutex_unlock(&dto_lock);

            // Sleep for a short time to avoid rapid loops
            usleep(1000);  // Sleep for 1 millisecond
        }

        // Sleep to avoid busy-waiting if there's no update
        usleep(1000);  // Sleep for 1 millisecond
    }

    return NULL;
}

// Function to run the server (runs in its own thread)
void* run_server(void* arg) {
    int server_fd, client_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(struct sockaddr_in);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);


    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind the socket
  bind(server_fd, (struct sockaddr*)&server, sizeof(server)) ;

    // Listen for connections
    listen(server_fd, 5);
    puts("Waiting for incoming connections...");

        const char* data = dto_data;  // Access the DTO data directly from shared memory

        int mem_fd = syscall(SYS_memfd_create, "shm_memfile", 0);

        size_t data_len = strlen(data);
    

    // Main server loop
while ((client_sock = accept(server_fd, (struct sockaddr*)&client, &client_len)) >= 0) {
    
    

    // Write shared memory data to the in-memory file
    write(mem_fd, data, data_len);
    lseek(mem_fd, 0, SEEK_SET);  // Reset file offset to the beginning

    

    // Use sendfile to send the data directly from the in-memory file descriptor
   sendfile(client_sock, mem_fd, NULL, data_len);

    // Clean up
    close(mem_fd);  // Close the in-memory file descriptor
    close(client_sock);  // Close the client socket connection
}

    close(server_fd);
    return NULL;
}

int main() {
    pthread_t server_thread, shm_thread;

    // Create the shared memory reader thread
    pthread_create(&shm_thread, NULL, shared_memory_reader, NULL);

    // Create the server thread
    pthread_create(&server_thread, NULL, run_server, NULL);

    // Wait for both threads to finish
    pthread_join(shm_thread, NULL);
    pthread_join(server_thread, NULL);

    return 0;
}
