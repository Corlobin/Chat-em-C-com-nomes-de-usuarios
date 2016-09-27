/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_CLIENTES 15
#define MAX_MSG 	 256
typedef struct {
	int conexao;
	char nome[36];
	int id;
} connection_info;
connection_info* clientes[MAX_CLIENTES];

int conectados;

void bcast(char* msg, int id) {
	int i;
	char local_buff[MAX_MSG];
	for (i = 0; i < MAX_CLIENTES; i++) {
		if(clientes[i] != NULL && clientes[i]->id != id) {
			sprintf(local_buff, "[%s] diz: %s", clientes[i]->nome, msg);
			write(clientes[i]->conexao,local_buff,strlen(local_buff));
		}
	}
}

int add_cliente(connection_info* arg) { 
	int i = 0;
	for( ; i < MAX_CLIENTES; i++) {
		if (clientes[i]==NULL){
			clientes[i] = arg;
			return 1;
		}
	}
	return 0;
}
void remove_cliente(int cli_id){
	int i = 0;
	for( ; i < MAX_CLIENTES; i++) {
		if (clientes[i] != NULL && clientes[i]->id ==cli_id){
			free(clientes[i]);
			clientes[i] = NULL;
		}
	}	
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void* conexao(void* arg){
	connection_info* cliente = (connection_info*)arg;
	printf("ID: %d\n", cliente->id);
	
	int n;
	char buffer[MAX_MSG];
	while((n = (int)read(cliente->conexao,buffer,255)) != 0)
	{ 
		// primeira vez
		if(!strcmp(cliente->nome, "none")) {
			// primeira msg de nome
			strcpy(cliente->nome, buffer);
		} else{
			printf("[%s]: %s\n",cliente->nome, buffer);
			bcast(buffer, cliente->id);			
		}
		if (n < 0) error("ERROR reading from socket");
		if (strcmp(buffer, "bye") == 0){
			break;
		}
		if (n < 0) error("ERROR writing to socket");
		bzero(buffer,MAX_MSG);		
	}
	printf("O cliente %s saiu da sala.\n", cliente->nome);
	remove_cliente(cliente->id);
	
}

int main(int argc, char *argv[])
{
	pthread_t thread;
	
	int sockfd, newsockfd, portno, clilen;	
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,MAX_CLIENTES);
	clilen = sizeof(cli_addr);
	while(newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))
	{
		connection_info* cliente = (connection_info*) malloc(sizeof(connection_info));
		cliente->conexao = newsockfd;
		cliente->id = conectados;
		strcpy(cliente->nome, "none");
		add_cliente(cliente);

		pthread_create(&thread, NULL, (void*)conexao, (void*)cliente);
		conectados++;
	}

	return 0; 
}