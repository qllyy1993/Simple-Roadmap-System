#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//This client for manager
char Command_m[20][256]=
        {
                "retrieve(\"retrieve.txt\")",//0
                "addVertex(POI, \"RCH\")",//1
                "addVertex(POI, \"E5\")",//2
                "addEdge(0, 7,false, 0.7f,  8.0f)",//3
                "addEdge(1, 7,true, 1.7f,  11.0f)",//4
                "store(\"store.txt\")",//19
                "addEdge(2, 8,false, 2.8f, 2.0f)",//5
                "addEdge(6, 7,false, 6.7f, 1.0f)",//6
                "addEdge(6, 8,true, 6.8f, 6.0f)",//7
                "addEdge(7, 8,false, 7.0f, 7.0f)",//8
                "edgeEvent(0,CLOSURE)",//9
                "edgeEvent(2,ACCIDENT)",//10
                "edgeEvent(3,CARSTOPED)",//11
                "edgeEvent(4,DEBRIS)",//12
                "road(1,3,4,7,9,\"University Ave\")",//13
                "road(2,5,6,\"Columbia\")",//14
                "vertex(\"Toronto\")",//15
                "trip(\"DC\", \"DP\")",//16
                "trip(\"SLC\", \"Walmart\")",//17
                "removeEvent(2,ACCIDENT)"//18

        };
char Command_u[5][256]=
        {
                "vertex(\"DC\")",
                "vertex(\"DP\")",
                "vertex(\"Toronto\")",
                "trip(\"DC\", \"DP\")",
                "trip(\"SLC\", \"Walmart\")",
        };


void error(const char *msg)
{
    perror(msg);
    exit(0);
}



int main(int argc, char *argv[])
{
    int sockfd, portno, n,i=0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer_c[256],buffer_s[256];
   // char *temp=0;
   /* if (argc < 2) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }*/

    portno = 60000;//server port number
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");//get host by name，using localhost
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          (size_t)server->h_length);
    serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            error("ERROR connecting");
        else printf("Connect success!\n");
    do
    {
        bzero(buffer_c,255);
        printf("Client(me): ");
       // fgets(temp,2,stdin);
        strcpy(buffer_c,Command_m[i++]);//text with command_m for Manager client
        printf("%s\n",buffer_c);
        //fgets(buffer_c,255,stdin);
        n = (int)send(sockfd,buffer_c,strlen(buffer_c),0);
        if (n < 0)
        {
            perror("ERROR writing to socket:");
            exit(0);
        }
        bzero(buffer_s,255);
        n = (int)recv(sockfd,buffer_s,255,0);
        printf("Server: %s",buffer_s);
    }while (n>0 || i<5);//test 5 case
    if (n == 0)
        printf("Server disconnected~\n");

    close(sockfd);

    return 0;
}
