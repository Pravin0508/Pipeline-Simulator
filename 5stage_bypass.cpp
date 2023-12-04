#include"bits/stdc++.h"
#include <boost/tokenizer.hpp>
using namespace std;
#define MemSize 1024*1024
int registers[32] = {0};
int haz_poss[32]={0};
std::unordered_map<std::string, int> registerMap, address;
std::vector<std::vector<std::string>> commands;
int dataMem[MemSize >> 2] = {0};
struct IFStruct {
    int  PC;
    bool nop;
};
struct IDStruct {
    vector<string> Instr;
    bool nop;  
};
struct EXStruct {
	string reg1,reg2;
    string   brnch;
    string   Wrt_reg_addr;
	string op;
	int val1,val2,val3;
	bool get1,get2,get3;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop;  
    map<string,int> latch;
};
struct MEMStruct {
    int ALUresult;
	int val_write;
    string   Wrt_reg_addr;
	bool get;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
    map<string,int> latch;
};
struct WBStruct {
    int  Wrt_data;
	string  Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};
struct stateStruct{
    IFStruct IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
} ;
pair<int,string>locateAddress(std::string location){
	int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
	std::string reg = location.substr(lparen + 1,location.length()-lparen-2);
	return {offset,reg};
}
int ans(string op,int a,int b){
	if(op=="mul")return a*b;
	if(op=="add")return a+b;
	if(op=="sub")return a-b;
	if(op=="beq")return a==b;
	return a!=b;
}
void store_data(int address,int data){
	dataMem[address]=data;
}
void parseCommand(std::string line){
	line = line.substr(0, line.find('#'));
	std::vector<std::string> command;
	boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
	for (auto &s : tokens)command.push_back(s);
    if (command.empty())return;
	else if (command.size() == 1)
	{
		std::string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
		if (address.find(label) == address.end())address[label] = commands.size();
		else address[label] = -1;
		command.clear();
	}
	else if (command[0].back() == ':'){		
		std::string label = command[0].substr(0, command[0].size() - 1);
		if (address.find(label) == address.end())address[label] = commands.size();
		else address[label] = -1;
		command = std::vector<std::string>(command.begin() + 1, command.end());
	}
	else if (command[0].find(':') != std::string::npos){
		int idx = command[0].find(':');
		std::string label = command[0].substr(0, idx);
		if (address.find(label) == address.end())address[label] = commands.size();
		else address[label] = -1;
		command[0] = command[0].substr(idx + 1);
	}
	else if (command[1][0] == ':'){
		if (address.find(command[0]) == address.end())address[command[0]] = commands.size();
		else address[command[0]] = -1;
		command[1] = command[1].substr(1);
		if (command[1] == "") command.erase(command.begin(), command.begin() + 2);
		else command.erase(command.begin(), command.begin() + 1);
	}
	if (command.empty())return;
	if (command.size() > 4)
		for (int i = 4; i < (int)command.size(); ++i)
			command[3] += " " + command[i];
	command.resize(4);
	commands.push_back(command);
}
void init_reg(){
	for (int i = 0; i < 32; ++i)registerMap["$" + std::to_string(i)] = i;
	registerMap["$zero"] = 0;
	registerMap["$at"] = 1;
	registerMap["$v0"] = 2;
	registerMap["$v1"] = 3;
	for (int i = 0; i < 4; ++i)registerMap["$a" + std::to_string(i)] = i + 4;
	for (int i = 0; i < 8; ++i)registerMap["$t" + std::to_string(i)] = i + 8, registerMap["$s" + std::to_string(i)] = i + 16;
	registerMap["$t8"] = 24;
	registerMap["$t9"] = 25;
	registerMap["$k0"] = 26;
	registerMap["$k1"] = 27;
	registerMap["$gp"] = 28;
	registerMap["$sp"] = 29;
	registerMap["$s8"] = 30;
	registerMap["$ra"] = 31;
}
void printstate(stateStruct state){
    cout<<state.IF.nop<<" "<<state.IF.PC<<"\n";
    cout<<state.ID.nop<<" ";
    for(auto v:state.ID.Instr)cout<<v<<" ";
    cout<<"\n";
    cout<<state.EX.nop<<" "<<state.EX.op<<" "<<state.EX.reg1<<" "<<state.EX.reg2<<" ";
    cout<<state.EX.val1<<" "<<state.EX.val2<<" "<<state.EX.val3<<" "<<state.EX.get1<<" ";
    cout<<state.EX.get2<<" "<<state.EX.get3<<" "<<state.EX.val3<<" "<<state.EX.get1<<" ";
    cout<<state.EX.wrt_enable<<" "<<state.EX.wrt_mem<<" "<<state.EX.rd_mem<<" "<<state.EX.Wrt_reg_addr<<" "<<state.EX.brnch<<"\n";
    cout<<"alu latch\n";
    for(auto v:state.EX.latch)cout<<"              "<<v.first<<" "<<v.second<<"\n";
    cout<<state.MEM.nop<<" "<<state.MEM.ALUresult<<" "<<state.MEM.val_write<<" "<<state.MEM.Wrt_reg_addr<<" ";
    cout<<state.MEM.wrt_enable<<" "<<state.MEM.wrt_mem<<" "<<state.MEM.rd_mem<<"\n";
    cout<<"mem latch\n";
    for(auto v:state.MEM.latch)cout<<"              "<<v.first<<" "<<v.second<<"\n";
    cout<<state.WB.nop<<" "<<state.WB.Wrt_data<<" "<<state.WB.Wrt_reg_addr<<" "<<state.WB.wrt_enable<<"\n";
}
int main(){
    std::ifstream file("input.asm");
    // ofstream filw("result.txt");
    std::string line;
	while (getline(file, line))parseCommand(line);
	file.close();
	stateStruct state, newState;
    state.IF.PC=0;
    state.IF.nop=0;
	state.ID.nop=1;
	state.ID.Instr={};
    state.EX.nop=1;
	state.EX.reg1="";
	state.EX.reg2="";
    state.EX.brnch="";
	state.EX.op="";
	state.EX.val1=0;
	state.EX.val2=0;
	state.EX.val3=0;
	state.EX.get1=0;
	state.EX.get2=0;
    state.EX.get3=0;
	state.EX.wrt_enable=0;
	state.EX.wrt_mem=0;
	state.EX.rd_mem=0;
	state.EX.Wrt_reg_addr="";
    state.MEM.nop=state.WB.nop=1;
	state.MEM.ALUresult=0;
	state.MEM.val_write=0;
	state.MEM.rd_mem=0;
	state.MEM.wrt_enable=0;
	state.MEM.wrt_mem=0;
	state.MEM.Wrt_reg_addr="";
    state.MEM.get=0;
	// state.WB.Wrt_data=0;
	// state.WB.wrt_enable=0;
	// state.WB.Wrt_reg_addr="";
    init_reg();
	int cycle=0;
	vector<string>instr;
    for(int i=0;i<32;i++)cout<<registers[i]<<" ";
	cout<<"\n";
    cout<<"0\n";
    while(true){
        int haz=0;
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop) {
            break;	
		}
        newState=state;
		newState.MEM.latch.clear();
		newState.EX.latch.clear();
        // cout<<state.IF.nop<<" "<<state.ID.nop<<" "<<state.EX.nop<<" "<<state.MEM.nop<<" "<<state.WB.nop<<"\n";
        cycle++;
		// cout<<"cycle number is: "<<cycle<<"\n";
		// printstate(state);

        vector<pair<int,int>> memc;
        /* --------------------- WB stage --------------------- */
		if ((!state.WB.nop) && state.WB.wrt_enable){
			haz_poss[registerMap[state.WB.Wrt_reg_addr]]=haz_poss[registerMap[state.WB.Wrt_reg_addr]]-1;
			registers[registerMap[state.WB.Wrt_reg_addr]] = state.WB.Wrt_data;
		}
        /* --------------------- MEM stage --------------------- */
		newState.WB.nop = state.MEM.nop;
		if (!state.MEM.nop){
			if (state.MEM.wrt_mem){
				// cout<<"yesbaby\n";
				int data;
                bool get=false;
				if(state.MEM.get==true)data=state.MEM.val_write,get=true;
                if(!get)if(haz_poss[registerMap[state.MEM.Wrt_reg_addr]]==0)data=registers[registerMap[state.MEM.Wrt_reg_addr]],get=true; 
				if(!get)for(auto v:state.EX.latch)if(v.first==state.EX.Wrt_reg_addr)data=v.second,get=true;
				if(!get)for(auto v:state.MEM.latch)if(v.first==state.EX.Wrt_reg_addr)data=v.second,get=true;
				if(get){
					store_data(state.MEM.ALUresult/4,data);
					memc.push_back({state.MEM.ALUresult/4,data});
				}
				else{
					haz++;
					newState.MEM=state.MEM;
					newState.WB.nop=1;
				}
			}
			else if (state.MEM.rd_mem){
				newState.WB.Wrt_data = dataMem[state.MEM.ALUresult / 4];
				newState.MEM.latch[state.MEM.Wrt_reg_addr]=dataMem[state.MEM.ALUresult/4];
			}
            else if(state.MEM.wrt_enable) {
				newState.WB.Wrt_data = state.MEM.ALUresult;
				newState.MEM.latch[state.MEM.Wrt_reg_addr]=state.MEM.ALUresult;
			}
			else{
				// cout<<"abra ka dabra";
			}

			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.wrt_enable = state.MEM.wrt_enable;

		}
        /* --------------------- EX stage --------------------- */
		if(haz==0){
			newState.MEM.nop = state.EX.nop;
			if (!state.EX.nop){
				newState.MEM.rd_mem=state.EX.rd_mem;
				newState.MEM.wrt_enable=state.EX.wrt_enable;
				newState.MEM.wrt_mem=state.EX.wrt_mem;
				newState.MEM.Wrt_reg_addr=state.EX.Wrt_reg_addr;
				newState.MEM.get=0;
				if(state.EX.rd_mem==0 && state.EX.wrt_mem==0 && state.EX.wrt_enable==0){
					bool get1=state.EX.get1,get2=state.EX.get2;
					int val1,val2;
					if(get1)val1=state.EX.val1;
					if(!get1)if(haz_poss[registerMap[state.EX.reg1]]==0)val1=registers[registerMap[state.EX.reg1]],get1=true;
					if(!get1)for(auto v:state.EX.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(!get1)for(auto v:state.MEM.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(get2)val2=state.EX.val2;
					if(!get2)if(haz_poss[registerMap[state.EX.reg2]]==0)val2=registers[registerMap[state.EX.reg2]],get2=true;
					if(!get2)for(auto v:state.EX.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(!get2)for(auto v:state.MEM.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(get1 && get2){
						haz=-1;
						newState.IF.nop=0;
						newState.EX.nop=1;
						if(state.EX.op=="beq"){
							if(val1==val2)newState.IF.PC=address[state.EX.brnch];
						}
						else{
							if(val1!=val2)newState.IF.PC=address[state.EX.brnch];
						}
					}
					else{
						haz++;
						newState.EX=state.EX;
						newState.MEM.nop=1;
					}
				}
				else if(state.EX.op=="addi"){
					int val1,val2=state.EX.val2,get1=state.EX.get1;
					if(get1)val1=state.EX.val1;
					if(!get1)if(haz_poss[registerMap[state.EX.reg1]]==0)val1=registers[registerMap[state.EX.reg1]],get1=true;
					if(!get1)for(auto v:state.EX.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(!get1)for(auto v:state.MEM.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(get1){
						newState.MEM.ALUresult=val1+val2;
						newState.MEM.val_write=newState.MEM.ALUresult;
						newState.EX.latch[state.EX.Wrt_reg_addr]=val1+val2;
					}
					else{
						haz++;
						newState.EX=state.EX;
						newState.MEM.nop=1;
					}
				}
				else if(state.EX.op=="add" || state.EX.op=="sub"|| state.EX.op=="mul"){
					// if(cycle==16)cout<<"jaisrm\n";
					int val1,val2;
					bool get1=state.EX.get1,get2=state.EX.get2;
					// cout<<state.EX.reg1<<" "<<state.EX.reg2<<"\n";
					// cout<<haz_poss[registerMap["$7"]]<<"\n";
					if(get1)val1=state.EX.val1;
					if(!get1)if((haz_poss[registerMap[state.EX.reg1]]==0)||(haz_poss[registerMap[state.EX.reg1]]==1 && state.EX.Wrt_reg_addr==state.EX.reg1))val1=registers[registerMap[state.EX.reg1]],get1=true; 
					if(!get1)for(auto v:state.EX.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(!get1)for(auto v:state.MEM.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(get2)val2=state.EX.val2;
					if(!get2)if((haz_poss[registerMap[state.EX.reg2]]==0)||(haz_poss[registerMap[state.EX.reg2]]==1 && state.EX.Wrt_reg_addr==state.EX.reg2))val2=registers[registerMap[state.EX.reg2]],get2=true;
					if(!get2)for(auto v:state.EX.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(!get2)for(auto v:state.MEM.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(get1 && get2){
						newState.MEM.ALUresult=ans(state.EX.op,val1,val2);
						newState.MEM.val_write=newState.MEM.ALUresult;
						newState.EX.latch[state.EX.Wrt_reg_addr]=val1+val2;
					}
					else{
						haz++;
					    newState.EX=state.EX;
					    newState.MEM.nop=1;
					}
				}
				else if(state.EX.op=="lw"){
					int val1,val2;
					bool get1=state.EX.get1,get2=state.EX.get2;
					if(get1)val1=state.EX.val1;
					if(!get1)if(haz_poss[registerMap[state.EX.reg1]]==0)val1=registers[registerMap[state.EX.reg1]],get1=true;
					if(!get1)for(auto v:state.EX.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(!get1)for(auto v:state.MEM.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(get2)val2=state.EX.val2;
					if(!get2)if(haz_poss[registerMap[state.EX.reg2]]==0)val2=registers[registerMap[state.EX.reg2]],get2=true;
					if(!get2)for(auto v:state.EX.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(!get2)for(auto v:state.MEM.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(get1 && get2){
						newState.MEM.ALUresult=val1+val2;
					}
					else{
						haz++;
						newState.EX=state.EX;
						newState.MEM.nop=1;
					}
				}
				else if(state.EX.op=="sw"){
					int val1,val2;
					bool get1=state.EX.get1,get2=state.EX.get2;
					// cout<<get1<<" "<<get2<<" "<<state.EX.reg1<<" "<<state.EX.reg2<<"\n";
					if(get1)val1=state.EX.val1;
					if(!get1)if(haz_poss[registerMap[state.EX.reg1]]==0)val1=registers[registerMap[state.EX.reg1]],get1=true;
					if(!get1)for(auto v:state.EX.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(!get1)for(auto v:state.MEM.latch)if(v.first==state.EX.reg1)val1=v.second,get1=true;
					if(get2)val2=state.EX.val2;
					if(!get2)if(haz_poss[registerMap[state.EX.reg2]]==0)val2=registers[registerMap[state.EX.reg2]],get2=true;
					if(!get2)for(auto v:state.EX.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(!get2)for(auto v:state.MEM.latch)if(v.first==state.EX.reg2)val2=v.second,get2=true;
					if(get1 && get2){
						newState.MEM.ALUresult=val1+val2;
						if(state.EX.get3)newState.MEM.get=1,newState.MEM.val_write=state.EX.val3;
						else if(haz_poss[registerMap[state.EX.Wrt_reg_addr]]==0){
							newState.MEM.get=true,newState.MEM.val_write=registers[registerMap[state.EX.Wrt_reg_addr]];
						}
					}
				}
				else{
					haz++;
					newState.EX=state.EX;
					newState.MEM.nop=1;
                }
			}	
		}
        
        /* --------------------- ID stage --------------------- */
		if(haz==0){
			newState.EX.nop=state.ID.nop;
			if(!state.ID.nop){
				newState.EX.rd_mem = false;
				newState.EX.wrt_mem = false;
				newState.EX.wrt_enable = false;
				newState.EX.reg1="";
				newState.EX.reg2="";
				newState.EX.val1=0;
				newState.EX.val2=0;
				newState.EX.val3=0;
				newState.EX.get1=0;
				newState.EX.get2=0;
				newState.EX.get3=0;
				instr=state.ID.Instr;	
				newState.EX.op=instr[0];
				if(instr[0]=="addi"){
					newState.EX.Wrt_reg_addr=instr[1];
					newState.EX.reg1=instr[2];
					if(!haz_poss[registerMap[instr[2]]])newState.EX.val1=registers[registerMap[instr[2]]],newState.EX.get1=1;
					newState.EX.val2=stoi(instr[3]);
					newState.EX.get2=1;
					newState.EX.wrt_enable=true;
				}
				else if(instr[0]=="add" ||instr[0]=="sub" ||instr[0]=="mul"){
					newState.EX.Wrt_reg_addr=instr[1];
					newState.EX.wrt_enable=true;
					newState.EX.reg1=instr[2];
					newState.EX.reg2=instr[3];	
					if(haz_poss[registerMap[instr[2]]]==0)newState.EX.val1=registers[registerMap[instr[2]]],newState.EX.get1=1;
					if(haz_poss[registerMap[instr[3]]]==0)newState.EX.val2=registers[registerMap[instr[3]]],newState.EX.get2=1;
					// if(cycle==16)cout<<"there there\n";
				}
				else if (instr[0] == "sw") {
					pair<int,string>ab=locateAddress(instr[2]);
					newState.EX.Wrt_reg_addr = instr[1];
					newState.EX.reg1=ab.second;
					newState.EX.val2=ab.first;
					newState.EX.get2=true;
					newState.EX.wrt_mem = true;        
					if(!haz_poss[registerMap[instr[1]]])newState.EX.val3=registers[registerMap[instr[1]]],newState.EX.get3=1;
					if(!haz_poss[registerMap[ab.second]])newState.EX.val1=registers[registerMap[ab.second]],newState.EX.get1=1;
				}
				else if (instr[0] == "lw") {
					pair<int,string>ab=locateAddress(instr[2]);
					newState.EX.Wrt_reg_addr = instr[1];
					newState.EX.reg1=ab.second;
					newState.EX.val2=ab.first;
					newState.EX.get2=true;
					newState.EX.rd_mem = true; 
					newState.EX.wrt_enable=true;       
					if(!haz_poss[registerMap[ab.second]])newState.EX.val1=registers[registerMap[ab.second]],newState.EX.get1=1;
				}			
				else if(instr[0]=="beq" || instr[0]=="bne"){
					newState.EX.brnch=instr[3];
					newState.IF.nop=1;
					state.IF.nop=1;
					newState.EX.reg1=instr[1];
					newState.EX.reg2=instr[2];
					if(!haz_poss[registerMap[instr[1]]])newState.EX.val1=registers[registerMap[instr[1]]],newState.EX.get1=1;
					if(!haz_poss[registerMap[instr[2]]])newState.EX.val2=registers[registerMap[instr[2]]],newState.EX.get2=1;
				}
				else{
					newState.IF.PC=address[instr[1]];
					newState.IF.nop=0;
					newState.EX.nop=1;
					newState.ID.nop=1;
				}
				if(newState.EX.wrt_enable==true){
					// newState.EX.latch[newState.EX.Wrt_reg_addr]=registers[registerMap[newState.EX.Wrt_reg_addr]];
					haz_poss[registerMap[newState.EX.Wrt_reg_addr]]=haz_poss[registerMap[newState.EX.Wrt_reg_addr]]+1;
				}
			}
		}
		else  {
			newState.ID=state.ID;
		}
        /*------------- IF stage --------------------- */
		if(haz==0){
			newState.ID.nop=state.IF.nop;
			if(!state.IF.nop){
				newState.ID.Instr=commands[state.IF.PC];
				newState.IF.PC=state.IF.PC+1;
				if (newState.IF.PC>=commands.size()||newState.ID.Instr[0]=="exit"){  
					newState.IF.PC=state.IF.PC;
					newState.IF.nop=1;
				}
			}
		}
		else if(haz!=-1)newState.IF=state.IF;
		if(newState.IF.PC>=commands.size())newState.IF.nop=1 ;
        state=newState;
		for(int i=0;i<32;i++)cout<<registers[i]<<" ";
		cout<<"\n";
		cout<<memc.size()<<" ";
		for(auto v:memc)cout<<v.first<<" "<<v.second<<" ";
		cout<<"\n";    
		if(cycle==1000)break;
    }
}
