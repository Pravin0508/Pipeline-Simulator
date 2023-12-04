#include"bits/stdc++.h"
#include <boost/tokenizer.hpp>
using namespace std;
#define MemSize 1024*1024
int registers[32] = {0};
int haz_poss[32]={0};
bool haz_curr[32]={0};
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
    int Read_data1;
    int  Read_data2;
    string   brnch;
    string   Wrt_reg_addr;
	string op;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop;  
};
int nonbrn=-1;
struct MEMStruct {
    int ALUresult;
    string   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};
struct WBStruct {
    int  Wrt_data;
	string  Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};
struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};
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
void store_data(int address,string reg){
	dataMem[address]=registers[registerMap[reg]];
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
    cout<<state.EX.nop<<" "<<state.EX.op<<" "<<state.EX.Read_data1<<" "<<state.EX.Read_data2<<" ";
    cout<<state.EX.rd_mem<<" "<<state.EX.wrt_enable<<" "<<state.EX.wrt_mem<<" "<<state.EX.Wrt_reg_addr<<" "<<state.EX.brnch<<"\n";
    cout<<state.MEM.nop<<" "<<state.MEM.ALUresult<<" "<<state.MEM.Wrt_reg_addr<<" ";
    cout<<state.MEM.wrt_enable<<" "<<state.MEM.wrt_mem<<" "<<state.MEM.rd_mem<<"\n";
    cout<<state.WB.nop<<" "<<state.WB.Wrt_data<<" "<<state.WB.Wrt_reg_addr<<" "<<state.WB.wrt_enable<<"\n";
}
int main(){
    std::ifstream file("input.asm");
    ofstream filw("result.txt");
    std::string line;
	while (getline(file, line))parseCommand(line);
	file.close();
	// for(auto v:commands){
	// 	for(auto u:v)cout<<u<<" ";
	// 	cout<<"\n";
	// }
	// for(auto v:address)cout<<v.first<<" "<<v.second<<"\n";
	stateStruct state, newState,temp;
    state.IF.PC=0;
    state.IF.nop=0;
	state.ID.nop=state.EX.nop=state.MEM.nop=state.WB.nop=1;
	state.ID.Instr={};
	state.EX.rd_mem=0;
	state.EX.Read_data1=0;
	state.EX.Read_data2=0;
    state.EX.brnch="";
	state.EX.op="";
	state.EX.wrt_enable=0;
	state.EX.wrt_mem=0;
	state.EX.Wrt_reg_addr="";
	state.WB.Wrt_data=0;
	state.WB.wrt_enable=0;
	state.WB.Wrt_reg_addr="";
    init_reg();
	int cycle=0;
	vector<string>instr;
    temp=state;
    temp.IF.nop=1;
    for(int i=0;i<32;i++)cout<<registers[i]<<" ";
	cout<<"\n";
    cout<<"0\n";
    while(true){
        int haz=0;
        if (state.IF.nop  && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop) {
            break;	
		}
		// cout<<state.IF.nop<<" "<<state.ID.nop<<" "<<state.EX.nop<<" "<<state.MEM.nop<<" "<<state.WB.nop<<"\n";
        newState=state;
        cycle++;
		// cout<<"cycle is "<<cycle<<"\n";
		// printstate(state);
        vector<pair<int,int>> memc;
        /* --------------------- WB stage --------------------- */
		if ((!state.WB.nop) && state.WB.wrt_enable){
			haz_poss[registerMap[state.WB.Wrt_reg_addr]]--;
			registers[registerMap[state.WB.Wrt_reg_addr]] = state.WB.Wrt_data;
		}
        /* --------------------- MEM stage --------------------- */
		newState.WB.nop = state.MEM.nop;
		if (!state.MEM.nop){
			if (state.MEM.wrt_mem){
				store_data(state.MEM.ALUresult / 4, state.MEM.Wrt_reg_addr);
				memc.push_back({state.MEM.ALUresult / 4, dataMem[state.MEM.ALUresult / 4]});
			}
			else if (state.MEM.rd_mem)newState.WB.Wrt_data = dataMem[state.MEM.ALUresult / 4];
            else newState.WB.Wrt_data = state.MEM.ALUresult;
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.wrt_enable = state.MEM.wrt_enable;

		}
        /* --------------------- EX stage --------------------- */
		newState.MEM.nop = state.EX.nop;
		if (!state.EX.nop){
            if(state.EX.rd_mem==0 && state.EX.wrt_mem==0 && state.EX.wrt_enable==0){
               
                if(state.EX.op=="beq"){
					 haz=-1;
                newState.IF.nop=0;
                    if(state.EX.Read_data1==state.EX.Read_data2)newState.IF.PC=address[state.EX.brnch];
                }
                else if(state.EX.op=="bne"){
					 haz=-1;
                newState.IF.nop=0;
                    if(state.EX.Read_data1!=state.EX.Read_data2)newState.IF.PC=address[state.EX.brnch];
                }
				else{

				}
            }
            else{
                newState.MEM.ALUresult = ans(state.EX.op, state.EX.Read_data1, state.EX.Read_data2);
            }
            newState.MEM.rd_mem = state.EX.rd_mem;
			newState.MEM.wrt_mem = state.EX.wrt_mem;
			newState.MEM.wrt_enable = state.EX.wrt_enable;
			newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
		}
        /* --------------------- ID stage --------------------- */
        newState.EX.nop=state.ID.nop;
        if(!state.ID.nop){
            instr=state.ID.Instr;	
			newState.EX.op=instr[0];
			if(instr[0]=="addi"){
				newState.EX.op="add";
				newState.EX.Wrt_reg_addr=instr[1];
				newState.EX.Read_data1=registers[registerMap[instr[2]]];
				newState.EX.Read_data2=stoi(instr[3]);
				newState.EX.rd_mem=false;
				newState.EX.wrt_mem=false;
				newState.EX.wrt_enable=true;
				if(haz_poss[registerMap[instr[2]]]){
					haz_curr[registerMap[instr[2]]]=1;
					newState.ID.nop=1;
					newState.IF.nop=1;
				}
			}
			else if(instr[0]=="add" ||instr[0]=="sub" ||instr[0]=="mul"){
				newState.EX.Wrt_reg_addr=instr[1];
				newState.EX.Read_data1=registers[registerMap[instr[2]]];
				newState.EX.Read_data2=registers[registerMap[instr[3]]];
				newState.EX.rd_mem=false;
				newState.EX.wrt_mem=false;
				newState.EX.wrt_enable=true;
                if(haz_poss[registerMap[instr[2]]] ){
					newState.EX.nop=1;
                    newState.ID=state.ID;
                    haz++;
				}
                if(haz_poss[registerMap[instr[3]]] && instr[3]!=instr[2]){
					newState.EX.nop=1;
                    newState.ID=state.ID;
                    haz++;
				}
			}
			else if (instr[0] == "lw") {
				newState.EX.op="add";
				newState.EX.Wrt_reg_addr = instr[1];
				pair<int,string>ab=locateAddress(instr[2]);
				newState.EX.Read_data1 = registers[registerMap[ab.second]];
				newState.EX.Read_data2 = ab.first;
				newState.EX.rd_mem = true;
				newState.EX.wrt_mem = false;
				newState.EX.wrt_enable = true;        
				if(haz_poss[registerMap[ab.second]] ){
					newState.EX.nop=1;
                    newState.ID=state.ID;
                    haz++;
				}
			}
			else if (instr[0] == "sw") {
				newState.EX.op="add";
				pair<int,string>ab=locateAddress(instr[2]);
				newState.EX.Read_data1 = registers[registerMap[ab.second]];
				newState.EX.Read_data2 = ab.first;
				newState.EX.Wrt_reg_addr = instr[1];
				newState.EX.rd_mem = false;
				newState.EX.wrt_mem = true;
				newState.EX.wrt_enable = false;
				if(haz_poss[registerMap[ab.second]] ){
					newState.EX.nop=1;
                    newState.ID=state.ID;
                    haz++;
				}
                if(haz_poss[registerMap[instr[1]]] && instr[1]!=ab.second ){
					newState.EX.nop=1;
                    newState.ID=state.ID;
                    haz++;
				}
			}		
			else if(instr[0]=="j"){
				newState.EX.op=instr[0];
                newState.EX.brnch=instr[1];
                state.IF.nop=1;
				newState.EX.Wrt_reg_addr = "";
				// newState.EX.Read_data1 = registers[registerMap[instr[1]]];
				// newState.EX.Read_data2 = registers[registerMap[instr[2]]];
				newState.EX.rd_mem = false;
				newState.EX.wrt_mem = false;
				newState.EX.wrt_enable = false;
				newState.IF.PC=address[instr[1]];
				newState.IF.nop=0;
                // if(haz_poss[registerMap[instr[1]]] ||haz_poss[registerMap[instr[2]]]){
					// newState.EX.nop=1;
                    // newState.ID=state.ID;
                    // haz++;
				// }
			}	
			else {
				newState.EX.op=instr[0];
                newState.EX.brnch=instr[3];
                state.IF.nop=1;
				newState.EX.Wrt_reg_addr = "";
				newState.EX.Read_data1 = registers[registerMap[instr[1]]];
				newState.EX.Read_data2 = registers[registerMap[instr[2]]];
				newState.EX.rd_mem = false;
				newState.EX.wrt_mem = false;
				newState.EX.wrt_enable = false;
                if(haz_poss[registerMap[instr[1]]] ||haz_poss[registerMap[instr[2]]]){
					newState.EX.nop=1;
                    newState.ID=state.ID;
                    haz++;
				}
			}
			if(newState.EX.nop==0){
                if(newState.EX.wrt_enable==true){
                    haz_poss[registerMap[newState.EX.Wrt_reg_addr]]++;
                }
            }
		}
         /* --------------------- IF stage --------------------- */
        
        if(haz==0){
            newState.ID.nop=state.IF.nop;
            if(!state.IF.nop){
                newState.ID.Instr=commands[state.IF.PC];
                newState.IF.PC=state.IF.PC+1;
                if (newState.IF.PC>=commands.size()||newState.ID.Instr[0]=="exit"){  
                    newState.IF.PC=state.IF.PC; //PC remains the same//
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
