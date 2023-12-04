//cash register client
//Required imports
#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<string>
#include<unistd.h>
#include<ctype.h>
#include<sstream>
#include<cstring>
#include<vector>
//Defining maximum length of communication buffer
#define MAXLENGTH 100
using namespace std;

int command(int);
void display_command(void);//display command  containing the specific format
void sig_handler(int signal); //signal handler
int socketfd;//socket descriptor for client
typedef struct sockaddr sockaddr;


int main(int argc,char **argv)
{
    int check_code; //b
    int SERVER_PORT;
    stringstream sstoi(argv[2]);
    sstoi>>SERVER_PORT;
    struct sockaddr_in serveraddress;
    socketfd=socket(AF_INET,SOCK_STREAM,0);//creating the TCP-socket with IPV4
    if(socketfd<0)
    {
        perror("Socket Cannot Be Created!!!!");
        return 0;
    }
     //Server socket address format
    serveraddress.sin_family=AF_INET;//IPV4
	serveraddress.sin_addr.s_addr = inet_addr(argv[1]); //IP address of server passed as command line args
	serveraddress.sin_port = htons(SERVER_PORT);// Port number passed as command line args
    if((connect(socketfd,(sockaddr *) &serveraddress,sizeof(serveraddress)))<0) //client calls the connect function to establish connection with server
	{
		perror("Server is down!"); // if a negative no is returned (-1) typically it means server isn't in working condition
		return 0;
	}
    signal(SIGINT,sig_handler);//Invoking user-defined  command handler 
    cout<<endl<<"Connection Is Made :::"<<endl;
     
     while(true)
     {
        
        check_code=command(socketfd);
       if(check_code<0) //server terminated
       {
        cout<<"Closing client Socket!!!"<<endl;
        close(socketfd);
        exit(EXIT_FAILURE);
       }
       else if(check_code==1) //client closes connection
       {
         close(socketfd);
         exit(EXIT_SUCCESS);
       }

     }
     
    return 0;
}

void display_command()
{
    cout<<"\nEnter command in the following format:\n<request type><item code><quantity>"<<endl; //displauing command format

}
int command(int socketfd) //function for sending specific command to server for communication
{
    string command_buff;
    vector<char> received_buff(MAXLENGTH);
    display_command(); 

 while(true)
 {
    cin>>command_buff; //talking user input

    send(socketfd,command_buff.c_str(),MAXLENGTH,0);

    recv(socketfd, &*(received_buff.begin()),MAXLENGTH,0);

    if(received_buff[0]=='0') //if  server sends code-0 which means received_buff contains valid data about price and item name
    {
        cout<<&*(received_buff.begin())<<endl; // print the data
    }
    else if(received_buff[0]=='2' || received_buff[0]=='4') //if  client closes connection or error receiving command
    {
        cout<<&*(received_buff.begin())<<endl; //print the data
        close(socketfd); //close socket connection
        return 1;
    }
    else if(received_buff[0]=='1') //protocol error or upc code not found
    {
        cout<<&*(received_buff.begin())<<endl;
        display_command(); //prints the specific command format
        continue;

    }
    else if(received_buff[0]=='3') //server terminated
    {
        cout<<"FATAL ERROR!!! Exiting..."<<endl;
        cout<<&*(received_buff.begin())<<endl;
        close(socketfd);
        return -1;
    }

 }
}

void sig_handler(int signal)
{
    //User defined Ctrl+C command handler
    string command_buff;
    cout<<"Terminating......."<<endl; //Client Terminating
    command_buff = "-999";//Sending this special message indicating client has terminated with Ctrl+C
    send(socketfd,command_buff.c_str(),MAXLENGTH,0);
    close(socketfd);
    exit(EXIT_FAILURE);
}




