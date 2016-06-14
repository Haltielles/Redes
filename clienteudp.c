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
#define MAX_BUFFER 100
#define MAX_CONNECT 5

typedef unsigned short u16;
typedef unsigned long u32;
//---------------------------------------udp soma de verificação-----------------------------------//
u16 udp_sum_calc(u16 len_udp, u16 src_addr[],u16 dest_addr[], int padding, u16 buff[]){
u16 prot_udp=17;
u16 padd=0;
u16 word16;
u32 sum;	
int i;
	// Find out if the length of data is even or odd number. If odd,
	// add a padding byte = 0 at the end of packet
	if (padding&1==1){
		padd=1;
		buff[len_udp]=0;
	}
	
	//initialize sum to zero
	sum=0;
	
	// make 16 bit words out of every two adjacent 8 bit words and 
	// calculate the sum of all 16 vit words
	for (i=0;i<len_udp+padd;i=i+2){
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (unsigned long)word16;
	}	
	// add the UDP pseudo header which contains the IP source and destinationn addresses
	for (i=0;i<4;i=i+2){
		word16 =((src_addr[i]<<8)&0xFF00)+(src_addr[i+1]&0xFF);
		sum=sum+word16;	
	}
	for (i=0;i<4;i=i+2){
		word16 =((dest_addr[i]<<8)&0xFF00)+(dest_addr[i+1]&0xFF);
		sum=sum+word16; 	
	}
	// the protocol number and the length of the UDP packet
	sum = sum + prot_udp + len_udp;

	// keep only the last 16 bits of the 32 bit calculated sum and add the carries
    	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);
		
	// Take the one's complement of sum
	sum = ~sum;

return ((u16) sum);
}

//---------fica a espera de solicitações de conexão de outros nós----------//
void *pListen(){
	int listenner,saida,sizesender; //socket q aguarda solicitações
	struct sockaddr_in listenner_in,sender_in; //estrutura do socket
	char buffer[MAX_BUFFER];	//buffer da mensagem
	//criando o socket
	listenner=socket(AF_INET,SOCK_DGRAM,0);
	listenner_in.sin_family=AF_INET;
	listenner_in.sin_addr.s_addr=htonl(INADDR_ANY);
	listenner_in.sin_port=htons(LOCAL_SERVER_PORT);
	if(bind(listenner,(struct sockaddr *)&listenner_in,sizeof(listenner_in))<0){
		printf("cannot bind listenner port %d\n",LOCAL_SERVER_PORT);
		exit(1);
	}
	printf("pronto para receber mensagens\n");
	sizesender=sizeof(sender_in);
	//loop pra recebimento dos pacotes do buffer
	while(1) {
        memset(buffer,0x0,MAX_BUFFER); //seta zeros na memoria alocada para a variável
        saida = recvfrom(listenner, buffer,MAX_BUFFER, 0,(struct sockaddr *) &sender_in,&sizesender);
        if(saida<0) {
            printf(": não foi possível receber os dados\n");
            continue;
        }
        printf(": from %s:UDP%u : %s \n",inet_ntoa(sender_in.sin_addr),ntohs(listenner_in.sin_port),buffer);

    }

    return;
}

//--------------------solicita conexções a outros nós---------------------//
void *pSender(){
	int sending,saida;
	struct sockaddr_in sending_in,receiver_in;
	char buffer[MAX_BUFFER];
	struct hostent *sendip;	//estrutura auxiliar do socket
	//criando o socket
	sending=socket(AF_INET,SOCK_DGRAM,0);
	if(sending<0){
		printf("erro ao criar socket sending\n");
		exit(1);
	}
	sending_in.sin_family=AF_INET;
	sending_in.sin_addr.s_addr=htonl(INADDR_ANY);
	sending_in.sin_port=htons(0);
	saida=bind(sending,(struct sockaddr *)&sending_in,sizeof(sending_in));
	if(saida<0){
		printf("cannot bind sending\n");
		exit(1);
	}

	//criando receptor do datagrama
	sendip=gethostbyname("127.0.0.1");
	if(sendip==NULL){
		printf("ip inválido\n");
		exit(1);
	}
	receiver_in.sin_family=sendip->h_addrtype;
	memcpy((char *) &receiver_in.sin_addr.s_addr, sendip->h_addr_list[0], sendip->h_length);
	receiver_in.sin_port=htons(REMOTE_SERVER_PORT);

	//loop para envio da mensagem
	while(1){
        //pegando mensagem
        printf("etre com a mensagem\n");
        scanf("%s",buffer);
        if(!strcmp(buffer,"exit"))
            exit(1);
        /* send data */
	//for(i=2;i<argc;i++) {
		//ssize_t sendto(int socket, const void *message, size_t length,int flags, const struct sockaddr *dest_addr,socklen_t dest_len);
		saida = sendto(sending,buffer, MAX_BUFFER, 0,(struct sockaddr *) &receiver_in,sizeof(receiver_in));
		if(saida<0) {
		  printf(": cannot send data \n");
		  close(sending);
		  exit(1);
		}
	}
}
int main(int argc, char *argv[]) {
	int op;
	pthread_t tSend,tListen;
	pthread_create(&tSend, NULL,pSender, NULL);
	pthread_create(&tListen,NULL,pListen,NULL);
	while(1){

	}
	// do{printf("escolha o que deseja fazer\n");
 //                	printf("0 para sair\n");
 //                	printf("1 parainiciar chat\n");
 //                	scanf("%d",&op);
	// }while(op<0||op>1);
	// switch (op){
	// 	case 1:
	// 		createConnect();
	// 		break;
	// }
  return 1;

}

