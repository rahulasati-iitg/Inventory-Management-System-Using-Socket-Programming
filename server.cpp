//Required imports
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<unistd.h>
#include<vector>
#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<sstream>
#include <unordered_map>
#include<ctype.h>

//Defining maximum length of communication buffer
#define MAXLENGTH 100
using namespace std;

//Database entity or schema
class MyData 
{
    private:
        int upcCode; //Primary Key
        string itemName;//Name of item
        double price;//Price of item
    public:
        //Getters
        int getUpcCode()
        {
            return upcCode;
        }
        string getItemName()
        {
            return itemName;
        }
        double getPrice()
        {
            return price;
        }
        //Setters
        void setUpcCode(int upcCode)
        {
            this->upcCode = upcCode;
        }
        void setItemName(string itemName)
        {
            this->itemName = itemName;
        }
        void setPrice(double price)
        {
            this->price = price;
        }
};
//Local instance of database using Hash Map
unordered_map<int,MyData*> db;
//Initializing database from file
void create_db(string);
//Function to validate whether requested code exists in database or not
MyData* validate_code(int);
//Creating multiple Child processes to handle concurrent clients
void create_child_process(int,int);
//Ctrl+C command input handler
void sig_handler(int);
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
//Socket descriptors for Server
int listensd,connectsd;
int main(int argc,char **argv)
{
    socklen_t client_len;//To get the length of client socket
    pid_t childpid;//Process id for child process
    sockaddr_in server_addr,client_addr;//socket address structure for client and server

    if(argc < 2)
    {
        cout<<"Very few arguments passed!..exiting\n";
        exit(EXIT_FAILURE);
    } 
    stringstream sstoi(argv[2]);
    int SERVER_PORT;
    sstoi>>SERVER_PORT;
    //creating the TCP-socket with IPV4
    if((listensd=socket(AF_INET,SOCK_STREAM,0))<0) 
    {
        perror("Socket creation failed!");
        return 0;
    }
    //Server socket address format
    server_addr.sin_family = AF_INET;//IPV4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);//IP address of server passed as command line args
    server_addr.sin_port = htons(SERVER_PORT); // Port number passed as command line args

    
    //Binding socket for listening
    if(::bind(listensd,(sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed!");
        return 0;
    }
    //Server is ready to accept connections from client
    cout<<"Server is running..."<<endl;
    cout<<"Server IP : "<<inet_ntoa(server_addr.sin_addr)<<endl;
    cout<<"Socket ID : "<<listensd<<"\nServer Port : "<<SERVER_PORT<<endl;
    //Listening to accept connections from client with maximum waiting queue of length 5
    listen(listensd,5);
    //Initialising Data base from file : data.csv
    create_db("data.csv");
    //Invoking user-defined Ctrl+C command input handler 
    signal(SIGINT,sig_handler);
    while(true)
    {
        client_len = sizeof(client_addr);
        //Accepting connections from multiple clients
        if((connectsd=::accept(listensd,(sockaddr *)&client_addr,&client_len))<0)
        {
            perror("Failed to establish connection!");
            return 0;
        }
        if((childpid=fork())==0)
        {
            //Child process closes its copy of listen socket descriptor
            close(listensd);
            cout<<"Client process : "<<getpid()<<" servicing"<<endl;
            //Handling client requests
            create_child_process(connectsd,getpid());
            //Closing client conneceted socket for child process
            close(connectsd);
            exit(EXIT_FAILURE);
        }
        //Closing Parent process' copy of socket
        close(connectsd);
    }
}
//Function accepts connected socket descriptor and process id of child process
void create_child_process(int connectsd,int pid)
{
    int length,request_type,number,upc_code;
    double total = 0.0;
    MyData* dbout;
    //Receive and send buffer
    vector<char> buff(MAXLENGTH),message(MAXLENGTH);
    string msg;
    while(true)
    {
        //Clearing out the message send buffer
        fill(message.begin(),message.end(),0);
        if((length = recv(connectsd,&*(buff.begin()),MAXLENGTH,0)) < 0)
        {
            msg = "4 : Error receiving command..exiting\n";
            cout<<"Client process : "<<pid<<" response : "<<msg<<endl;
            copy(msg.begin(),msg.end(),message.begin());
            send(connectsd,&*(message.begin()),MAXLENGTH,0);
            close(connectsd);
            exit(EXIT_FAILURE);
        }
        //Cltr+C Command from client
        if(strcmp(&*(buff.begin()),"-999")==0)
        {
            cout<<"Client process : "<<pid<<" terminated abnormally!"<<endl;
            close(connectsd);
            exit(EXIT_FAILURE);
        }
        //Client buffer size should be minimum of size 5 if first char is 0
        int buff_size = strlen(&*(buff.begin()));
        if(buff_size == 0 || ((request_type = buff[0] - '0')==0 && buff_size<5))
        {
            msg = "1 : Protocol error..discarding packet!Resend request!\n";
            cout<<"Client process : "<<pid<<" response : "<<msg<<endl;
            copy(msg.begin(),msg.end(),message.begin());
            send(connectsd,&*(message.begin()),MAXLENGTH,0);

        }
        else {
            //Correct request type
            if(request_type==0)
            {
                //Converting 3 digit upc code in string to integer
                upc_code = 100*(buff[1] - '0') + 10*(buff[2] - '0') + (buff[3] - '0');
                number = atoi(&*(buff.begin() + 4));//Quantity
                dbout = validate_code(upc_code);//Check for existence
                if(dbout == NULL)
                {
                    msg = "1 : UPC code "+to_string(upc_code)+" not found!Resend packet!\n";
                    cout<<"Client process : "<<pid<<" response : "<<msg<<endl;
                    send(connectsd,msg.c_str(),MAXLENGTH,0);
                }
                else{
                    total = total + dbout->getPrice()*number;
                    msg = "0 : Price = "+to_string(dbout->getPrice())+"\tItem name : "+dbout->getItemName()+"\n";
                    cout<<"Client process : "<<pid<<" response : "<<msg<<endl;
                    send(connectsd,msg.c_str(),MAXLENGTH,0);

                }
            }
            else if(request_type == 1)//Client closes connection request
            {
                msg = "2 : Total cost = "+to_string(total)+"\n";
                send(connectsd,msg.c_str(),MAXLENGTH,0);
                cout<<"Client process : "<<pid<<" response : "<<msg<<endl;
                cout<<"Client process : "<<pid<<" serviced successfully!!"<<endl;
                close(connectsd);
                exit(EXIT_SUCCESS);
            }
            else{ //Any other request is not accepted
                msg = "1 : Protocol error...discarding packet!Request to resend!\n";
                cout<<"Client process : "<<pid<<" response : "<<msg<<endl;
                send(connectsd,msg.c_str(),MAXLENGTH,0);
            }
        }
        

    }
}
//Instantiating the data base
void create_db(string filename)
{
    ifstream fin;
    fin.open(filename);
    if(!fin)
    {
        cout<<"File can't be opened\n";
        return ;
    }
    vector<string> row;//Stores current row of .csv file as vector of strings
    string line,column;

    while(!fin.eof())
    {
        row.clear();
        getline(fin,line);
        //Tokenizing into multiple strings delimited by ','
        stringstream ss(line);
        while(getline(ss,column,','))
        {
            row.push_back(column);
        }
        /*cout<<row[0]<<","<<row[1]<<","<<row[2]<<endl;*/
        MyData *mydata = new MyData();
        mydata->setUpcCode(stoi(row[0]));
        mydata->setItemName(row[1]);
        mydata->setPrice(stod(row[2]));
        //Inserting into database 
        db.insert(make_pair(stoi(row[0]),mydata));
    }
    /*for(auto x : db)
    {
        cout<<x.first<<","<<x.second->getItemName()<<","<<x.second->getPrice()<<endl;
    }*/

}
MyData* validate_code(int code)
{
    //Checking the requested upc code exists in db or not
    if(db.find(code) != db.end())
        return db[code];
    return NULL;
}
//User defined Signal handler for handling Ctrl+C command in Server 
void sig_handler(int signal)
{
    close(listensd);
    cout<<"Server exiting!!...\n";
    //Sending Client message that server is terminated
    string msg = "3 : Server terminated!\n";
    send(connectsd,msg.c_str(),MAXLENGTH,0);
    //close(connectsd);
    exit(EXIT_FAILURE);
}
