/**
 * @client
 * @author  Sen Pan <senpan@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the client.
 */

#include "client.hpp"

int ClientHost::client_start()
{

	int head_socket, selret, sock_index, fdaccept=0;
	int server_socket = -1;
	fd_set master_list, watch_list;

	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	
	/* Register STDIN */
	FD_SET(STDIN, &master_list);

	head_socket = STDIN;

	int server;
	// server = connect_to_host(argv[1], atoi(argv[2]));
	
	/* Check if we have sockets/STDIN to process */
	while(TRUE){
		printf("\n[PA1-Client@CSE489/589]$ ");
		fflush(stdout);

		memcpy(&watch_list, &master_list, sizeof(master_list));

		/* select() system call. This will BLOCK */
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if(selret < 0)
			perror("select failed.");

       	if(selret > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){

                if(FD_ISSET(sock_index, &watch_list)){
				
					 /* Check if new command on STDIN */
                    if (sock_index == STDIN){
						
						char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
						memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);

						// printf("\nI got: %s\n", cmd);
						// int cmdint = atoi(cmd);

						/*------------------Process PA1 commands here --------------------*/

						/*Slipt input command string into tokens*/
						std::vector<std::string> commands;
						cmd_parser(cmd, commands);

						/*check if command is valid*/
						if(!is_cmd_valid(commands[0]))
						{
							std::cout << commands[0] << " : Invlid command!" << std::endl;
							continue;
						}

						/* convert command string to command enum for switching */
						Instructions command_enum = InstructionMap.at(commands[0]);

						/* declare terminal outputs vector */
						std::vector<std::string> terminal_outs;

						/*switch into corresponding commands*/
						switch(command_enum){
							case AUTHOR:
							{
								terminal_outs.clear();
								std::string terminal_out = "I,senpan, have read and understood the course academic integrity policy.";
								terminal_outs.push_back(terminal_out);
								terminal_output_success(commands[0], terminal_outs);
								break;
							}

							case IP:
							{
								terminal_outs.clear();
								std::string terminal_out = "IP:" + find_external_ip();
								terminal_outs.push_back(terminal_out);
								terminal_output_success(commands[0], terminal_outs);
								break;
							}

							case PORT:
							{
								terminal_outs.clear();
								std::string terminal_out = "PORT:" + std::to_string(port);
								terminal_outs.push_back(terminal_out);
								terminal_output_success(commands[0], terminal_outs);
								break;
							}

							case LOGIN:
							{	
								/* check size of commands */
								if(commands.size() != 3)
								{
									std::cout << "invalid parameter!" << std::endl;
									continue;
								}
								
								/* check if ip address is valid */
								if(!is_ip_valid(commands[1]))
								{
									std::cout << "invalid ip address!" << std::endl;
									continue;										
								}

								/* check if ip address is valid */
								int host_port_num = std::stoi(commands[2]);
								if(!is_port_valid(host_port_num))
								{
									std::cout << "invalid ip address!" << std::endl;
									continue;										
								}
								
								this->server_fd = connect_to_server(commands[1],host_port_num, port);
								if(this->server_fd < 0)
								{
									std::cout << "failed to connect to server" << std::endl;
									continue;
								}

								/* Register the listening socket */
								FD_SET(this->server_fd, &master_list);
								head_socket = this->server_fd;
								
								/*change login status*/
								this->login_status = 1;

								/*send host name to server*/
								std::string localhostname;
								get_local_hostname(localhostname);
								std::string cmd_msgs = "SENDHSTNAM ";
								send_msg(this->server_fd, cmd_msgs+localhostname);

								/*out put status to terminal*/
								terminal_outs.clear();
								terminal_output_success(commands[0],terminal_outs);
								break;
							}

							case SEND:
							{
								if(commands.size() < 2)
								{
									std::cout << "No IP address found, please input IP address of the receiver!" << std::endl;
									continue;
								}
								else if(commands.size() < 3)
								{
									std::cout << "No messages to send, please input messages after SEND command!" << std::endl;
									continue;
								}
								
								// std::string msgs;
								// cmd_msg_parser(cmd, msgs);
								send_msg(this->server_fd, std::string(cmd));

								break;
							}
							case LOGOUT:
							{

								break;
							}

							case EXIT:
							{

								break;
							}
						}//end of switch
						
					}else if(sock_index == this->server_fd)
					{
						/*check if any new message from server*/
						this->recv_msg(this->server_fd);
					}
					else
					{
						
					}

				}//End of checking socket_index
			}//End of for loop of chcking socket index
		}//End of select positive

	}//end of while loop
}

int ClientHost::connect_to_server(std::string &server_ip, int server_port, int client_port)
{
    int fdsocket, len;
    struct sockaddr_in remote_server_addr;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0)
       perror("Failed to create socket");

	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(client_port);

	if(bind(fdsocket, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in)) < 0)
	{
		close(fdsocket);
		perror("client fail to bind with port");
		return -1;
	}


    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip.c_str(), &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
    {
		close(fdsocket);
		perror("client fail to connect with server");
		return -1;
	}
	else
	{
		std::cout << "Connect Success!" << std::endl;
	}
	
    return fdsocket;
}

int ClientHost::send_msg(int server_socketfd, const std::string &msg)
{
	const char *msg_cstr = msg.c_str();
	int len = strlen(msg_cstr);

	if(send(server_socketfd, msg_cstr, len, 0));
	return 0;
}

int ClientHost::recv_msg(int server_fd)
{
	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);

	if(recv(server_fd, buffer, BUFFER_SIZE, 0) >= 0){
		printf("Server responded: %s", buffer);
		fflush(stdout);
	}else
	{
		printf("fail to reveive message from server");
		fflush(stdout);
		return -1;
	}
	
	return 0;
}

int ClientHost::broadcast_msg()
{
	return 0;
}
