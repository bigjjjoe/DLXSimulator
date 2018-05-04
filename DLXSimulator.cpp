#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>
#include <map>

#include <iostream>
using namespace std;

struct instruction{
    int opcode,RD,RS1,RS2;
    string imm;//lable or imm
    bool useful=true;
};

map<int,int> mem;
void Write_mem_int(int addr,int val)
{
    int a,b,c,d;
    a=(unsigned int)val>>24&0xff;
    b=(unsigned int)val>>16&0xff;
    c=(unsigned int)val>>8&0xff;
    d=val&0xff;
    mem[addr]=a;
    mem[addr+1]=b;
    mem[addr+2]=c;
    mem[addr+3]=d;
}
int Read_mem_int(int addr)
{
    int a,b,c,d;
    a=(mem[addr]&0xff)<<24;
    b=(mem[addr+1]&0xff)<<16;
    c=(mem[addr+2]&0xff)<<8;
    d=mem[addr+3]&0xff;
    return a+b+c+d;
}
void Write_mem_byte(int addr,int val)
{
    mem[addr]=val%256;
}
int Read_mem_byte(int addr)
{
    return mem[addr]%256;
}

void read_mem(char filename[256])
{
    ifstream fin;
    fin.open(filename);
    string buf; 
    while(getline(fin,buf))
    {
        string address(buf,0,10);
        string value(buf,11,14);
        int addr=strtol(address.c_str(),NULL,16);
        int val=strtol(value.c_str(),NULL,16);
        Write_mem_byte(addr,val);
    }
}


int main(int argc,char **argv)
{   //init
    map<string,int> code;
    code["HALT"]= 0;
    //r-type
    code["ADD"] = 1;   code["SUB"] = 2;     code["AND"] = 3;   code["OR"]  = 4;   code["XOR"] = 5;
    code["ADDI"]= 101; code["SUBI"]= 102;   code["ANDI"]= 103; code["ORI"] = 104; code["XORI"]= 105;
    //i-type
    code["SGT"] = 6;   code["SEQ"] = 7;     code["SGE"] = 8;   code["SLT"] = 9;  code["SNE"] = 10; code["SLE"] = 11;
    code["SGTI"]= 106; code["SEQI"]= 107;   code["SGEI"]= 108; code["SLTI"]= 109; code["SNEI"]= 110;code["SLEI"]= 111;
    
    code["SLLI"]= 112; code["SRLI"]= 113; 
    //mem ins
    code["LB"]  = 201; code["LW"]  = 202;   code["SB"]  = 203; code["SW"]  = 204;
   
    //j-type
    code["BEQZ"]= 301; code["BNEZ"]= 302;   code["JR"]  = 303;  
    
    code["OP"]  = 401;
    

    
    bool readmem=false;
    int i;
    char filename[3][256];
    for(i=0;i<argc-1;i++)
    {
        if(!strcmp(argv[i],"-i"))
            strcpy(filename[0],argv[i+1]);
        else if(!strcmp(argv[i],"-m"))
        {
            strcpy(filename[1],argv[i+1]);
            readmem=true;
        }
        else if(!strcmp(argv[i],"-o"))
            strcpy(filename[2],argv[i+1]);
    }
    //read input
    freopen(filename[0], "r", stdin);
    freopen(filename[2], "w", stdout);

    vector<instruction> ins;
    //do the first time pretreatment
    string buf;
    char pre,cur,emp;
    int size=0;
    pre=' ';
    emp=' ';
    while (cur=getchar())
    {
        if(cur==' '&&pre==' ')
            continue;
        if(cur=='\n'||cur=='\t'||cur=='\r')
            cur=' ';
        if((cur==';'||cur==':')&&pre!=emp)
        {
            buf+=emp;
            size++;
        }
        if(cur!=emp&&(pre==':'||pre==';'))
        {
            buf+=emp;
            size++;
        }
        if(cur>='a'&&cur<='z')
            cur-=32;
        else
            size++;
        if(cur==EOF)
            break;
        buf+=cur;
        pre=cur;
    }

    //ins
    istringstream is(buf);
    string s;
    vector<string> ins_tmp;
    map<string,int> lable_map;
    ins_tmp.clear();
    string x=";";
    string y=":";
    string z="HALT";
    int count=0,jump=0;
    while(is>>s)
    {
        ins_tmp.push_back(s);
        if(s==y)//:
        {
            string lable;
            lable=ins_tmp[0];
            lable_map[lable]=count;
            ins_tmp.clear();
            continue;
        }
        if(s==z)
        {
            instruction tmp;
            int opcode=code[ins_tmp[0]];
            tmp.opcode=opcode;
            ins.push_back(tmp);
            break;
        }
        if(s==x)//;
        {
           instruction tmp;
           int opcode = code[ins_tmp[0]];
           tmp.opcode = opcode;
            if(ins_tmp.size()==5)
               {
                   string rd(ins_tmp[1],1);
                   tmp.RD=atoi(rd.c_str());
                   string rs1(ins_tmp[2],1);
                    tmp.RS1=atoi(rs1.c_str());
                    if(opcode>=201&&opcode<=204||opcode>=101&&opcode<=113)
                    //now RS2 is imm
                        tmp.RS2=atoi(ins_tmp[3].c_str());
                    else{
                        string rs2(ins_tmp[3],1);
                        tmp.RS2=atoi(rs2.c_str());
                    }  
                }
                else if(ins_tmp.size()==4)
               {
                   string rd(ins_tmp[1],1);
                   tmp.RD=atoi(rd.c_str());
                    if(tmp.opcode==401)//op
                        tmp.RS1=atoi(ins_tmp[2].c_str());
                    else
                        tmp.imm=ins_tmp[2];//imm
                    if(tmp.opcode==301||tmp.opcode==302)
                        jump++;
                }
                else if(ins_tmp.size()==3)
               {
                    if(tmp.opcode==401)//op
                    {
                        tmp.opcode=402;
                        string rd(ins_tmp[1],1);
                        tmp.RD=atoi(rd.c_str());  
                    }
                    else
                        tmp.imm=ins_tmp[1];//jr lable
                }
           count++;
           ins.push_back(tmp);
           ins_tmp.clear();
        }
    }

    fclose(stdin);
    //Optimize
    map<string,int>::iterator it;
    int ad[100];
    memset(ad,0,sizeof(ad));
    int num=0;
    bool optflag=false;
    if(count>25&&jump<5)
    {
        map<int,int> block;
        map<int,int> relation;
        int i=0,j=0,k=0,num=0;     
        int p=count;
        //cout<<"count="<<count<<endl;
        instruction cur_ins;
        while(p>=0)
        {
            cur_ins=ins[p];
            if(cur_ins.opcode==402)//op
            {
                relation[cur_ins.RD]=1;
                //cout<<"i find a op rd R"<<cur_ins.RD<<endl;
                i++;
            }
            if(cur_ins.opcode==302||cur_ins.opcode==301)//bnez!=0||bqez==0
            {
                 //cout<<"i find a block"<<endl;
                 relation[cur_ins.RD]=1;
                 it=lable_map.find(cur_ins.imm);
                 if(it==lable_map.end())
                    //imm
                    block[p]=p+atoi(cur_ins.imm.c_str());
                 else//lable
                    block[p]=lable_map[cur_ins.imm];
                 //cout<<"the block is from "<<p<<"to "<<block[p]<<endl;
                 if(p-block[p]<=5)
                 {
                     p--;
                     continue;
                 }
                 int t=p-1;
                 int end=block[p];
                 //cout<<"now i will find out the useless ins"<<endl;
                 while(1)
                 {
                     cur_ins=ins[t];
                     if(relation[cur_ins.RD]==1)
                     {
                         relation[cur_ins.RS1]=1;
                         relation[cur_ins.RS2]=1;
                         t--;
                         continue;
                     }
                     else 
                         relation[cur_ins.RD]=0;
                     cur_ins.useful=false;
                     ins[t]=cur_ins;
                     //cout<<"i find pc="<<t<<"is useless"<<endl;
                     t--;
                     if(t<=end)
                     break;
                 }
                 k=0;
                 t=p-1;
                 while(t>=end)
                 {
                     cur_ins=ins[t];
                     if(cur_ins.useful==false)
                     {
                         for(j=t;j>block[p];j--)
                         {
                             ins[j]=ins[j-1];
                         }
                         ins[block[p]]=cur_ins;
                         end++;
                         continue;
                     }
                     t--;
                 }
                 //block[p]=end;
                 cur_ins=ins[p];
                 if(it==lable_map.end())
                    //imm
                    cur_ins.imm=to_string(end);
                 else//lable
                    lable_map[cur_ins.imm]=end;
                 //cout<<"block[p]="<<end<<"="<<block[p]<<endl;
                 //cout<<"lable_map["<<cur_ins.imm<<"]="<<lable_map[cur_ins.imm]<<endl;
            }
            p--;
        }
    }

    //read memory
    if(readmem)
        read_mem(filename[1]);
    

    //
    int pc=0;
    int reg[32];
    memset(reg,0,sizeof(reg));
    bool power=true;
    
    while(power)
    {
        instruction cur_ins=ins[pc];//fetch
        switch(cur_ins.opcode)
        {
            case 0://halt
                power=false;
                break;
            case 1://add
                reg[cur_ins.RD]=reg[cur_ins.RS1]+reg[cur_ins.RS2];
                break;
            case 101://addi
                reg[cur_ins.RD]=reg[cur_ins.RS1]+cur_ins.RS2;
                break;
            case 2://sub
                reg[cur_ins.RD]=reg[cur_ins.RS1]-reg[cur_ins.RS2];
                break;
            case 102://subi
                reg[cur_ins.RD]=reg[cur_ins.RS1]-cur_ins.RS2;
                break;
            case 3://and
                reg[cur_ins.RD]=reg[cur_ins.RS1]&reg[cur_ins.RS2];
                break;
            case 103://andi
                reg[cur_ins.RD]=reg[cur_ins.RS1]&cur_ins.RS2;
                break;
            case 4://or
                reg[cur_ins.RD]=reg[cur_ins.RS1]|reg[cur_ins.RS2];
                break;
            case 104://ori
                reg[cur_ins.RD]=reg[cur_ins.RS1]|cur_ins.RS2;
                break;
            case 5://xor
                reg[cur_ins.RD]=reg[cur_ins.RS1]^reg[cur_ins.RS2];
                break;
            case 105://xori
                reg[cur_ins.RD]=reg[cur_ins.RS1]^cur_ins.RS2;
                break;
            case 6://sgt
                reg[cur_ins.RD]=reg[cur_ins.RS1]>reg[cur_ins.RS2]?1:0;
                break;
            case 106://sgti
                 if(optflag){  
                        reg[cur_ins.RS2]=reg[cur_ins.RS1]+1;
                        //cout<<"opt ok"
                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]>cur_ins.RS2?1:0;
                break;
            case 7://seq
                 if(optflag){  
                        reg[cur_ins.RS2]=reg[cur_ins.RS1];
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]==reg[cur_ins.RS2]?1:0;
                break;
            case 107://seqi
                 if(optflag){  
                        reg[cur_ins.RS2]=cur_ins.RS1;
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]==cur_ins.RS2?1:0;
                break;
            case 8://sge
                 if(optflag){  
                        reg[cur_ins.RS2]=reg[cur_ins.RS1]+1;
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]>=reg[cur_ins.RS2]?1:0;
                break;
            case 108://sgei
                 if(optflag){  
                        reg[cur_ins.RS2]=cur_ins.RS1+1;
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]>=cur_ins.RS2?1:0;
                break;
            case 9://slt
                 if(optflag){  
                        reg[cur_ins.RS1]=reg[cur_ins.RS2];
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]<reg[cur_ins.RS2]?1:0;
                break;
            case 109://slti
                 if(optflag){  
                        reg[cur_ins.RS1]=cur_ins.RS2;
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]<cur_ins.RS2?1:0;
                break;
            case 10://sne
                reg[cur_ins.RD]=reg[cur_ins.RS1]==reg[cur_ins.RS2]?0:1;
                break;
            case 110://snei
                reg[cur_ins.RD]=reg[cur_ins.RS1]==cur_ins.RS2?0:1;
                break;
            case 11://sle
                 if(optflag){  
                        reg[cur_ins.RS1]=reg[cur_ins.RS2]+1;
                        //cout<<"opt ok"<<endl;

                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]<=reg[cur_ins.RS2]?1:0;
                break;
            case 111://slei
                 if(optflag){  
                        reg[cur_ins.RS1]=cur_ins.RS2+1;
                 }
                reg[cur_ins.RD]=reg[cur_ins.RS1]<=cur_ins.RS2?1:0;
                break;
            case 112://slli
                reg[cur_ins.RD]=reg[cur_ins.RS1]<<cur_ins.RS2;
                break;
            case 113://srli
                reg[cur_ins.RD]=(unsigned int)reg[cur_ins.RS1]>>cur_ins.RS2;
                break;
            case 201://lb
                reg[cur_ins.RD]=Read_mem_byte(reg[cur_ins.RS1]+cur_ins.RS2);
                break;
            case 202://lw
                reg[cur_ins.RD]=Read_mem_int(reg[cur_ins.RS1]+cur_ins.RS2);
                break;
            case 203://sb
                Write_mem_byte(reg[cur_ins.RS1]+cur_ins.RS2,reg[cur_ins.RD]);
                break;
            case 204://sw
                Write_mem_int(reg[cur_ins.RS1]+cur_ins.RS2,reg[cur_ins.RD]);
                break;
            case 301://beqz
                if(reg[cur_ins.RD]==0)
                {
                    it=lable_map.find(cur_ins.imm);
                    if(it==lable_map.end())//imm
                        pc+=atoi(cur_ins.imm.c_str());
                    else//lable
                        pc=lable_map[cur_ins.imm]-1;
                }
                break;
            case 302://bnez
                if(reg[cur_ins.RD]!=0)
                {
                    it=lable_map.find(cur_ins.imm);
                    if(it==lable_map.end())
                    //imm
                        pc+=atoi(cur_ins.imm.c_str());
                    else//lable
                        pc=lable_map[cur_ins.imm]-1;
                }
                break;
            case 303://jr
                it=lable_map.find(cur_ins.imm);
                if(it==lable_map.end())//can't find
                    pc+=reg[atoi(cur_ins.imm.c_str())];//to an ins
                else//find it
                    pc=lable_map[cur_ins.imm]-1;
                break;
            case 401://op mem
                cout<<Read_mem_int(reg[cur_ins.RD]+cur_ins.RS1)<<endl;
                break;
            case 402://op reg
                cout<<reg[cur_ins.RD]<<endl; 
                break;


        }
        //cout<<"PC="<<pc<<" OP"<<cur_ins.opcode<<"+R"<<cur_ins.RD<<"+R"<<cur_ins.RS1<<"+R"<<cur_ins.RS2<<endl;
        //if(cur_ins.opcode==0)
        //power=false;
        pc++;
        
    }

    //cout<<"all work has been done";
    fclose(stdout);
    return 0;
}
