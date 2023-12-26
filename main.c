#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "job.h"

#define MAX_CLIENTS 5
#define PRINTERPORT 8088

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Échec de la création du socket serveur");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a specific IP address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PRINTERPORT);

    // Start listening for incoming connections
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Échec de la liaison du socket serveur");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Échec de l'écoute du socket serveur");
        exit(EXIT_FAILURE);
    }

    // Loop to accept incoming connections
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
            perror("Échec de l'acceptation de la connexion");
            exit(EXIT_FAILURE);
        }

        // Buffer to store received data
        struct job buffer;
        memset(&buffer, 0, sizeof(struct job));

        // Loop to receive data from the client
        while (1) {
            int bytes_received = recv(new_socket, &buffer, sizeof(struct job), 0);
            if (bytes_received < 0) {
                if (errno == EBADF) {
                    perror("Socket is not valid");
                } else {
                    perror("Échec de la réception des données");
                }
                exit(EXIT_FAILURE);
            }

            // If received data size is zero, it means the client has closed the connection
            if (bytes_received == 0) {
                printf("connection closed by client");
                close(new_socket); // Close the socket when done
                break; // Exit the outer while loop
            }

            // Process the received data
            FILE *printerFile;
            int ppm = 60;
            char name[20];
            sprintf(name, "imprimante_%s.log", buffer.printerId);
            printf("%s \n", name);

            printerFile = fopen(name, "a");
            if (printerFile == NULL) {
                perror("Cannot open file\n");
                exit(3);
            }

            fprintf(printerFile, "Imprime %d pages : ", buffer.pages);
            fflush(printerFile);

            sleep(buffer.pages * 60 / ppm);
            fprintf(printerFile, "Fait en %d secondes\n", buffer.pages * 60 / ppm);
            fflush(printerFile);
            printf("Fin impression %d \n", buffer.pages);
            fflush(stdout);
            printf("Received: Printer ID: %s, Pages: %d, Priority: %d \n", buffer.printerId, buffer.pages,
                   buffer.priority);
            close(new_socket); // Close the socket when done
            break; // Exit the outer while loop
        }
    }

    return 0;
}