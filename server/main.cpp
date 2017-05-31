/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>
#include <iostream>
#include <libgen.h>
#include "roadMap.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


void pushmap(unordered_map<string,int> &m)
{
    m["addVertex"]=1;
    m["addEdge"]=2;
    m["edgeEvent"]=3;
    m["removeEvent"]=4;
    m["road"]=5;
    m["trip"]=6;
    m["vertex"]=7;
    m["store"]=8;
    m["retrieve"]=9;
}

void trim(std::string &s)
{
    if (s.empty())
        return ;
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return ;
}

void par_quo(string &s)
{
    s.erase(0,s.find_first_not_of("\""));
    s.erase(s.find_last_not_of("\"")+1);
}


//parsing buffer
vector<string> parsing(string &s, string delimiter)
{
    vector<string> res;
    size_t pos=0;
    while ((pos=s.find(delimiter))!=string::npos)
    {
        res.push_back(s.substr(0,pos));
        s.erase(0,pos+delimiter.length());
    }
    res.push_back(s);
    return res;
}

VertexType stoVT(string &s)
{
    if (s=="POI")
        return POI;
    else if (s=="INTERSECTION")
        return INTERSECTION;
    else exit(1);
}

bool stoB(string &s)
{
    if (s=="1" || s=="true")
        return 1;
    else if (s=="0" || s=="false")
        return 0;
    else exit(1);
}


EventType stoET(string &s)
{
    EventType et;
    if (s=="ACCIDENT")
        et=ACCIDENT;
    if (s=="CARSTOPED")
        et=CARSTOPED;
    if (s=="DEBRIS")
        et=DEBRIS;
    if (s=="CLOSURE")
        et=CLOSURE;
    else
        exit(1);
    return et;
}

vector<int> vs_to_vi(vector<string> com)
{
    vector<int> com_i;
    for (int i = 0; i < com.size(); ++i)
        com_i.push_back(stoi(com[i]));
    return com_i;
}

bool execute(vector<string> com, RoadMap &g, unordered_map<string,int> &m, string com_name)
{
    switch (m[com_name])
    {
        case 1:
            //addVertex
            if (com.size()!=2)
                return 0;
            else
                g.addVertex(stoVT(com[0]),com[1]);
            std::cout<<"addVertex success"<<endl;
            return 1;
        case 2:
            //addEdge
            if (com.size()!=5)
                return 0;
            else
                g.addEdge((unsigned)stoi(com[0]),(unsigned)stoi(com[1]),stoB(com[2]),stof(com[3]),stof(com[4]));
            std::cout<<"addEdge success"<<endl;
            return 1;
        case 3:
            //edgeEvent
            if (com.size()!=2)
                return 0;
            else
                g.edgeEvent((unsigned)stoi(com[0]),stoET(com[1]));
            std::cout<<"edgeEvent success"<<endl;
            return 1;
        case 4:
            //removeEvent
            if (com.size()!=2)
                return 0;
            else
                g.removeEvent((unsigned)stoi(com[0]),stoET(com[1]));
            std::cout<<"removeEvent success"<<endl;
            return 1;
        case 5:
            //road
            if (com.size()>1)
            {
                string temp=com.back();
                com.pop_back();
                g.road(vs_to_vi(com),temp);
                std::cout<<"road success"<<endl;
                return 1;
            }
            else return 0;
        case 6:
            //trip
            if (com.size()!=2)
                return 0;
            else
            {
                Vertex v1=g.vertex(com[0]);
                Vertex v2=g.vertex(com[1]);
                if (v1.index!=INT_MAX && v2.index!=INT_MAX)
                    g.trip(v1,v2);
                std::cout<<"trip success"<<endl;
                return 1;
            }
        case 7:
            //vertex
            if (com.size()!=1)
                return 0;
            else g.vertex(com[0]);
            std::cout<<"vertex success"<<endl;
            return 1;
        case 8:
            //store
            if (com.size()!=1)
                return 0;
            else g.store(com[0]);
            std::cout<<"store success"<<endl;
            return 1;
        case 9:
            //retrieve
            if (com.size()!=1)
                return 0;
            else
                g.retrieve(com[0]);
            std::cout<<"retrieve success"<<endl;
            return 1;
        default:
            perror("No exists command!");
            return 0;

    }
}


//parsing and executing the request to the command and parameter
bool operation(string command, RoadMap &g, unordered_map<string,int> &m)
{
    //save all command parameters in com_para
    vector<string> com_para;
    string::size_type index_pre,index_suf;
    string com_name;
    //get command name and delete bracket
    index_pre=command.find_first_of("(");
    com_name = command.substr(0, index_pre);
    if (index_pre!=string::npos)
        command.erase(0, index_pre + 1);
    index_suf=command.find_last_not_of(")");
    if (index_suf!=string::npos)
        command.erase(index_suf+1);
    //get all parameters in command as string
    com_para=parsing(command,",");
    //delete the space and quotation of string
    for (int i = 0; i < com_para.size(); ++i)
    {
        trim(com_para[i]);
        if (com_para[i].find("\"")!=string::npos)
            par_quo(com_para[i]);
    }
    for (int j = 0; j < com_para.size(); ++j) {
        std::cout<<com_para[j]<<" ";
    }
    std::cout<<endl;

    bool res=execute(com_para,g,m,com_name);

    if (!res)
    {
        perror("Executing fail!");
        return 0;
    }
    else

        return 1;

}


int main(int argc, char *argv[])
{

    //chdir(dirname(argv[0]));
    RoadMap g;
    unordered_map<string, int> map;//for command operation
    pushmap(map);//set map for command name and int label
 int servsock, cliensock, portno,n=0;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer_s[256],buffer_c[256],command[256];
/*
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
*/
    servsock = socket(AF_INET, SOCK_STREAM, 0);
    if (servsock < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 60000;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (::bind(servsock, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(servsock,5);
    clilen = sizeof(cli_addr);
   if ((cliensock = accept(servsock, (struct sockaddr *) &cli_addr, &clilen)))
       printf("Accept success!\n");
   else
   {
       perror("Accept fail:");
       exit(0);
   }
    while ((n=(int)recv(cliensock,buffer_c,255,0))>0)
    {
        strcpy(command,buffer_c);
        printf("Client: %s\n",command);
        //operation on buffer_c
       // g.retrieve("retrieve.txt");
       if (!operation(command,g,map))
           printf("Executing failed~\n");
        bzero(buffer_c,255);
        printf("Server(me): ");
        fgets(buffer_s,255,stdin);
        n=(int)send(cliensock,buffer_s,strlen(buffer_s),0);
        if (n<0)
        {
            perror("Error writing to socket: ");
            exit(0);
        }
        bzero(buffer_s,255);
    }
    if (n==0)
        printf("Client disconnected~\n");
    else perror("Recv failed: ");

    close(cliensock);
    close(servsock);

    return 0;
}
