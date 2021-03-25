# computer-networks-projects---chatting-app
* This is a course project for modern comeputer network. 
* The project need to implement the client and server components of a text chat application, 
  consists of on chat server and multiple chat clients over TCP connections. 
* The app achieves message sending, broadcasting, buffering, statistics and P2P file transfer.
* The app is build with vanilla Socket and C++, no other library is used.


### To build:
1. git clone
2. cd senpan
3. cmake ..
4. make

### To Run the server side:
  run ./assignment1 [s] [port]
  
### To Run the client side:
  run ./assignment1 [c] [port]
 
### Commands of the app:
| Command       | Parameters    | Comments |
| ------------- | ------------- | ------------- |
| IP            | NA            |Print the IP address of this process |
| PORT            | NA            |Print the port number this process is listening on |
| LIST            | NA            |Display a numbered list of all the currently logged-in clients |
| STATISTICS           | NA            |Display a numbered list of all the clients that have ever logged-in to the server |
| BLOCKED           | client-ip  |Display a numbered list of all the clients blocked by the client with ip address |
| LOGIN            | server-ip server-port  |This command is used by a client to login to the server located at ip address |
| LOGOUT            | NA  |Logout from the server |
| REFRESH            | client-ip msg          |Get an updated list of currently logged-in clients from the server |
| SEND            | NA            |Send message to client with ip address client-ip|
| BROADCAST            | msg           |Send message to all logged-in clients |
| BLOCK            | client-ip   |Block all incoming messages from the client with IP address client-ip |
| UNBLOCK            | client-ip  |Unblock a previously blocked client with IP address client-ip |
| SENDFILE            | client-ip file  |Directly transfers file between two clients |
| EXIT            | NA |Logout from the server and terminate the application |

### Example:
<img src="https://github.com/gregsensem/camera-calibration/raw/master/example.png" width="50%" height="50%">
