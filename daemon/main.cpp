#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <filesystem>
#include <sys/stat.h> 
#include <sstream>
#include <chrono> // For std::chrono
#include "globals.hpp"
#include "storage/database.cpp"
#include "storage/transaction.cpp"


#include "./repl.cpp"


const char* SOCKET_PATH = "/tmp/mydb.sock";

void handle_client(int client_socket) {
    repl(client_socket);
    close(client_socket);
}


void run_server() {
    int server_socket, client_socket;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    listen(server_socket, 5);

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
}

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    chdir("/");

    for (long x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
}

int main() {
    daemonize();

    if (std::filesystem::exists(SOCKET_PATH)) {
        std::filesystem::remove(SOCKET_PATH);
    }

    if(!filesystem::exists(BASE_DIRECTORY)){
        filesystem::create_directory(BASE_DIRECTORY);
    }



    run_server();

    return 0;
}
