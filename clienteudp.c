/*Compilar - gcc clienteudp.c -o clienteudp
  Executar - ./clienteudp 127.0.0.1 mensagem
*/

/*
struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
*/
/*
struct hostent {
                      char    *h_name;        //official name of host
                      char    **h_aliases;    // alias list
                      int     h_addrtype;     // host address type
                      int     h_length;       //length of address
                      char    **h_addr_list;  // list of addresses
				}
              //#define h_addr  h_addr_list[0]  // for backward compatibility
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */
#include <stdlib.h>
#include <pthread.h>

#define REMOTE_SERVER_PORT 1500
#define LOCAL_SERVER_PORT  1500
#define MAX_MSG 100

void listenConnect(){
    int sd, rc, n, cliLen;
    struct sockaddr_in cliAddr, servAddr;
    char msg[MAX_MSG];

    /* socket creation */
    sd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sd<0) {
        printf(": cannot open socket \n");
        exit(1);
    }

    /* bind local server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(LOCAL_SERVER_PORT);
    rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
    if(rc<0) {
        printf(": cannot bind server port number %d \n", LOCAL_SERVER_PORT);
        exit(1);
    }

    printf(": waiting for data on port UDP %u\n",LOCAL_SERVER_PORT);

    /* server infinite loop */
    while(1) {

        /* init buffer */
        memset(msg,0x0,MAX_MSG);


        /* receive message */
        cliLen = sizeof(cliAddr);
        n = recvfrom(sd, msg, MAX_MSG, 0,(struct sockaddr *) &cliAddr, &cliLen);

        if(n<0) {
            printf(": cannot receive data \n");
            continue;
        }

        /* print received message */
        printf(": from %s:UDP%u : %s \n",inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port),msg);

    }/* end of server infinite loop */

    return ;
}
void *createConnect(){
	int sd, rc, i;
	char mensagem[30];
	struct sockaddr_in cliAddr, remoteServAddr;
	struct hostent *h;
             pthread_t pth;

	/* get server IP address (no check if input is IP address or DNS name */
	h = gethostbyname("127.0.0.1");//retora a estrutura hstent com base no ip char passado como parâmetro
	if(h==NULL) {//verifica se foi criado a estrutura hostent
		printf(": unknown host \n");
		exit(1);
	}
	printf(": sending %s to '%s' (IP : %d) \n",mensagem, h->h_name,h->h_addr_list[0]);

	remoteServAddr.sin_family = h->h_addrtype;	//seta o tipo AF_INET (Arpa internet Protocol)
	//void * memcpy ( void * destination, const void * source, size_t num );
	memcpy((char *) &remoteServAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);	//configura a porta de recebimento do servidor

	/* socket creation */
	sd = socket(AF_INET,SOCK_DGRAM,0);	//cria o socket
	if(sd<0) {
		printf(": cannot open socket \n");
		exit(1);
	}
	cliAddr.sin_family = AF_INET;	//seta o tipo da familia do socket
	cliAddr.sin_addr.s_addr = htonl(INADDR_ANY); //seta o ip do socket INADDR_ANY(recebe pacotes de todas as interfaces)
	cliAddr.sin_port = htons(0);//seta a porta do socket
	//int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
	rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));	//nomeia o socket recebendo a porta como nome
	if(rc<0) {//verifica se a funcão bind funcionou
		printf(": cannot bind cliente port\n");
		exit(1);
	}

            //criando a thread q escuta as mensagens
             // pth:identificador da nova thread    att:é um atributo de argumento      function:função que será chamada        arg:parametros que deseja passar para a função
             pthread_create(&pth, NULL, listenConnect, NULL);

             while(1){
                            //pegando mensagem
                            printf("etre com a mensagem\n");
                            scanf("%s",mensagem);
                            if(!strcmp(mensagem,"exit"))
                                exit(1);
             /* send data */
	//for(i=2;i<argc;i++) {
		//ssize_t sendto(int socket, const void *message, size_t length,int flags, const struct sockaddr *dest_addr,socklen_t dest_len);
		rc = sendto(sd,mensagem, strlen(mensagem), 0,(struct sockaddr *) &remoteServAddr,sizeof(remoteServAddr));
		if(rc<0) {
		  printf(": cannot send data \n");
		  close(sd);
		  exit(1);
		}

	//}
             }
	return;
}

int main(int argc, char *argv[]) {
	int op;

	do{printf("escolha o que deseja fazer\n");
                	printf("0 para sair\n");
                	printf("1 parainiciar chat\n");
                	scanf("%d",&op);
	}while(op<0||op>1);
	switch (op){
		case 1:
			createConnect();
			break;
	}
  return 1;

}

