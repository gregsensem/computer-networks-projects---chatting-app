#include "../include/common.hpp"
#include "../include/logger.h"

// namespace COMMON{

std::unordered_map<std::string, Instructions> InstructionMap = 
{
    {"AUTHOR", Instructions::AUTHOR},
    {"IP", Instructions::IP},
    {"PORT", Instructions::PORT},
    {"LIST", Instructions::LIST},
    {"LOGIN", Instructions::LOGIN},
    {"LOGOUT", Instructions::LOGOUT},
    {"EXIT", Instructions::EXIT}
};

//refered to stackoverflow solution of splitting string in c++
std::vector<std::string> split(const std::string &text, char sep) {
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while ((end = text.find(sep, start)) != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}

void input_parser(char * cmd, std::vector<std::string> &cmd_str)
{
    std::string s(cmd);
    //remove the newline character at the end of the line
    if (!s.empty() && s[s.length()-1] == '\n') 
        s.erase(s.length()-1);

    cmd_str = split(s, ' ');
}

void terminal_output_success(std::string &cmd, std::vector<std::string> &outputs)
{
    // std::cout << '[' << cmd << ":SUCCESS]" << std::endl;
    // std::cout << msg << std::endl;
    // std::cout << '[' << cmd << ":END]" << std::endl;
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    for(auto &output : outputs)
    {
        cse4589_print_and_log("%s\n", output.c_str());
    }
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void terminal_out_fail(std::string &cmd)
{
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

std::string find_external_ip()
{
    struct sockaddr_in google_dns_addr;
    google_dns_addr.sin_family = AF_INET;
    google_dns_addr.sin_port = htons(GOOGLE_DNS_PORT);
    google_dns_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    // inet_pton(AF_INET,GOOGLE_DNS,&(google_dns_addr.sin_addr));

    int socketfd = socket(PF_INET, SOCK_DGRAM, 0);

    if(connect(socketfd, (struct sockaddr *)&google_dns_addr, sizeof(google_dns_addr)) == -1 )
    {
        perror("Get IP fail: connect");
    }

    struct sockaddr_in host_addr;
    char host_ip[16];

    socklen_t host_addr_len = sizeof(host_addr);
    socklen_t host_ip_len = sizeof(host_ip);

    if(getsockname(socketfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addr_len) == -1)
    {
        perror("Get IP fail: getsockname");
    }
    inet_ntop(AF_INET, &host_addr.sin_addr, host_ip, host_ip_len);

    char host_name[100];
    size_t host_name_len = sizeof(host_name);
    if(gethostname(host_name, host_name_len) == -1)
    {
        perror("Get IP fail: gethostname");
    }

    return std::string(host_ip);
}

void get_peer_ip(int socketfd, std::string &peer_ip_str, int &peer_port, std::string &peer_hostname)
{
    /*-------------------find out the peer IP & PORT --------------------------*/

    struct sockaddr_in peer_addr;
    char peer_ip[16];

    socklen_t peer_addr_len = sizeof(peer_addr);
    socklen_t peer_ip_len = sizeof(peer_ip);

    if(getpeername(socketfd, (struct sockaddr *)&peer_addr, (socklen_t *)&peer_addr_len) == -1)
    {
        perror("Got peer IP fail: get_peer_ip");
    }
    inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, peer_ip_len);

    std::string s(peer_ip);
    peer_ip_str = s;
    peer_port = ntohs(peer_addr.sin_port);

    // /*-------------------find out the peer hostname--------------------------*/
    // int status;
    // struct addrinfo hints;
    // struct addrinfo *peerinfo;

    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;

    // const char *peer_ip_char = peer_ip_str.c_str();
    // std::string port = std::to_string(peer_port);
    // char const *peer_port_char = port.c_str(); 

    // printf("peer_ip_char %s \n", peer_ip_char);
    // printf("port %s\n",peer_port_char);

    // if((status = getaddrinfo("www.google.com", NULL, &hints, &peerinfo)) != 0)
    // {
    //     perror("Got peer hostname fail: getaddrinfo()");
    // }
    // else
    // {
    //     std::string h(peerinfo->ai_canonname);
    //     peer_hostname = h;
    // }
}

bool is_ip_valid(const std::string &ip_str)
{
    struct sockaddr_in ip_net;
    return inet_pton(AF_INET, ip_str.c_str(), &(ip_net.sin_addr)) > 0;
}

bool is_port_valid(const int &port)
{
    return (port >= 0) && (port <= 65535);
}

bool is_cmd_valid(const std::string &cmd)
{
    if(InstructionMap.find(cmd) == InstructionMap.end())
    {
        return false;
    }else
    {
        return true;
    }
}

int send_msg(std::string to_ip, int to_port, std::string msg)
{
}


Client::Client(){};
Client::Client(int socketfd_, std::string ip_, std::string hostname_, int port_, std::string status_ ) 
: socketfd(socketfd_), ip(ip_), hostname(hostname_), port(port_), status(status_)
{
    // std::cout << "New client :" << ip << "login!" << std::endl;
};

bool Client::operator < (const Client& str) const
{
    return (port < str.port);
}

void Client::set_status(std::string status_)
{
    status = status_;
}

std::string Client::get_status()
{
    return status;
}

std::string Client::get_hostname()
{
    return hostname;
}

std::string Client::get_ip()
{
    return ip;
}

int Client::get_port()
{
    return port;
}


void ClientsList::add(int fd,  Client client)
{
    clients_map[fd]=client;
    clients_vector.push_back(client);
}

void ClientsList::sort_clients()
{
    std::sort(clients_vector.begin(),clients_vector.end());
}

void ClientsList::display_login_clients(std::vector<std::string> &terminal_outputs)
{
    int i = 1;
    sort_clients();
    for(auto it : clients_vector)
    {
        if(it.get_status() == "LOGIN")
        {
            char buff[256];
            int buff_len = snprintf(buff, sizeof(buff), "%-5d%-35s%-20s%-8d", i++, it.get_hostname().c_str(), it.get_ip().c_str(), it.get_port());
            terminal_outputs.push_back(std::string(buff, buff_len));
        }
    }
}

std::string ClientsList::get_clientslist_str()
{
    sort_clients();
    std::string clientslist_str;
    int i = 1;
    for(auto it : clients_vector)
    {
        std::string client_str = std::to_string(i++)+ "|" + it.get_hostname() + "|" + it.get_ip() + "|" + std::to_string(it.get_port()) + "#";
        clientslist_str += client_str;
    }

    return clientslist_str;
}

