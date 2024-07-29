#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using namespace std;
const char* SOCKET_PATH = "/tmp/mydb.sock";

void send_command(int client_socket, const std::string& command) {
    char buffer[200000];

    write(client_socket, command.c_str(), command.length());
    int n = read(client_socket, buffer, 19999);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << buffer << std::endl;
    }
}

int main() {
    int client_socket;
    struct sockaddr_un server_addr;

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char buffer[200000];

    int n = read(client_socket, buffer, 19999);
    buffer[n]='\0';
    cout<<buffer<<endl;
    std::string command;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        if (command == "exit") {
            break;
        }
        send_command(client_socket, command);
    }

    close(client_socket);
    return 0;
}
