#ifndef GLOBAL_PA1
#define GLOBAL_PA1

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <string.h> 
#include <strings.h>
#include <algorithm>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#define GOOGLE_DNS "8.8.8.8"
#define GOOGLE_DNS_PORT 53

// namespace COMMON {
enum Instructions{
    AUTHOR,
    IP,
    PORT,
    LIST,
    STATISTICS,
    BLOCKED,
    LOGIN,
    REFRESH,
    SEND,
    SENDHSTNAM,
    BROADCAST,
    BLOCK,
    UNBLOCK,
    LOGOUT,
    EXIT
};


extern std::unordered_map<std::string, Instructions> InstructionMap;

std::vector<std::string> split(const std::string &text, char sep);

void cmd_parser(const char * cmd, std::vector<std::string> &cmd_str);

void cmd_first_msg_parser(const char *cmd, std::string &msgs);

void cmd_sec_msg_parser(const char *cmd, std::string &msgs);

void terminal_output_success(std::string &cmd, std::vector<std::string> &outputs);

void terminal_output_fail(std::string &cmd);

void debug_output(const char * buffer);

std::string find_external_ip();

void get_peer_ip(int socketfd, std::string &peer_ip_str, int &peer_port, std::string &peer_hostname);

void get_local_hostname(std::string &hostname);

bool is_ip_valid(const std::string &ip_str);

bool is_port_valid(const int &port);

bool is_cmd_valid(const std::string &cmd);

int send_msg(int server_socketfd, const std::string &msg);

class Client
{
private:
    int socketfd;
    int port;
    std::string ip;
    std::string hostname;
    std::string status;

    int num_msgs_recv;
    int num_msgs_sent;

    std::unordered_set<std::string> block_list;
    std::vector<std::string> msgs_buffer;

public:
    Client();
    Client(int socketfd_, std::string ip_, int port_, std::string status_ );

    bool operator < (const Client& str) const;

    void set_status(std::string status_);

    std::string get_status();

    std::string get_hostname();

    void set_hostname(std::string hostname_);

    std::string get_ip();

    int get_port();

    int get_num_msgs_sent();

    int get_num_msgs_recvd();

    void add_buffer_msgs(std::string buffer_msg);

    void increment_sent_msgs_count();

    void increment_recvd_msgs_count();

    void add_to_block_list(std::string ip);

    void remove_from_block_list(std::string ip);

    bool is_blocked(std::string ip);

    std::unordered_set<std::string> return_blocklist();

};

class ClientsList
{
private:
    std::unordered_map<int,Client> clients_map;
    std::vector<Client> clients_vector;
    std::unordered_map<std::string, int> ip_to_fd;
public:
    void add(int fd,  Client client);

    void remove(int fd);

    void sort_clients();

    void display_login_clients(std::vector<std::string> &terminal_outputs);

    void display_statistics(std::vector<std::string> &terminal_outputs);

    void display_block_list(std::string client_ip, std::vector<std::string> &terminal_outputs);

    std::string get_clientslist_str();

    const std::unordered_map<int,Client> get_clientslist();

    Client& get_client_by_fd(int fd);

    int get_fd_by_ip(std::string ip);

};
// }

#endif