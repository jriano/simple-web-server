/*
	HTML server program
	by Juan Riano
*/

#include "headers.h"

/*
 *  main() readies the listening socket, forks to create new connected sockets
 * 	and then calles serve(), whish delivers the content.
 */
int main(int argc, char *argv[])
{
	// vars
	int 					sock_fd, new_sock_fd, port, pid;
	struct sockaddr_in 		cli_addr, serv_addr;
	socklen_t				addr_len;


	// usage instructions
	if(argc != 2 || 4951 < atoi(argv[1]) || 1024 > atoi(argv[1]))
	{
		printf("Usage: ./ws port_number\n");
		printf("port number has to be between 1024 ~ 4951\n");
		return -1;
	}

	// set listening socket
	port = atoi(argv[1]);
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Socket file descriptor: %d\n");
	if(sock_fd < 0) syserr(FATAL, "Can't open socket\n", 0);
	printf("Creating main process socket...\n");

	// set server's address
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	// bind socket to address
	bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(sock_fd < 0)	syserr(FATAL, "Can't bind\n", 0);
	printf("Binding socket to port %d\n", port);

	// listen on socket
	if(listen(sock_fd, 5) < 0) syserr(FATAL, "Listen failed\n", 0);
	printf("Listening..\n");

	// infinite loop that waits for clients connections
	for(;;)
	{
		// set socket to manage children connections
		addr_len = sizeof(cli_addr);
		memset(&cli_addr, 0, addr_len);
		printf("Ready do accept connections\n");
		new_sock_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &addr_len);
		if(new_sock_fd < 0)
			syserr(FATAL, "Accept could not set socket\n", 0);
		printf("Accepted a new connection...\n");

		// create new process to manage children connection
		if((pid = fork()) < 0) syserr(ERROR, "Could not fork\n", 0);
		if(pid == 0)
		{
			// children process, manage connection, close listening socket
			(void)close(sock_fd);
			serve(new_sock_fd);
		}
		else
			// parent process, close children socket
			(void)close(new_sock_fd);
	}
	return 0;
}


/*
 * 	Serves pages and files pointed by the parameter file descriptor
 * 	Serve() runs inside their own process
 * 	It does all the validations needed before sending the file to the client.
 */
void serve(int fd)
{
	printf("Server is going to serve a file/page\n");

	// vars
	int 		get_size, ext_size, mime_match, file_fd, i;
	char		*file_type, buffer[BUFFERSIZE +1], file_url[256], temp;
	long 		file_size, req_size;
	size_t 		block_size;

	// ready buffer
	memset(&buffer, 0, BUFFERSIZE +1);

	// read client's request
	req_size = read(fd, buffer, BUFFERSIZE);
	if(req_size < 1) syserr(FATAL, "Could not read from buffer", 0);
	else
	{
		// cleans buffer of returns and new lines
		for(i=0; i<=req_size; i++)
			if(buffer[i] == '\n' || buffer[i] == '\r') buffer[i] = 0;
	}

	// find out if it is a GET request
	if(0 != strncmp(buffer, "GET ", 4)) syserr(FATAL, "Only GET supported\n", 0);

	// zero terminate string after file name
	// and also ignore case of the request
	for(i=4; i<req_size; i++)
	{
		temp = buffer[i];
		if(isblank(temp)) buffer[i] = '\0';
		else
			if(isupper(temp)) buffer[i] = tolower(temp);
	}

	// deal with no file-name provided
	if(0 == strncmp(buffer,"GET /\0",6)) strcpy(buffer,"GET /index.html");

	// figure out if requested file-type is supported
	get_size = strlen(buffer);
	mime_match = 0;
	// compare the extensions
	for(i=0; mimetype[i].ext!=0; i++)
	{
		ext_size = strlen(mimetype[i].ext);
		if(0 == strncmp(&buffer[get_size-ext_size], mimetype[i].ext, get_size))
		{
			mime_match = 1;
			file_type = mimetype[i].ext;
			break;
		}
	}

	if(!mime_match)	// file type not supported
		syserr(FATAL, "Requested file's type is not supported\n", 0);
	// file type supported, open it for reading
	// all files are located inside the html folder, assemble the file_url
	memset(file_url, 0, 256);
	strcpy(file_url, root_path);
	strcat(file_url, &buffer[5]);
	printf("Will try to serve %s\n", file_url);

	// open the requested file
	file_fd = open(file_url, 0, O_RDONLY);

	// if file could not be opened:
	if(-1 == file_fd)
	{
		// if the missing file is html, send a dummy page
		if(0 == strcmp("html",file_type))
			syserr(FMISSING, "", fd);
		// if the missing file is of other type, drop the request
		else syserr
			(FATAL, "File is not in this server\n", 0);
	}

	// get the file size
	file_size = (long)lseek(file_fd, (off_t)0, SEEK_END);
	(void)lseek(file_fd, (off_t)0, SEEK_SET);

	// assemble buffer with HTTP header, then send it to client
	(void)sprintf(buffer, "HTTP/1.0 200 OK\nContent-Length: %ld\n"
		"Content-Type: %s\nConnection: close\n\n", file_size, file_type);
	if(-1 == (int)write(fd, buffer, strlen(buffer)))
		syserr(FATAL, "Could not write to buffer", 0);

	// send file to client
	for(;;)
	{
		block_size = read(file_fd, buffer, BUFFERSIZE);
		if(0 >= (int)block_size) break;
		(void)write(fd, buffer, block_size);
	}
	printf("File %s was sent to client  successfully\n", file_url);

	close(file_fd);
	close(fd);

	// this process' useful life is gone, not need for it to live anymore
	exit(EXIT_SUCCESS);
}


/*
 * 	This function deals with the different errors that can
 * 	happen in the program. In some cases just shows the error in the
 * 	shell and continues, in other cases it terminates the calling
 * 	process after showing a message.
 */
void syserr(int error, char *str, int fd)
{
	// vars
	// dummy page to return for forbidden pages/files
	char *adenied =
	"HTTP/1.1 403 Forbidden\n"									// 23
	"Content-Length: 130\n"										// 20
	"Connection: close\n"										// 18
	"Content-Type: text/html\n\n"								// 25
	"<html><head><title>403 Forbidden</title></head>\n"			// 48
	"<body><h1>404 Forbidden</h1>\n"							// 29
	"<p>Acess to this file is forbiden.</p></body></html>\n";	// 53 = 130,216

	// dummy page to return when file requested is missing
	char *fmissing =
	"HTTP/1.0 404 Not Found\n"									// 23
	"Content-Length: 138\n"										// 20
	"Connection: close\n"										// 18
	"Content-Type: text/html\n\n"								// 25
	"<html><head><title>404 File Not Found</title></head>\n"	// 53
	"<body><h1>404 File Not Found</h1>\n"						// 34
	"<p>File requested was not found.</p></body></html>\n";		// 51 = 138,224

	// deals with different error types
	switch(error)
	{
		case FATAL:		// error for which the process has to terminate
			printf("%s", str);
			exit(-1);
		case ERROR: 	// error for which process may continue
			printf("%s: %d\n", str, errno);
			break;
		case FMISSING:	// show message page and exit
			printf("The file requested is not available.\n");
			(void)send(fd, fmissing, 224, 0);
			exit(-1);
		case ADENIED:	// show message page and exit
			printf("Access to the requested file is not permitted\n");
			(void)send(fd, adenied, 216, 0);
			exit(-1);
	}
}



