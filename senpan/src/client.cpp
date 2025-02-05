/**
 * @client
 * @author
 *   	Sai Venigalla 	50338187  	<srisaiya@buffalo.edu>
 * 		Sen Pan 		50321231 	<senpan@buffalo.edu>
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
	
	/* Check if we have sockets/STDIN to process */
	while(TRUE){
		/*check if EXIT command is executed*/
		if(this->exit_status)
			return 0;

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

						/*put a copy of cmd by debug*/
						//debug_output(cmd);
						
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
								std::string terminal_out = "I, senpan, have read and understood the course academic integrity policy.";
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

							case LIST:
							{
								terminal_outs.clear();
								int i = 1;

								/*sort local clients map using vector*/
								std::vector<Client> local_clients_vec;
								for(auto it : this->local_clients_map)
								{
									local_clients_vec.push_back(it.second);
								}

								std::sort(local_clients_vec.begin(),local_clients_vec.end());

								for(auto local_clients : local_clients_vec)
								{
									char buff[256];
									int buff_len = snprintf(buff, sizeof(buff), "%-5d%-35s%-20s%-8d", i++, local_clients.get_hostname().c_str(), 
									local_clients.get_ip().c_str(), local_clients.get_port());

            						terminal_outs.push_back(std::string(buff, buff_len));
								}
								terminal_output_success(commands[0], terminal_outs);

								break;
							}

							case LOGIN:
							{	
								/* check size of commands */

								if(commands.size() != 3)
								{
									std::cout << "invalid parameter!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								
								/* check if ip address is valid */
								if(!is_ip_valid(commands[1]))
								{
									std::cout << "invalid ip address!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;										
								}

								/* check if PORT is valid */

								char* p;
								long converted = strtol(commands[2].c_str(), &p, 10);
								if (*p) 
								{
									// conversion failed because the input wasn't a number
									std::cout << "invalid port number!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;	
								}

								int host_port_num = std::stoi(commands[2]);
								if(!is_port_valid(host_port_num))
								{
									std::cout << "invalid port number!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;										
								}

								/*check if the client is already logged in*/
								if(this->login_status == 1)
								{
									std::cout << "Already logged in!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								
								if(this->login_status == -1)
								{
									this->server_fd = connect_to_server(commands[1],host_port_num, port);
									if(this->server_fd < 0)
									{
										std::cout << "failed to connect to server" << std::endl;
										terminal_output_fail(commands[0]);
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
									send_msg(this->server_fd, cmd_msgs+localhostname + "$$");

									/*out put status to terminal*/
									terminal_outs.clear();
									terminal_output_success(commands[0],terminal_outs);
								}
								else if(this->login_status == 0)
								{
									this->send_msg(this->server_fd, "LOGIN$$");
								}

								break;
							}

							case SEND:
							{
								/*check if the client has logged in */
								if(this->login_status != 1)
								{
									std::cout << "please login first!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								if(commands.size() < 2)
								{
									std::cout << "No IP address found, please input IP address of the receiver!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								else if(commands.size() < 3)
								{
									std::cout << "No messages to send, please input messages after SEND command!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								if(!is_ip_valid(commands[1]))
								{
									std::cout << "invalid ip!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								else if (this->local_clients_map.find(commands[1]) == this->local_clients_map.end())
								{
									std::cout << "IP address not found in local list!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								// std::string msgs;
								// cmd_msg_parser(cmd, msgs);
								send_msg(this->server_fd, (std::string(cmd) + "$$"));
                                
								usleep(100000);

								break;
							}

							case BROADCAST:
							{
								/*check if the client has logged in */
								if(this->login_status != 1)
								{
									terminal_output_fail(commands[0]);
									continue;
								}

								else if(commands.size() < 2)
								{
									std::cout << "No messages to send, please input messages after SEND command!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								// std::string msgs;
								// cmd_msg_parser(cmd, msgs);
								send_msg(this->server_fd, (std::string(cmd) + "$$"));

								break;
							}
							
							case REFRESH:
							{
								/*check if the client has logged in */
								if(this->login_status != 1)
								{
									terminal_output_fail(commands[0]);
									continue;
								}

								this->send_msg(this->server_fd, commands[0]+"$$");

								break;
							}

							case LOGOUT:
							{
								/*check if the client has logged in */
								if(this->login_status != 1)
								{
									terminal_output_fail(commands[0]);
									continue;
								}

								this->send_msg(this->server_fd, commands[0] + "$$");
								usleep(1000);
								this->login_status = 0;
								// FD_CLR(this->server_fd, &master_list);
								// close(this->server_fd);

								/*out put status to terminal*/
								terminal_outs.clear();
								terminal_output_success(commands[0],terminal_outs);

								break;
							}

							case BLOCK:
							{
								/*check if the client has logged in */
								if(this->login_status != 1)
								{
									terminal_output_fail(commands[0]);
									continue;
								}
								/*check if input is valid*/
								if(commands.size() != 2)
								{
									std::cout << "invalid parameter!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								
								/* check if ip address is valid */
								if(!is_ip_valid(commands[1]))
								{
									std::cout << "invalid ip address!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;										
								}

								/* check if ip address is in local list */
								if (this->local_clients_map.find(commands[1]) == this->local_clients_map.end())
								{
									std::cout << "IP address not found in local list!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								/* check if ip address has already been blocked */
								if (this->local_block_list.find(commands[1]) != this->local_block_list.end())
								{
									std::cout << "IP address has already been blocked!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								
								/*ask server to block this ip and add to local block list */
								if(this->send_msg(this->server_fd, std::string(cmd)+"$$") == 0)
								{
									this->local_block_list.emplace(commands[1]);
								}	

								break;
							}

							case UNBLOCK:
							{
								/*check if the client has logged in */
								if(this->login_status != 1)
								{
									terminal_output_fail(commands[0]);
									continue;
								}

								/*check if input is valid*/
								if(commands.size() != 2)
								{
									std::cout << "invalid parameter!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								
								/* check if ip address is valid */
								if(!is_ip_valid(commands[1]))
								{
									std::cout << "invalid ip address!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;										
								}

								/* check if ip address is in local list */
								if (this->local_clients_map.find(commands[1]) == this->local_clients_map.end())
								{
									std::cout << "IP address not found in local list!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								/* check if ip address is blocked */
								if (this->local_block_list.find(commands[1]) == this->local_block_list.end())
								{
									std::cout << "IP address has not been blocked!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								/*ask server to unblock this ip and erase from local block list */
								if(this->send_msg(this->server_fd, std::string(cmd)+"$$") == 0)
								{
									this->local_block_list.erase(commands[1]);
								}	

								break;
							}

							case EXIT:
							{
								if(this->login_status == 1)
								{
									this->send_msg(this->server_fd, commands[0] + "$$");
								}
								else
								{
									this->exit_status = 1;
									/*out put status to terminal*/
									terminal_outs.clear();
									terminal_output_success(commands[0],terminal_outs);
								}

								break;
							}
						
							case SENDFILE:
							{
								/*check if cmd is valid */
								if(commands.size() < 2)
								{
									std::cout << "No IP address found, please input IP address of the receiver!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								else if(commands.size() < 3)
								{
									std::cout << "No file to send, please input messages after SENDFILE command!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}

								std::string dest_ip = commands[1];
								this->p2p_dest_ip = dest_ip;

								if(!is_ip_valid(dest_ip))
								{
									std::cout << "invalid ip!" << std::endl;
									terminal_output_fail(commands[0]);
									continue;
								}
								else if (this->local_clients_map.find(dest_ip) == this->local_clients_map.end())
								{
									std::cout << "IP address not found in local list!" << std::endl;
									terminal_output_fail(commands[0]);

									continue;
								}

								int dest_port = this->local_clients_map.at(dest_ip).get_port();
								std::cout << "the dest client's port is : " << dest_port << std::endl;

								std::string file_name;
								cmd_sec_msg_parser(cmd, file_name);
								this->p2p_filename = file_name;

								send_msg(this->server_fd, ("P2PREQ " + dest_ip + ' ' + file_name + "$$"));

								terminal_outs.clear();
								terminal_output_success(commands[0], terminal_outs);

								break;
							}
						}//end of switch
						
					}
					else if(sock_index == this->server_fd)
					{
						/*check if any new message from server*/
						std::string msgs_recvd;
						this->recv_msg(this->server_fd, msgs_recvd);
						/*Slipt input command string into tokens*/
						std::vector<std::string> commands;
						commands.clear();
						cmd_parser(msgs_recvd.c_str(), commands);

						/*check if command is valid*/
						if(!is_cmd_valid(commands[0]))
						{
							std::cout << commands[0] << " : Invlid command from server!" << std::endl;
							continue;
						}

						/* convert command string to command enum for switching */
						Instructions command_enum = InstructionMap.at(commands[0]);

						/* declare terminal outputs vector */
						std::vector<std::string> terminal_outs;

						/*switch into corresponding commands*/
						switch(command_enum)
						{
							case LOGIN:
							{
								std::cout << "LOGIN ACTIVATED" << std::endl;
								this->login_status = 1;
								/*out put status to terminal*/
								terminal_outs.clear();
								terminal_output_success(commands[0],terminal_outs);
								break;
							}

							case REFRESH:
							{
								// std::cout << "Refresh this from server:" << msgs_recvd << std::endl;
								update_local_clients_map(msgs_recvd);
								break;
							}

							case EXIT:
							{
								this->login_status = 0;
								/*close socket*/
								FD_CLR(sock_index, &master_list);
								close(this->server_fd);
								this->exit_status = 1;

								/*out put status to terminal*/
								terminal_outs.clear();
								terminal_output_success(commands[0],terminal_outs);
								break;
							}

							case SEND:
							{
								/*parse the received messages from client*/
								std::string from_ip;
								from_ip = "msg from:" + commands[1];

								std::string msg_content;
								cmd_sec_msg_parser(msgs_recvd.c_str(), msg_content);
								msg_content = "[msg]:" + msg_content;
								
								/*out put status to terminal*/
								terminal_outs.clear();
								terminal_outs.push_back(from_ip);
								terminal_outs.push_back(msg_content);

								std::string received = "RECEIVED";
								terminal_output_success(received,terminal_outs);
								
								//debug_output(msg_content.c_str());
								break;
							}
						
							case P2PREQ:
							{
								std::string src_ip = commands[1];
								std::string file_name;
								cmd_sec_msg_parser(msgs_recvd.c_str(), file_name);
								
								/*creat socket and start to receive file*/
								this->p2p_receivefile(src_ip, file_name);

								break;
							}
						
							case P2PRES:
							{
								/*parse out the dest port*/
								std::string dest_port_str = commands[1];
								int dest_port = atoi(dest_port_str.c_str());

								/*establish connnection and send file to peer client*/
								this->p2p_sendfile(dest_port, this->p2p_dest_ip, this->p2p_filename);
								
								break;
							}
						}
					}
					else
					{
						std::cerr << "Unkown connection!" << std::endl;						
					}

				}//End of checking socket_index
			}//End of for loop of chcking socket index
		}//End of select positive

	}//end of while loop

	return 1;
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

int ClientHost::connect_to_peer(std::string &dest_ip, int dest_port)
{
	int p2p_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(p2p_fd < 0)
	{
		perror("can not open a socket");
		return -1;
	}
	
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(dest_port);
	// sin.sin_port = htons(60000);
	sin.sin_addr.s_addr = inet_addr(dest_ip.c_str());

	if(connect(p2p_fd, (struct sockaddr *)&sin, sizeof sin) < 0){
		perror("connect to peer client failed");
		close(p2p_fd);
		return -1;
	}
	else
	{
		std::cout << "Connect to peer client Success!" << std::endl;
	}
	
    return p2p_fd;
}

int ClientHost::send_msg(int server_socketfd, const std::string &msg)
{
	const char *msg_cstr = msg.c_str();
	int len = strlen(msg_cstr);

	int sent_size = send(server_socketfd, msg_cstr, len, 0);

	if(sent_size == -1)
	{
		std::cout << "client sent message error!" << std::endl;

		return -1;
	}
	else
	{
		// std::cout << "msg size: " << msg.size() << "sent size: " << sent_size << std::endl;
	}
	
	return 0;
}

int ClientHost::recv_msg(int server_fd, std::string &msgs_recvd)
{
	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);

	if(recv(server_fd, buffer, BUFFER_SIZE, 0) >= 0){
		msgs_recvd = std::string(buffer);
		printf("Server responded: %s\n", buffer);
		fflush(stdout);
	}
	else
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

void ClientHost::update_local_clients_map(std::string clientslist_str)
{
	local_clients_map.clear();
	std::vector<std::string> clients;
	clients = this->split(clientslist_str, ' ');
	std::cout << clients.size() << std::endl;

	for(auto c : clients)
	{
		std::cout << c << std::endl;
	}

	// for(int i = 1; i < clients.size()-1; i++)
	// {
	// 	std::vector<std::string> client_info = this->split(clients[i], '|');
	// 	/*create a new local client*/
	// 	std::cout << client_info.size() << std::endl;
	// 	std::cout << client_info[0] << "\t" << client_info[1] << "\t" <<client_info[2] << std::endl;
	// }

	for(int i = 1; i < clients.size()-1; i++)
	{
		std::vector<std::string> client_info = this->split(clients[i], '|');
		/*create a new local client*/
		Client local_client(NULL,client_info[1],std::stoi(client_info[2]),"logged-in");
		local_client.set_hostname(client_info[0]);
		/*add new local client to local client map*/
		local_clients_map[client_info[1]] = local_client;
	}
}

std::vector<std::string> ClientHost::split(const std::string &text, char sep) {
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while ((end = text.find(sep, start)) != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}

int ClientHost::p2p_sendfile(int dest_port, std::string dest_ip, std::string filename)
{
	/*send file to destination client*/
	int filefd = socket(PF_INET, SOCK_STREAM, 0);
	if(filefd < 0)
	{
		std::cerr << "fail to create a new socket" << std::endl;
		return -1;
	}
	
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(dest_port);

	sin.sin_addr.s_addr = inet_addr(dest_ip.c_str());

	if(connect(filefd, (struct sockaddr *)&sin, sizeof sin) < 0){
		std::cerr << "fail to connect with peer client" << std::endl;
		close(filefd);
		return -1;
	}

	FILE *file_ptr = fopen(filename.c_str(), "rb");
	if (file_ptr == nullptr) 
	{
		std::cerr << "fail to open local file" << std::endl;
		return -1;
	}
	int len;

	char file_send_buffer[1024];
	while ((len = fread(file_send_buffer, sizeof(char), sizeof file_send_buffer, file_ptr)) > 0) 
	{
		if (ferror(file_ptr))
		{
			std::cerr << "fail to open local file" << std::endl;
			fclose(file_ptr);
			close(filefd);
			return -1;
		}
		
		if (send(filefd, file_send_buffer, len, 0) == -1)
		{
			std::cerr << "fail to open local file" << std::endl;
			return -1;
		}
	}

	fclose(file_ptr);
	close(filefd);

	return 0;
}

int ClientHost::p2p_receivefile(std::string src_ip, std::string filename)
{
	/*create a new socket and port for p2p file transfer*/
	struct sockaddr_in peer_addr, my_addr;
	bzero(&peer_addr, sizeof(peer_addr));
	bzero(&my_addr, sizeof(my_addr));
	socklen_t my_addr_len = sizeof my_addr;
	int new_socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if (new_socket_fd == -1) { 
		std::cerr << "fail to create new socket\n" << std::endl; 
		return -1;
	}
	else
	{
		std::cout << "success on creating new socket" << std::endl;
	}
	
	peer_addr.sin_family = AF_INET; 
	peer_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	peer_addr.sin_port = htons(0);

	if ((bind(new_socket_fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr))) != 0) { 
		std::cerr << "fail to bind with port" << std::endl; 
		close(new_socket_fd);
		return -1;
	}
	else
	{
		std::cout << "success on binding with port: " << ntohs(my_addr.sin_port) << std::endl;
	}
	

	if (listen(new_socket_fd, 10) == -1) 
	{
		close(new_socket_fd);
		std::cerr << "failed to listen on the new socket" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "success on listen on the new socket" << std::endl;
	}
	

	getsockname(new_socket_fd, (struct sockaddr *)&my_addr, &my_addr_len);
	
	int localport = ntohs(my_addr.sin_port);

	/*send the new created port to src client*/
	if(send_msg(this->server_fd, ("P2PRES " + src_ip + ' ' + std::to_string(localport) + "$$")) == -1)
	{
		std::cerr << "fail to send P2PRES" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "successfully send P2PRES to server" << std::endl;
	}

	/*accept connection from peer client*/
	struct sockaddr_in src_client_addr;
	socklen_t src_client_addr_len = sizeof src_client_addr;
	int p2p_fd = accept(new_socket_fd, (struct sockaddr *)&src_client_addr, &src_client_addr_len);

	if(p2p_fd < 0){
		std::cerr << "fail to accept connection from peer" << std::endl;
		close(new_socket_fd);
		return -1;
	}

	/* start receiving new file */
	FILE* file_ptr = fopen(filename.c_str(), "wb");

	char file_buffer[1024];

	while(true)
	{
		int recv_len = recv(p2p_fd, file_buffer, sizeof file_buffer, 0);
		if(recv_len <= 0)
		{
			/*Entire file has been fully received*/
			std::cout << "file fully received" << std::endl;
			fclose(file_ptr);
			close(new_socket_fd);
			close(p2p_fd);
			break;
		}
		else
		{
			/*continue to receive file*/
			fwrite (file_buffer , sizeof(char), recv_len, file_ptr);
		}
	}

	return 0;
}