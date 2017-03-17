/*
 * IPK.2015L
 *
 * Demonstration of trivial TCP communication.
 *
 * Ondrej Rysavy (rysavy@fit.vutbr.cz)
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


int main (int argc, const char * argv[]) {

	int client_socket, port_number, bytestx, bytesrx;
    socklen_t serverlen;
    const char *server_hostname;
    struct hostent *server;
    struct sockaddr_in server_address;
    //char *buf;//[BUFSIZE];
     
    /* 1. test vstupnich parametru: */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    server_hostname = argv[1];
    port_number = atoi(argv[2]);
    
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
	    
    /* nacteni zpravy od uzivatele */
    //bzero(buf, BUFSIZE);
    //printf("Please enter msg: ");

    string command ="mkd ";
    // TODO convert input like this to remote REST address - name:port/user/dir
    string remote_path ="http://localhost:12345/tonda/foo/bar";

    // TODO parse local_path
    cout << "command           : " << command << endl ;
    cout << "rem path          : " << command << endl ;

    /*
     * create http header
     */
    string rest_command = string("PUT /Users/majko/Desktop/foo/daco?type=folder HTTP/1.1 ") + "\r\n";
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


    cout << "" << endl;

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
		perror("ERROR: connect");
		exit(EXIT_FAILURE);        
    }

    cout << "pred sendom" << endl;
    /* odeslani zpravy na server */
    //cout << buf << endl;
    bytestx = send(client_socket, buf, strlen(buf), 0);

    cout << "send = " << bytestx << endl;

    while (bytesrx = recv(client_socket, buf, BUFSIZE, 0) > 0) {

/*        if (bytestx < 0) {
            perror("ERROR in sendto");
        }*/
        /* prijeti odpovedi a jeji vypsani */

        if (bytesrx < 0) {
            perror("ERROR in recvfrom");
            break;
        }
    }


      
    printf("Echo from server: %s", buf);
        
    close(client_socket);
    return 0;
}

