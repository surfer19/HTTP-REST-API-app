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
string lsDir(string rest_path);
int rmFile(string file_path);
string setServerHttpHeader(int response, string data, string error_string);
const string currentDateTime();

void parseClientHeader(char *buff, int comm_socket, string root_path);
vector<string> split_string(const string& str, const string& delimiter);
int isInString(string s1, string s2);


int main (int argc, const char * argv[]) {
	int welcome_socket;
	struct sockaddr_in6 sa;
	struct sockaddr_in6 sa_client;
	char str[INET6_ADDRSTRLEN];
    int port_number;

    string root_path = "";

	if (argc == 1) {
		port_number = 6677;
		root_path = "";
	}
	else if (argc == 3){
		// have some root path on first place
		if (string(argv[1]) == "-r"){
			port_number = 6677;
			root_path = argv[2];
		}
		else if(string(argv[1]) == "-p") {
			port_number = atoi(argv[2]);
			root_path = "";
		}
		else {
			fprintf(stderr, "Unknown error.\n");
			//exit(//exit_FAILURE);
		}
	}
	else if (argc == 5){
		root_path = argv[2];
		port_number = atoi(argv[4]);
	}
	else {
		fprintf(stderr, "Unknown error.\n");
		//exit(//exit_FAILURE);
	}

	char *cstr = &root_path.back();
	if (strcmp(cstr, "/") == 0) {
		root_path.erase(root_path.end() - 1);
	}

	socklen_t sa_client_len=sizeof(sa_client);
	if ((welcome_socket = socket(PF_INET6, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR: socket");
		//exit(//exit_FAILURE);
	}

	memset(&sa,0,sizeof(sa));
	sa.sin6_family = AF_INET6;
	sa.sin6_addr = in6addr_any;
	sa.sin6_port = htons(port_number);

	if ((::bind(welcome_socket, (struct sockaddr*)&sa, sizeof(sa))) < 0)
	{
		perror("ERROR: bind");
		//exit(//exit_FAILURE);
	}
	if ((listen(welcome_socket, 1)) < 0)
	{
		perror("ERROR: listen");
		//exit(EXIT_FAILURE);
	}
	while(1)
	{
		int comm_socket = accept(welcome_socket, (struct sockaddr*)&sa_client, &sa_client_len);
		if (comm_socket > 0)
		{
			inet_ntop(AF_INET6, &sa_client.sin6_addr, str, sizeof(str));
//				printf("INFO: New connection:\n");
//				printf("INFO: Client address is %s\n", str);
//				printf("INFO: Client port is %d\n", ntohs(sa_client.sin6_port));
//			}

			char buff[1024];
			recv(comm_socket, buff, 1024,0);
			parseClientHeader(buff, comm_socket, root_path);
		}

		//fprintf(stderr, "Connection to closed\n");
		close(comm_socket);
	}
}

void parseClientHeader(char *buff, int comm_socket, string root_path){
	//cout << "[SERVER] toto parsujem" << endl;

	// convert char * to string
	string Str = string(buff);
	//cout << Str;

	// create list of header attributes (vector)
	vector<string> list_header_attributes = {};

	// push each splited value as one item of vector
	auto strings = split_string(Str, "\n");

	for (auto itr = strings.begin(); itr != strings.end(); itr++){
		list_header_attributes.push_back(*itr);
	}

	string rest_command = list_header_attributes.at(0);
	string host = list_header_attributes.at(1);
	string date = list_header_attributes.at(2);
	/*string accept = list_header_attributes.at(3);
	string accept_encoding = list_header_attributes.at(4);
	string content_type = list_header_attributes.at(5);
	string content_length = list_header_attributes.at(6);*/

	// get type of rest request
	string delimiter = " ";
	string rest_type = rest_command.substr(0, rest_command.find(delimiter)); // rest type is "GET/PUT..."
	// delete type from string
	int length_rest_type = rest_type.length();
	rest_command.erase(0,length_rest_type + 1); // +1 because we want erase white space too

	// buffer for send data to client
	char server_buff[1024];

	int response = 0;
	int ret_code_del_file;
	int length_rest_path;
	int create_folder = 0;
	int delete_folder = 0;
	int get_folder = 0;
	int delete_file = 0;
	string rest_path;
	string delimiter2;

	// get folder!
	if (isInString(rest_command, "folder") != -1){
		//cout << "REST COMMAND ITS FOLDER!" << endl;

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
		string rest_tmp = rest_path;

		if (root_path != ""){
			root_path += rest_path;
			rest_path = root_path;
		}

		// delete path from string
		length_rest_path = rest_tmp.length();
		rest_command.erase(0,length_rest_path + 1); // +1 because we want erase '?' too

	}
	else if (isInString(rest_command, "file")) {
		//cout << "REST COMMAND ITS FILE!" << endl;

		if (rest_type == "DELETE"){
			delete_file = 1;
		}

		// get path
		delimiter2 = "?";
		rest_path = rest_command.substr(0, rest_command.find(delimiter2)); // rest path to folder
		string rest_tmp = rest_path;

		if (root_path != ""){
			root_path += rest_path;
			rest_path = root_path;
		}

		// delete path from string
		length_rest_path = rest_tmp.length();
		rest_command.erase(0,length_rest_path + 1); // +1 because we want erase '?' too

	}

	/*
	 *
	 * Start doing command
	 *
	 */
	int ret_code_mkd;
	int ret_code_rmd;
	string error_string = "";
	string ls_string = "";

	if (create_folder == 1){
		//cout << "dir for make !! = " << rest_path << endl;
		ret_code_mkd = makeDir(rest_path);

		if (ret_code_mkd == 0){
			//cout << "successfull CREATE DIR" << endl;
			response = 200;
		}
		else if (ret_code_mkd == 2 ){
			error_string = "Already exists.";
			response = 400;
		}
		else if (ret_code_mkd == 1){
			error_string = "Directory not found.";
			response = 404;
		}
		else {
			response = 200;
		}

		string str_header = setServerHttpHeader(response, "", error_string);
		strcpy(server_buff, strdup(str_header.c_str()));
		send(comm_socket, server_buff, 1024 , 0);//strlen(server_buff)
	}

	if (delete_folder == 1){
		//cout << "remove dir = " << rest_path << endl;
		ret_code_rmd = removeDir(rest_path);

		if (ret_code_rmd == 0){
			//cout << "successfull REMOVE DIR" << endl;
			response = 200;
		}
		else if (ret_code_rmd == 1){
			error_string = "Directory not empty.";
			response = 400;
		}
		else if (ret_code_rmd == 2) {
			error_string = "Directory not found.";
			response = 404;
		}
		else if (ret_code_rmd == 3){
			error_string = "Unknown error.";
			response = 400;
		}
		string str_header = setServerHttpHeader(response, "", error_string);
		strcpy(server_buff, strdup(str_header.c_str()));
		send(comm_socket, server_buff, 1024 , 0);//strlen(server_buff)
	}

	if (get_folder == 1) {
		ls_string = lsDir(rest_path);

		if (ls_string == "1"){
			error_string = "Directory not found.";
			response = 404;
		}
		else {
			response = 200;
		}

		// clear buff and set new header
		// push data after header
		string str_header = setServerHttpHeader(response, ls_string, error_string);
		//server_buff = strdup(str_header.c_str());
		strcpy(server_buff, strdup(str_header.c_str()));
		send(comm_socket, server_buff, strlen(server_buff) , 0);//strlen(server_buff)
	}

	if (delete_file == 1){
		// 0 ok
		// 1 error
		ret_code_del_file = rmFile(rest_path);
		if (ret_code_del_file == 0){
			response = 200;
		}
		else {
			//fprintf(stderr, "%s", "File not found.\n");
			error_string = "File not found.";
			response = 404;
		}

		// clear buff and set new header
		// push data after header
		string str_header = setServerHttpHeader(response, "", error_string);
		//server_buff = strdup(str_header.c_str());
		strcpy(server_buff, strdup(str_header.c_str()));
		send(comm_socket, server_buff, strlen(server_buff) , 0);//strlen(server_buff)
	}
}

string setServerHttpHeader(int response, string data, string error_string) {
//	200 OK - operace byla provedena úsopěšně
//	404 Not Found - objekt (soubor/adresář) v požadavku neexistuje
//	400 Bad Request - při přístupu k objektu jiného typu než uvedeného v požadaku
//	(požadavek na operaci nad souborem, ale REMOTE-PATH ukazuje na adresář)

	string response_status;

	switch (response){
		case 200:
			response_status = " OK";
			break;
		case 404:
			response_status = " Not Found";
			break;
		case 400:
			response_status = " Bad Request";
			break;
	}

	string response_number = to_string(response);
	string http_response = string("HTTP/1.1 ")+ response_number + response_status +  " \r\n ";
	string date = "Date: " + currentDateTime() + " \r\n ";
	string content_type = string("Content-Type: text/plain") + " \r\n ";
	string content_length = string("Content-Length: 42") + " \r\n ";
	string content_encoding = string("Content-Encoding: identity") + " \r\n ";

	string server_data = data;

	string err = "";
	if (error_string.empty()){
		err = "";
	}
	else {
		err = "ERR:";
	}
	string final_header = http_response + date + content_type + content_length + content_encoding + server_data + "\n" + err + error_string;

	return final_header;
}

int rmFile(string file_path){
	FILE * file;
	file = fopen(strdup(file_path.c_str()), "r");
	if (file){
		//cout << "file exists" << endl;
		//file exists and can be opened
		// close file when you're done
		fclose(file);
	}
	else {
		//file doesn't exists or cannot be opened (es. you don't have access permission )
		//cout << "file doesnt exist";
		return 1;
	}
	//char filename[] = file_path.c_str();
	int ret = remove(file_path.c_str());
	if (ret != 0){
		// err
		return 1;
	}
	return 0;
}

string lsDir(string rest_path){

	DIR *dir;
	struct dirent *ent;
	string ls_string = "";
	if ((dir = opendir(rest_path.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			//printf ("%s\n", ent->d_name);
			ls_string.append(ent->d_name);
			ls_string.append("\n");
		}

		closedir (dir);
		return ls_string;
	} else {
		/* could not open directory */
		perror ("");
		return "1";
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
// arg 1 - string
// arg 2 - find substr
int isInString(string s1, string s2){
	if (s1.find(s2) != string::npos) {
		return 1;
	}
	return -1;
}
