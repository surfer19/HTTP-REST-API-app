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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <dirent.h>
#include <string>
#include <vector>
#include <sys/stat.h>

using namespace std;

int makeDir(string rest_path);
int removeDir(string rest_path);
int lsDir(string rest_path);

void parseClientHeader(char *buff);


vector<string> split_string(const string& str, const string& delimiter);


int main (int argc, const char * argv[]) {
	int rc;
	int welcome_socket;
	struct sockaddr_in6 sa;
	struct sockaddr_in6 sa_client;
	char str[INET6_ADDRSTRLEN];
    int port_number;
    
    if (argc != 2) {
       fprintf(stderr,"usage: %s <port>\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    port_number = atoi(argv[1]);
    
    
	socklen_t sa_client_len=sizeof(sa_client);
	if ((welcome_socket = socket(PF_INET6, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR: socket");
		exit(EXIT_FAILURE);
	}
	
	memset(&sa,0,sizeof(sa));
	sa.sin6_family = AF_INET6;
	sa.sin6_addr = in6addr_any;
	sa.sin6_port = htons(port_number);	
        
    
    // todo why it crash with 'using namespace std;' ??
	if ((::bind(welcome_socket, (struct sockaddr*)&sa, sizeof(sa))) < 0)
	{
		perror("ERROR: bind");
		exit(EXIT_FAILURE);
	}
	if ((listen(welcome_socket, 1)) < 0)
	{
		perror("ERROR: listen");
		exit(EXIT_FAILURE);				
	}
	while(1)
	{
		int comm_socket = accept(welcome_socket, (struct sockaddr*)&sa_client, &sa_client_len);		
		if (comm_socket > 0)
		{
			if(inet_ntop(AF_INET6, &sa_client.sin6_addr, str, sizeof(str))) {
				printf("INFO: New connection:\n");
				printf("INFO: Client address is %s\n", str);
				printf("INFO: Client port is %d\n", ntohs(sa_client.sin6_port));
			}

			char buff[1024];
			int res = 0;

			string ahoj = "";
			//while ((res = recv(comm_socket, buff, 1024,0)) > 0) {
				res = recv(comm_socket, buff, 1024,0);
//				cout << "[SERVER] response" << res << endl;
				ahoj.append(buff);
				cout << "cycle " << ahoj << "\n";
//				if (res <= 0)
//					break;
			//}

			cout << "po cykle " << ahoj << endl;

			parseClientHeader(buff);

			send(comm_socket, buff, strlen(buff), 0);

		}
		else
		{
			printf(".");
		}
	printf("Connection to %s closed\n",str);
		close(comm_socket);
	}	
}

void parseClientHeader(char *buff){
	cout << "[SERVER] toto parsujem" << endl;

	// convert char * to string
	string Str = string(buff);
	//cout << Str;

	// create list of header attributes (vector)
	vector<string> list_header_attributes = {};

	// push each splited value as one item of vector
	auto strings = split_string(Str, "\n");
	int i = 1;
	for (auto itr = strings.begin(); itr != strings.end(); itr++){
		list_header_attributes.push_back(*itr);
	}
	// print attributes
	//for (auto i = list_header_attributes.begin(); i != list_header_attributes.end(); ++i)
	//	cout << *i << ' ';

	string rest_command = list_header_attributes.at(0);
	string host = list_header_attributes.at(1);
	string date = list_header_attributes.at(2);
	string accept = list_header_attributes.at(3);
	string accept_encoding = list_header_attributes.at(4);
	string content_type = list_header_attributes.at(5);
	string content_length = list_header_attributes.at(6);

	// get type of rest request
	string delimiter = " ";
	string rest_type = rest_command.substr(0, rest_command.find(delimiter)); // rest type is "GET/PUT..."
	// delete type from string
	int length_rest_type = rest_type.length();
	rest_command.erase(0,length_rest_type + 1); // +1 because we want erase white space too

	int length_rest_path;
	int create_folder;
	int delete_folder;
	int get_folder;
	string rest_path;
	string delimiter2;

	// get folder!
	if (rest_command.find("folder")){
		cout << "REST COMMAND ITS FOLDER!" << endl;

		if(rest_type == "PUT"){
			create_folder = 1;
		}
		else if(rest_type == "DELETE"){
			delete_folder = 1;
		}
		else if(rest_type == "GET"){
			get_folder = 1;
		}

		// get path
		delimiter2 = "?";
		rest_path = rest_command.substr(0, rest_command.find(delimiter2)); // rest path to folder

		// delete path from string
		length_rest_path = rest_path.length();
		rest_command.erase(0,length_rest_path + 1); // +1 because we want erase '?' too

	}
	else {
		cout << "REST COMMAND ITS FILE!" << endl;
	}

	/*
	 *
	 * Start doing command
	 *
	 */
	int ret_code_mkd;
	int ret_code_rmd;

	if (create_folder == 1){

		ret_code_mkd = makeDir(rest_path);

		if (ret_code_mkd == 0){
			cout << "successfull CREATE DIR" << endl;
		}
		else if (ret_code_mkd == 2 ){
			cout << "Already exists." << endl;
		}
		// todo not in wis? :/
		else if (ret_code_mkd == 1){
			cout << "Directory not found." << endl;
		}
		else {
			cout << "Unknown error." << endl;
		}
	}

	if (delete_folder == 1){

		ret_code_rmd = removeDir(rest_path);

		if (ret_code_rmd == 0){
			cout << "successfull REMOVE DIR" << endl;
		}
		else if (ret_code_rmd == 1){
			cout << "Directory not empty." << endl;
		}
		else if (ret_code_rmd == 2) {
			cout << "Directory not found." << endl;
		}
		else if (ret_code_rmd == 3){
			cout << "Unknown error." << endl;
		}
	}

	if (get_folder == 1) {

		lsDir(rest_path);

	}

	cout << "[SERVER] end parse";
}

int lsDir(string rest_path){

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(rest_path.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			printf ("%s\n", ent->d_name);
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("");
		return -1;
	}
}

int makeDir(string rest_path){

	mkdir(rest_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	// parent doesnt exist
	if (errno == ENOENT){
		return 1;
	}
	// The named file exists.
	else if (errno == EEXIST){
		return 2;
	}
	return 0;
}


int removeDir(string rest_path){
	int status;
	status = rmdir(rest_path.c_str());
	// Directory not found.
	// Directory not empty. ENOTEMPTY

	// its ok
	if (status == 0){
		return 0;
	}
	// Directory not empty.
	else if (errno == ENOTEMPTY){
		return 1;
	}
	// A component of path does not name an existing file, or the
	// path argument names a nonexistent directory or points to an empty string.
	else if (errno == ENOENT){
		return 2;
	}
	//Unknown error.
	else {
		return 3;
	}
}

vector<string> split_string(const string& str, const string& delimiter)
{
	vector<string> strings;

	string::size_type pos = 0;
	string::size_type prev = 0;
	while ((pos = str.find(delimiter, prev)) != string::npos)
	{
		strings.push_back(str.substr(prev, pos - prev));
		prev = pos + 1;
	}


	// To get the last substring (or only, if delimiter is not found)
	strings.push_back(str.substr(prev));

	return strings;
}
