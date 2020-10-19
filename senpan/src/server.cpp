/**
 * @server
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
                        std::string new_client_status = "LOGIN";
                        int new_client_port;

                        get_peer_ip(fdaccept, new_client_ip, new_client_port, new_client_hostname);

                        std::cout << "remote client IP: " << new_client_ip << std::endl;

                        Client client(fdaccept,new_client_ip, new_client_port, new_client_status);
                        /* Add the new client into ClientList */
                        clients_list.add(fdaccept, client);

                        // /* send the ClientList to the new client */
                        // std::string clientlists_str = clients_list.get_clientslist_str();
                        // // char *clientslist_buff = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        // // memset(clientslist_buff, '\0', BUFFER_SIZE);
                        // const char * clientslist_buff = clientlists_str.c_str();
                        // printf("Send clients list to new client ... \n");
                        // if(send(fdaccept, clientslist_buff, strlen(clientslist_buff), 0) == strlen(clientslist_buff))
                        //     printf("Done!\n");
                    }
                    /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *client_msg_buff = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(client_msg_buff, '\0', BUFFER_SIZE);

                        if(recv(sock_index, client_msg_buff, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");
                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...
                            Client& a = clients_list.get_client_by_fd(sock_index);

                            std::string client_ip = a.get_ip();
                        	printf("\nClient sent me: %s\n", client_msg_buff);

                            /* parse out the command*/
                            std::vector<std::string> client_msgs;
                            cmd_parser(client_msg_buff, client_msgs);
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
                                    cmd_sec_msg_parser(client_msg_buff, payload);
                                    std::cout << "message from" << client_ip << "to:" << dest_ip << ": " << payload <<std::endl;
                                    
                                    /*if the destination client is loggedin, send the message*/
                                    if(clients_list.get_client_by_fd(dest_fd).get_status() == "LOGIN")
                                    {
                                        std::string relayed_msg = "SEND" + ' ' + client_ip + ' ' + payload;

                                        std::string relayed = std::string("RELAYED");
                                        
                                        std::string msg_to_dest = "SEND " + client_ip + ' ' + payload;
                                        if(send_msg(dest_fd, msg_to_dest) != 0)
                                        {
                                            terminal_output_fail(relayed);
                                        }else
                                        {
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
                                    else
                                    {
                                        std::string buffer_msg = client_ip + ' ' + payload;
                                        /* the client is logged out, buffer the messages */
                                        clients_list.get_client_by_fd(dest_fd).add_buffer_msgs(buffer_msg);
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

                                case BLOCK:
                                {

                                    break;
                                }

                                case UNBLOCK:
                                {
                                    break;
                                }

                                case REFRESH:
                                {
                                    std::string login_clients = "REFRESH " + clients_list.get_clientslist_str();
                                    send_msg(sock_index, login_clients);

                                    break;
                                }

                                case LOGOUT:
                                {
                                    Client& c = clients_list.get_client_by_fd(sock_index);
                                    c.set_status("LOGOUT");
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
                            }

                        }

                        free(client_msg_buff);
                    }//End of reading from existing client;
                }//End of checking socket_index
            }//End of for loop of chcking socket index
        }//End of select positive
    }//End of while loop

    return 0;
}