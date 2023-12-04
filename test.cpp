#include<vector>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <unordered_map>
using namespace std;
class MyData 
{
    private:
        int upcCode;
        string itemName;
        double price;
    public:
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
unordered_map<int,MyData*> db;
void create_db(string);
MyData* validate_code(int);
int main()
{

    /*MyData* mydata = new MyData();
    mydata->setItemName("Chodna");
    mydata->setPrice(1000000);
    mydata->setUpcCode(atoi("069"));
    db.insert(make_pair(69,mydata));*/
    int request_type,upc_code,number;
    char s[] = "015689";
    vector<char> buff;
    buff.reserve(100);

    buff[0] = '5';
    buff[1] = '3';
    //buff.assign(s,s+6);
    /*request_type = buff[0] - '0';
        upc_code = 100*(buff[1] - '0') + 10*(buff[2] - '0') + (buff[3] - '0');
        number = atoi(&*(buff.begin() + 4));*/
    //cout<<request_type<<endl<<upc_code<<endl<<number<<endl<<buff.size()<<endl;
    /*MyData * temp;
    if(db.find(68)==db.end())
        temp=NULL;
    else temp = db[68];
    cout<<(temp==NULL)<<endl;*/
    create_db("data.csv");
    int x=90;
    string str = "Hello ram "+to_string(x)+" Ending ";
    cout<<str;

}
void create_db(string filename)
{
    ifstream fin;
    fin.open(filename);
    if(!fin)
    {
        cout<<"File can't be opened\n";
        return ;
    }
    vector<string> row;
    string line,column;

    while(!fin.eof())
    {
        row.clear();
        getline(fin,line);
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
        db.insert(make_pair(stoi(row[0]),mydata));
    }
    /*for(auto x : db)
    {
        cout<<x.first<<","<<x.second->getItemName()<<","<<x.second->getPrice()<<endl;
    }*/

}