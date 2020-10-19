#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../include/common.hpp"
#define TRUE 1
#define CMD_SIZE 1024
#define MSG_SIZE 1024
#define BUFFER_SIZE 1024
#define STDIN 0


class ClientHost
{
private:
int server_fd;
int port;
int login_status;
int exit_status;

std::unordered_map<std::string,Client> local_clients_map;

public:
ClientHost(){};
ClientHost(int port_) : port(port_), login_status(0), exit_status(0) {};
~ClientHost();
int client_start();
int connect_to_server(std::string &server_ip, int server_port, int client_port);
int send_msg(int server_socketfd, const std::string &msg);
int recv_msg(int server_fd, std::string &msgs_recvd);
int broadcast_msg();
void update_local_clients_map(std::string clientslist_str);
std::vector<std::string> split(const std::string &text, char sep);
};

#endif