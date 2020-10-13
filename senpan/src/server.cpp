/**
 * @server
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
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
    ClientsList clints_list;

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
                        input_parser(cmd, commands);

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
                                std::string terminal_out = "I,senpan, have read and understood the course academic integrity policy.";
                                terminal_outs.push_back(terminal_out);
                                terminal_output_success(commands[0], terminal_outs);
                                break;
                            }

                            case IP:
                            {
                                terminal_outs.clear();
                                std::string terminal_out = find_external_ip();
                                terminal_outs.push_back(terminal_out);
                                terminal_output_success(commands[0], terminal_outs);
                                break;
                            }

                            case PORT:
                            {
                                terminal_outs.clear();
                                std::string terminal_out = std::to_string(port);
                                terminal_outs.push_back(terminal_out);
                                terminal_output_success(commands[0], terminal_outs);
                                break;
                            }
                            
                            case LIST:
                            {
                                terminal_outs.clear();
                                clints_list.display(terminal_outs);
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

                        Client client(fdaccept,new_client_ip, new_client_hostname, new_client_port, new_client_status);
                        /* Add the new client into ClientList */
                        clints_list.add(fdaccept, client);
                    }
                    /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...

                        	printf("\nClient sent me: %s\n", buffer);
							printf("ECHOing it back to the remote host ... ");
							if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
								printf("Done!\n");
							fflush(stdout);
                        }

                        free(buffer);
                    }//End of reading from existing client;
                }//End of checking socket_index
            }//End of for loop of chcking socket index
        }//End of select positive
    }//End of while loop

    return 0;
}