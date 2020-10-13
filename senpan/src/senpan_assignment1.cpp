/**
 * @senpan_assignment1
 * @author  SEN PAN <senpan@buffalo.edu>
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
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>

#include "../include/logger.h"
#include "../include/server.hpp"
#include "../include/client.hpp"
#include "common.hpp"

using namespace std;

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

	/*Start Here*/

	/*-----------------------Check input format-----------------------------*/
	if(argc != 3)
	{
		std::cout << "Invalid input. Input should be in below format:\n" << argv[0] << "   [c/s]    port" << std::endl;
		return 0;
	}
	
	int local_port = atoi(argv[2]);

	/*-----------------------Check port number-----------------------------*/
	if(!is_port_valid(local_port))
	{
		std::cout << "invalid port" << std::endl;
		return 0;
	}

	if (strncmp(argv[1],"s",1) == 0)
	{
		server(local_port);
	}else if(strncmp(argv[1],"c",1) == 0)
	{
		ClientHost *client = new ClientHost(local_port);
		client->client_start();
	}else
	{
		std::cout << "invalid parameter for [c/s]" << std::endl;
	}
	
	return 0;
}
