#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void escuta(void* arg)
{
	int sockfd = *(int*)arg;
	char buffer[256];
	int n;
	while(sockfd > 0)
	{
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) 
			error("ERROR reading from socket");
		printf("%s\n",buffer);		
	}
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
	pthread_t thread;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    pthread_create(&thread, NULL, (void*)escuta, (void*)&sockfd);
	//envia
	printf("Digite o seu usuario: ");
	bzero(buffer, 256);		
	scanf("%[^\n]s", buffer);
	setbuf(stdin, NULL);
	if( !strcmp(buffer, "bye") || !strcmp(buffer, " ") ) {
		pthread_cancel(thread);
		return 0;
	}
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
		error("ERROR writing to socket");
	while(sockfd > 0)
	{
		bzero(buffer, 256);		
		scanf("%[^\n]s", buffer);
		setbuf(stdin, NULL);
		n = write(sockfd,buffer,strlen(buffer));

		if (strcmp(buffer, "bye") == 0){
			break;
		}
		if (n < 0) 
			error("ERROR writing to socket");
	}	
	
	
	return 0;
}