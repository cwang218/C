//CS 3800 - HW 3: Jeff Goymerac and Christine Wang



/************************************************************************/ 
/*   PROGRAM NAME: client.c  (works with serverX.c)                     */ 
/*                                                                      */ 
/*   Client creates a socket to connect to Server.                      */ 
/*   When the communication established, Client writes data to server   */ 
/*   and echoes the response from Server.                               */ 
/*                                                                      */ 
/*   To run this program, first compile the server_ex.c and run it      */ 
/*   on a server machine. Then run the client program on another        */ 
/*   machine.                                                           */ 
/*                                                                      */ 
/*   COMPILE:    gcc -o client client.c -lnsl                           */ 
/*   TO RUN:     client  server-machine-name                            */ 
/*                                                                      */ 
/************************************************************************/ 
 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h>  /* define socket */ 
#include <netinet/in.h>  /* define internet socket */ 
#include <netdb.h>       /* define internet socket */ 
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define SERVER_PORT 9669     /* define a server port number */ 
 


void catchINT(int sig);
void * reader(void * arg);
void * writer(void *arg);

struct clients
{
    int sd;
    char username[512];
};

struct clients client_info;

pthread_t readT, writeT;

int main() 
{ 

    system("clear");

    char hostname[27], username[20], temp[20];
    struct hostent *hp; 
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) }; 


    printf("\n\nServer hostname: ");
    fgets(hostname, 27, stdin);

    /* get the host */ 
    while( ( hp = gethostbyname( hostname ) ) == NULL ) 
    { 
        printf("\n\nHostname does not exist.\nServer hostname: ");
        fgets(hostname, 27, stdin);
    } 
    
    printf("\n\n");
  
    bcopy( hp->h_addr_list[0], (char*)&server_addr.sin_addr, hp->h_length ); 
 
    /* create a socket */
    if( ( client_info.sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) 
    { 
        perror( "client: socket failed" ); 
        exit( 1 ); 
    } 

    /* connect a socket */ 
    if( connect( client_info.sd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) == -1 ) 
    { 
    	perror( "client: connect FAILED:" ); 
    	exit( 1 ); 
    } 

    write(client_info.sd, client_info.username, strlen(client_info.username-1));
 
    printf("connect() successful! will send a message to server\n"); 

    printf("\nUsername: ");


    //while((getchar() != '\n') && !feof(stdin)); /* Empty loop */    

    fgets(client_info.username, 20, stdin);

    username[strlen(client_info.username)-1]='\0';

    signal(SIGINT, catchINT);

    if ( pthread_create( &writeT, NULL, writer, &client_info))
    {
            perror( "client: unable to create thread");
            exit( 1 );
    }
    if ( pthread_create( &readT, NULL, reader, &client_info))
    {
        perror( "client: unable to create thread");
        exit( 1 );
    }
    
    write(client_info.sd, client_info.username, strlen(client_info.username-1));

    pthread_join(writeT, NULL);
    pthread_join(readT, NULL);

    close(client_info.sd); 

    return(0); 
} 

void catchINT(int sig)
{
    printf("\n\nPlease use /exit /quit or /part to exit the chatroom.\n\n");
}

void * writer(void * arg)
{
    char wbuf[512];

   while (strcmp(wbuf,"/quit")!=0 && strcmp(wbuf,"/exit")!=0 && strcmp(wbuf,"/part")!=0)
    {
        gets(wbuf);

        write(client_info.sd, wbuf, strlen(wbuf) + 1);

    }

    pthread_cancel(readT);
    pthread_exit(arg);
}


void * reader(void * arg)
{
    struct clients * readPtr = (struct clients*)arg;
    char rbuf[512];
        
    while( read(readPtr->sd, rbuf, sizeof(rbuf)) > 0 )
    {        
        printf("%s\n", rbuf);
        memset(rbuf, '\0', sizeof(rbuf));
    }
    
    pthread_cancel(writeT);    
    pthread_exit(arg);
}