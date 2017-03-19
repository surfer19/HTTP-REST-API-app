/*
 * IPK.2017
 *
 * TCP communication with REST API.
 *
 * Marián Mrva (xmrvam01)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <string.h>
#include <time.h>

using namespace std;

#define BUFSIZE 1024

// arg 1 - string
// arg 2 - find substr
int isInString(string s1, string s2){
    if (s1.find(s2) != string::npos) {
        return 1;
    }
    return -1;
}

const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%A, %d %b %Y %X %Z", &tstruct);

    return buf;
}
// todo ip port

int main (int argc, const char * argv[]) {

	int client_socket, port_number, bytestx, bytesrx;
    socklen_t serverlen;
    const char *server_hostname;
    struct hostent *server;
    struct sockaddr_in server_address;
    //char *buf;//[BUFSIZE];

    /* 1. test vstupnich parametru: */
    if (argc != 3) {
        fprintf(stderr, "Unknown error.\n");
        exit(EXIT_FAILURE);
    }

    /*
     *
     *  parse args
     */
    const char * command  = argv[1];
    const char * all_path = argv[2];
    string s_command = command;
    string s_path = all_path;
    cout <<  "command = " <<  command << endl;
    cout <<  "path = " << all_path << endl;
    string serv_hostname;
    string serv_port = "";
    string serv_path;

    s_path.erase(0,7); // delete first 7 chars http://

    string delimiter = ":";
    serv_hostname = s_path.substr(0, s_path.find(delimiter)); // get ip/localhost

    // delete ip/localhost from path
    s_path.erase(0,serv_hostname.length() +1); // +1 for delete ':'

    // get port
    string delimiter2 = "/";
    serv_port = s_path.substr(0, s_path.find(delimiter2)); // get ip/localhost

    // delete port
    s_path.erase(0,serv_port.length());

    // just path stay here now
    serv_path = s_path;

    server_hostname = strdup(serv_hostname.c_str());
    port_number = stoi(serv_port);

    cout << "final hostname = " << server_hostname << endl;
    cout << "final port = " << port_number << endl;

    // todo default port

    /*
     *
     *  convert client command to server command
     *
     */
    string final_command;
   if (s_command == "del" || s_command == "rmd") {
       final_command = "DELETE";
   }
   else if (s_command == "get" || s_command == "lst") {
       final_command = "GET";
   }
   else if (s_command == "put" || s_command == "mkd") {
       final_command = "PUT";
   }
   else {
       fprintf(stderr,"Unknown error.%s \n");
   }
    string type_of_medium;
    // its file or folder ?
    if (isInString(s_path, ".") != -1){
        // its file
        type_of_medium = "file";
    }
    else {
        type_of_medium = "folder";
    }

    
    /* 2. ziskani adresy serveru pomoci DNS */
    
    if ((server = gethostbyname(server_hostname)) == NULL) {
        fprintf(stderr,"ERROR: no such host as %s\n", server_hostname);
        exit(EXIT_FAILURE);
    }
    
    /* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);
   
    /* tiskne informace o vzdalenem soketu */ 
    printf("INFO: Server socket: %s : %d \n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
    
    /* Vytvoreni soketu */
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		perror("ERROR: socket");
		exit(EXIT_FAILURE);
	}

    string remote_path ="http://localhost:12345/tonda/foo/bar";
    /*
     * create http header
     */
    // DELETE /Users/majko/workspace/HTTP-REST-API-app/client.cpp
    string rest_command = final_command + " " + serv_path + "?type=" + type_of_medium + string(" HTTP/1.1 ") + " \r\n ";
    string host = string("Host: ") + inet_ntoa(server_address.sin_addr) + " \r\n ";
    string date = "Date: " + currentDateTime() + " \r\n ";
    string accept = string("Accept: application/json") + " \r\n ";
    string accept_encoding = string("Accept-Encoding: identity") + " \r\n ";
    string content_type = string("Content-Type: application/octet-stream") + " \r\n ";
    string content_length = string("Content-Length: 12345") + " \r\n ";

    /*
     *  concatenate header and convert to char *
     */

    string final_header = rest_command + host + date + accept + accept_encoding + content_type + content_length;

    //    Use strdup() to copy the const char * returned by c_str()
    //    into a char * TODO (remember to free() it afterwards)
    //    fgets(buf, BUFSIZE, stdin);
    char *buf = strdup(final_header.c_str());

    cout << "------- header --------" << endl;
    cout << final_header;


    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
		perror("ERROR: connect");
		exit(EXIT_FAILURE);        
    }

    /* odeslani zpravy na server */
    bytestx = send(client_socket, buf, strlen(buf), 0);
    // send err
    if (bytestx < 0) {
        perror("ERROR in sendto");
    }

    //cout << "send = " << bytestx << endl;

    while (bytesrx = recv(client_socket, buf, BUFSIZE, 0) > 0) {

        if (bytesrx < 0) {
            perror("ERROR in recvfrom");
            break;
        }
    }

    printf("[CLIENT] RESPONSE from server: %s \n", buf);

    string s_header = string(buf);
    string s_helper = "";
    string s_error = "";

    if (isInString(s_header, "ERR") != -1) {
        // its err
        // get error string
        string delimiter = "ERR";
        s_helper = s_header.substr(0, s_header.find(delimiter)); // get all chars before 'ERR:'
        s_header.erase(0,s_helper.length());

        s_header.erase(0, 4); // rm 'ERR:'

        s_error = s_header;
        fprintf(stderr, "%s \n", s_error.c_str());

    }
        
    close(client_socket);
    return 0;
}

