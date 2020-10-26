/**
 * @server
 * @author
 *   	Sai Venigalla 	50338187  	<srisaiya@buffalo.edu>
 * 	    Sen Pan 		50321231 	<senpan@buffalo.edu>
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
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */

#include "server.hpp"
#include "common.hpp"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256

 /**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int server(int port)
{
    /*server messages buffer*/
    char server_buffer[1024];
    int start_post = 0;
    int delimiter_pos = 0;
    int count = 0;
    int total = 0;

	int server_socket, head_socket, selret, sock_index, fdaccept=0;
    socklen_t caddr_len;
	struct sockaddr_in server_addr, client_addr;
	fd_set master_list, watch_list;

	/* Socket */
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
		perror("Cannot create socket");

	/* Fill up sockaddr_in struct */

	bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /* Bind */
    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    	perror("Bind failed");

    /* Listen */
    if(listen(server_socket, BACKLOG) < 0)
    	perror("Unable to listen on port");

    /*--------------------------Initialize Clients List ---------------------------*/
    ClientsList clients_list;

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    
    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;

    while(TRUE){
        memcpy(&watch_list, &master_list, sizeof(master_list));

        //printf("\n[PA1-Server@CSE489/589]$ ");
		//fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
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
                        switch(command_enum)
                        {
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
                                std::string terminal_out = "IP:" + find_external_ip() ;
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
                                clients_list.display_login_clients(terminal_outs);
                                terminal_output_success(commands[0], terminal_outs);
                                break;
                            }

                            case STATISTICS:
                            {
                                terminal_outs.clear();
                                clients_list.display_statistics(terminal_outs);
                                terminal_output_success(commands[0], terminal_outs);
                                break;
                            }

                            case BLOCKED:
                            {
                                // debug

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

                                /*check if ip address exist in local client list*/
                                if(clients_list.get_fd_by_ip(commands[1]) == -1)
                                {
                                    std::cout << "ip address not found in local clients list!" << std::endl;
									terminal_output_fail(commands[1]);
									continue;
                                }

                                terminal_outs.clear();
                                clients_list.display_block_list(commands[1], terminal_outs);
                                terminal_output_success(commands[0], terminal_outs);
                            }
                        }

						free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if(fdaccept < 0)
                            perror("Accept failed.");

						printf("\nRemote Host connected!\n");                        

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket) head_socket = fdaccept;
                        /* Create a client object for the accepted new client*/
                        
                        std::string new_client_ip;
                        std::string new_client_hostname;
                        std::string new_client_status = "logged-in";
                        int new_client_port;

                        get_peer_ip(fdaccept, new_client_ip, new_client_port, new_client_hostname);

                        std::cout << "remote client IP: " << new_client_ip << std::endl;

                        Client client(fdaccept,new_client_ip, new_client_port, new_client_status);
                        /* Add the new client into ClientList */
                        clients_list.add(fdaccept, client);

                    }
                    /* Read from existing clients */
                    else{

                        std::string cmd_str;
                        while((count = recv(sock_index, &server_buffer[total], sizeof(server_buffer) - count, 0)) > 0)
                        {
                            total += count;
                            std::string recvd_msg(server_buffer, total);
                            delimiter_pos = recvd_msg.find("$$");
                            if( delimiter_pos == std::string::npos)
                            {
                                /*message incomplete, continue while loop*/
                                continue;
                            }

                            printf("Server buffer:%s\n", server_buffer);

                            cmd_str = recvd_msg.substr(0,delimiter_pos);

                            /*clear server buffer*/
                            memset(server_buffer, '\0', sizeof(server_buffer));
                            total = 0;
                            delimiter_pos = 0;

                            break;
                        }

                        if(count <= 0)
                        {
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");
                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);

                            continue;
                        }
                                
                        /* whole message received, start handle input*/

                        //Process incoming data from existing clients here ...
                        Client& a = clients_list.get_client_by_fd(sock_index);

                        std::string client_ip = a.get_ip();
                        printf("\nClient sent me: %s\n", cmd_str.c_str());

                        /* parse out the command*/
                        std::vector<std::string> client_msgs;
                        cmd_parser(cmd_str.c_str(), client_msgs);
                        Instructions client_cmd_enum = InstructionMap.at(client_msgs[0]);

                        /* terminal outs*/
                        std::vector<std::string> terminal_outputs;

                        /* swtich to different response according to the command */
                        switch(client_cmd_enum)
                        {
                            case SEND:
                            {
                                std::string dest_ip = client_msgs[1];
                                int dest_fd = clients_list.get_fd_by_ip(dest_ip);
                                std::string payload;
                                cmd_sec_msg_parser(cmd_str.c_str(), payload);
                                // std::cout << "message from" << client_ip << "to:" << dest_ip << ": " << payload <<std::endl;
                                
                                /*check if the source client has been blocked by the dest client */
                                if(!clients_list.get_client_by_fd(dest_fd).is_blocked(client_ip))
                                {
                                    /*if the destination client is loggedin, send the message*/
                                    if(clients_list.get_client_by_fd(dest_fd).get_status() == "logged-in")
                                    {
                                        std::string relayed_msg = "SEND" + ' ' + client_ip + ' ' + payload;

                                        std::string relayed = std::string("RELAYED");
                                        
                                        std::string msg_to_dest = "SEND " + client_ip + ' ' + payload;
                                        if(send_msg(dest_fd, msg_to_dest) != 0)
                                        {
                                            terminal_output_fail(relayed);
                                        }else
                                        {
                                            /*updta statiscis*/
                                            clients_list.get_client_by_fd(sock_index).increment_sent_msgs_count();
                                            clients_list.get_client_by_fd(dest_fd).increment_recvd_msgs_count();

                                            /*terminal output relay event*/
                                            terminal_outputs.clear();
                                            char from_to_buff[256];
                                            int from_to_buff_len = snprintf(from_to_buff, sizeof(from_to_buff), "msg from:%s, to:%s", client_ip.c_str(), dest_ip.c_str());
                                            std::string from_to = std::string(from_to_buff, from_to_buff_len);
                                            std::string msg = "[msg]:" + payload;

                                            terminal_outputs.push_back(from_to);
                                            terminal_outputs.push_back(msg);

                                            terminal_output_success(relayed, terminal_outputs);
                                        } 
                                    }
                                    else if(clients_list.get_client_by_fd(dest_fd).get_status() == "logged-out")
                                    {
                                        std::string buffer_msg = client_ip + ' ' + payload;
                                        /* the client is logged out, buffer the messages */
                                        clients_list.get_client_by_fd(dest_fd).add_buffer_msgs(buffer_msg);
                                    }
                                }

                                break;
                            }

                            case SENDHSTNAM:
                            {
                                Client& c = clients_list.get_client_by_fd(sock_index);
                                c.set_hostname(client_msgs[1]);
                                std::cout << "Received and updated client hostname!" << std::endl;
                                std::cout << c.get_hostname() << std::endl;

                                /*send the updated clients list back to client*/
                                std::string login_clients = "REFRESH " + clients_list.get_clientslist_str();
                                send_msg(sock_index, login_clients);
                                std::cout << "Sent updated clients list to new client!" << std::endl;

                                break;
                            }

                            case BROADCAST:
                            {
                                /*parse out the payload message*/
                                std::string payload;
                                cmd_first_msg_parser(cmd_str.c_str(), payload);

                                std::string relayed = std::string("RELAYED");

                                /*iterate through each client in the clientlist*/
                                for(auto it : clients_list.get_clientslist())
                                {
                                    /*check if the source client has been blocked by the dest client */
                                    if(!it.second.is_blocked(client_ip))
                                    {
                                        if((it.second.get_status() == "logged-in") && (it.first != sock_index))
                                        {
                                            /*broadcast message to this client*/

                                            std::string relayed_msg = "SEND" + ' ' + client_ip + ' ' + payload;                                    
                                            std::string msg_to_dest = "SEND " + client_ip + ' ' + payload;
                                            if(send_msg(it.first, msg_to_dest) != 0)
                                            {
                                                terminal_output_fail(relayed);
                                            }

                                            /*update statistics for receiver*/
                                            clients_list.get_client_by_fd(it.first).increment_recvd_msgs_count();
                                        }
                                        else if(it.second.get_status() == "logged-out")
                                        {
                                            /*buffer message to this client*/
                                            std::string buffer_msg = client_ip + ' ' + payload;
                                            /* the client is logged out, buffer the messages */
                                            clients_list.get_client_by_fd(it.first).add_buffer_msgs(buffer_msg);

                                            /*update statistics for receiver*/
                                            clients_list.get_client_by_fd(it.first).increment_recvd_msgs_count();
                                        }
                                    }
                                }

                                /*update statistics for sender*/
                                clients_list.get_client_by_fd(sock_index).increment_sent_msgs_count();

                                    /*terminal out success*/
                                terminal_outputs.clear();
                                char from_to_buff[256];
                                std::string broadcast_ip = "255.255.255.255";
                                int from_to_buff_len = snprintf(from_to_buff, sizeof(from_to_buff), "msg from:%s, to:%s", client_ip.c_str(), broadcast_ip.c_str());
                                std::string from_to = std::string(from_to_buff, from_to_buff_len);
                                std::string msg = "[msg]:" + payload;

                                terminal_outputs.push_back(from_to);
                                terminal_outputs.push_back(msg);

                                terminal_output_success(relayed, terminal_outputs);
                                
                                break;
                            }

                            case BLOCK:
                            {
                                clients_list.get_client_by_fd(sock_index).add_to_block_list(client_msgs[1]);

                                break;
                            }

                            case UNBLOCK:
                            {
                                clients_list.get_client_by_fd(sock_index).remove_from_block_list(client_msgs[1]);
                                
                                break;
                            }

                            case REFRESH:
                            {
                                std::string login_clients = "REFRESH " + clients_list.get_clientslist_str();
                                send_msg(sock_index, login_clients);

                                break;
                            }

                            case LOGIN:
                            {
                                Client& c = clients_list.get_client_by_fd(sock_index);
                                c.set_status("logged-in");

                                std::cout << "set client status to logged-in" << std::endl;

                                /*check if any message in the buffer to send after the client re-login*/
                                std::vector<std::string> buffer_msgs;
                                c.get_buffer_msgs(buffer_msgs);

                                std::cout << buffer_msgs.empty() << std::endl;

                                if(!buffer_msgs.empty())
                                {
                                    for(auto msg : buffer_msgs)
                                    {
                                        send_msg(sock_index,"SEND " + msg);
                                        /*terminal out put relay success*/
                                        std::vector<std::string> buffer_tokens;
                                        cmd_parser(msg.c_str(), buffer_tokens);
                                        std::string from_ip = buffer_tokens[0];
                                        std::string to_ip = client_ip;
                                        std::string buffer_msg;
                                        cmd_first_msg_parser(msg.c_str(), buffer_msg);
                                        
                                        /*terminal out success*/
                                        terminal_outputs.clear();
                                        char from_to_buff[256];
                                        int from_to_buff_len = snprintf(from_to_buff, sizeof(from_to_buff), "msg from:%s, to:%s", from_ip.c_str(), to_ip.c_str());
                                        std::string from_to = std::string(from_to_buff, from_to_buff_len);
                                        std::string msg_body = "[msg]:" + buffer_msg;

                                        terminal_outputs.push_back(from_to);
                                        terminal_outputs.push_back(msg_body);

                                        std::string relayed = std::string("RELAYED");
                                        terminal_output_success(relayed, terminal_outputs);

                                        usleep(100000);
                                    }

                                    /*clear the message in the buffer*/
                                    c.clear_buffer_msgs();

                                    std::cout<<"cleared buffer message"<<std::endl;
                                }
                                /*inform the client login success*/
                                std::cout << "before sending the client LOGIN" << std::endl;
                                usleep(100000);
                                send_msg(sock_index,"LOGIN");
                                std::cout<<"sent LOGIN to client"<<std::endl;

                                break;
                            }

                            case LOGOUT:
                            {
                                Client& c = clients_list.get_client_by_fd(sock_index);
                                c.set_status("logged-out");
                                break;
                            }

                            case EXIT:
                            {
                                /*inform clietn*/
                                send_msg(sock_index, "EXIT");
                                /*remove from clients list*/
                                clients_list.remove(sock_index);
                                /*remove from master fd list*/
                                FD_CLR(sock_index, &master_list);

                                break;
                            }

                            case P2PREQ:
                            {
                                /*parse dest ip*/
                                std::string dest_ip = client_msgs[1];
                                int dest_fd = clients_list.get_fd_by_ip(dest_ip);

                                /*parse file name*/
                                std::string file_name;
                                cmd_sec_msg_parser(cmd_str.c_str(), file_name);

                                /*find src ip*/
                                std::string src_ip = clients_list.get_client_by_fd(sock_index).get_ip();

                                send_msg( dest_fd, "P2PREQ " + src_ip + ' ' +  file_name);

                                break;
                            }
 
                            case P2PRES:
                            {
                                /*parse the src ip*/
                                std::string src_ip = client_msgs[1];
                                int src_fd = clients_list.get_fd_by_ip(src_ip);

                                /*parse the dest port*/
                                std::string dest_port = client_msgs[2];

                                send_msg( src_fd, "P2PRES " + dest_port);
                                break;
                            }
                        }

                        // free(client_msg_buff);
                    }//End of reading from existing client;
                }//End of checking socket_index
            }//End of for loop of chcking socket index
        }//End of select positive
    }//End of while loop

    return 0;
}