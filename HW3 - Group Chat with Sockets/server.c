/*
Jeff Goymerac & Christine Wang
jgg98         & cywm94

Server.c

compile using: gcc -o server server.c -pthread


*/  




#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include <pthread.h>
#define SERVER_PORT 9669    /* define a server port number */
#define MAX_CLIENT 10
void sig_handler(int sig);
void * ClientHandler(void *pts);
void WriteAll(int soc, char * buf);
void cleanUp(int netSock);

int FDarray[MAX_CLIENT];
int sd, counter;
pthread_mutex_t m;
pthread_t client_threads[MAX_CLIENT];

int main()
{
    bool tester = false;
    signal(SIGINT, sig_handler);
    
    int ns, k, pid, pid2, pid1;
    
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
    struct sockaddr_in client_addr = { AF_INET };
    int client_len = sizeof( client_addr );
    char buf[512], *host;
    char *test ="tester";
    counter =0;
    
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
    perror( "server: socket failed" );
    exit( 1 );
    }
    
 
    if( bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
    {
    perror( "server: bind failed" );
    exit( 1 );
    }

    
    if( listen( sd, MAX_CLIENT ) == -1 )
    {
    perror( "server: listen failed" );
    exit( 1 );
    }

    printf("SERVER is listening for clients to establish a connection\n");
    while( !tester)
    {
      
        if (counter < MAX_CLIENT)
        {    
            if((ns = accept( sd, (struct sockaddr*)&client_addr,
                       (socklen_t*)&client_len)) >0)

            {
                pthread_mutex_lock(&m);
                FDarray[counter]= ns;
                if(pthread_create(&client_threads[counter], NULL, 
                    ClientHandler, &FDarray[counter]))
                    {
                        perror( "server: Thread not created\n");
                        exit(1);
                    }
                counter++;    
                pthread_mutex_unlock(&m);
                
            }
        }

    }
  
    unlink((char *)&server_addr.sin_addr);

    return(0);
};


void * ClientHandler(void * pts)
{ int netSock = *((int*)pts);
    int num_read;
    char username[128];
    char buf[512];
    char sbuf[1024];
    
    
    if ( read( netSock, username, 128) != EOF)
    {
    
        strcpy(sbuf, ">> User '");
        strcat(sbuf, username);
        strcat(sbuf, "' connected.");
        WriteAll(-1, sbuf);
        printf("%s\n", sbuf);
    }
        
    
    while( read( netSock, buf, sizeof(buf)) > 0 )
    {
        if( strcmp(buf, "/exit") && strcmp(buf, "/quit") && 
            strcmp(buf, "/part") )
        {
            strcpy(sbuf, username);
            strcat(sbuf, " : ");
            strcat(sbuf, buf);
           
            WriteAll(netSock, sbuf);
           
            printf("%s\n", sbuf);
            memset(buf, '\0', sizeof(buf));
        }
        else
        {
            
            printf("Client %s is exiting.\n", username);
            
            break;


        }
     }
         
    strncpy(sbuf, ">> User '", sizeof(">> User '"));
    strcat(sbuf, username);
    strcat(sbuf, "' left.");
    WriteAll(netSock, sbuf);
    printf("%s\n", sbuf);
    
    
    cleanUp(netSock);
             
    pthread_exit( pts);
    
}
void sig_handler(int signo) 
{
    int pid2, i;
  switch(signo) {
  case SIGINT:  
    printf("\n\nProcess %d received SIGINT (Cntl-C)\n", getpid()); 
    
    printf("Server is shuting down in 10 seconds\n");
    WriteAll(-1, "Server is shutting down in 10 seconds\n");
    sleep(3);
    printf("Server is shuting down in 7 seconds\n");
    WriteAll(-1, "Server is shutting down in 7 seconds\n");
    sleep(3);
    printf("Server is shuting down in 4 seconds\n");
    WriteAll(-1, "Server is shutting down in 4 seconds\n");
    sleep(1);
    printf("Server is shuting down in 3 seconds\n");
    WriteAll(-1, "Server is shutting down in 3 seconds");
    sleep(1);
    printf("Server is shuting down in 2 seconds\n");
    WriteAll(-1, "Server down in 2");
    sleep(1);
    printf("Server is shuting down in 1 seconds\n");
    WriteAll(-1, "1!!!!!");
    
     for (i = 0; i < counter; i++)
    {
        pthread_cancel( client_threads[i]);
        close( FDarray[i] );
    };


    printf("Goodbye!!\n");
    kill(pid2, SIGINT);
  
    exit(1);
   
    break;

  case SIGQUIT:  

       

    printf("Process %d received SIGQUIT \n", getpid());
    printf("Process %d: passing SIGQUIT SIGQUIT SIGQUIT to process %d ............ \n", getpid(), pid2);
   
    break;
  default: break;
  }
  return;

}
void WriteAll(int ns, char * msg)
{
    pthread_mutex_lock(&m);
    int k;
    for ( k=0; k<counter;k++)
    {
        if (FDarray[k] != ns)
            write(FDarray[k], msg, strlen(msg)+1);
    }
    pthread_mutex_unlock(&m);
}

void cleanUp(int netSock)
{
    pthread_mutex_lock(&m);
    int i = 0;
    
   
    while ( i < counter && FDarray[i] != netSock)
    {
        i++;
    };
    
    
    int j;
    for( j=i; j<(counter-1); j++)
    {
        FDarray[j] = FDarray[j+1];
        client_threads[j] = client_threads[j+1];        
    }
    counter--;
    
    pthread_mutex_unlock(&m);
    
    return;
}
