/*
	HTML server program
	by Juan Riano
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>


// for managing errors
#define ADENIED 0
#define FMISSING 1
#define ERROR 2
#define FATAL 3


// read-buffer size
#define BUFFERSIZE 8192


// mime-types supported
struct
{
	char *ext;
	char *type;
} mimetype[] =
{
				{"png", "image/png"},
				{"jpg", "image/jpg"},
				{"gif", "image/gif"},
				{"jpg", "image/jpg"},
				{"jpeg","image/jpeg"},
				{"ico", "image/ico"},
				{"pdf", "application/pdf"},
				{"html","text/html"},
				{"htm", "text/html"},
				{"js", "text/css"},
				{"txt", "text/plain"},
				{0,0}
};


// default location for content
char *root_path = "./html/";


// Function prototypes
void syserr(int error, char *str, int fd);
void serve(int fd);

