#include"bits/stdc++.h"
#include <boost/tokenizer.hpp>
using namespace std;
#define MemSize 1024*1024
int registers[32] = {0};
int haz_poss[32]={0};
int haz_curr[32]={0};
std::unordered_map<std::string, int> registerMap, address;
std::vector<std::vector<std::string>> commands;
int dataMem[MemSize >> 2] = {0};
struct IF1Struct {
    int  PC;
    bool nop;  
};
struct IF2Struct {
    int  PC;
    bool nop;  
};
struct ID1Struct {
    vector<string> Instr;
    bool nop;  
};
struct ID2Struct {
    vector<string> Instr;
    bool nop;  
};
struct RRStruct{
    string op;
    string reg1;
    string reg2;
    string reg3;
    string brnch;
    int imm;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop; 
};
struct EXStruct {
    int val1;
    int val2;
    int val3;
    // int  Read_data2;
    string   brnch;
    string   Wrt_reg_addr;
	string op;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop;  
};
struct MM1Struct{
    int val1;
    int val2;
    string   Wrt_reg_addr;
    string op;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop;  
};
struct MM2Struct{
    int val1;
    int val2;
    string   Wrt_reg_addr;
    string op;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop;  
};
// int nonbrn=-1;
// struct MEMStruct {
//     int ALUresult;
//     string   Wrt_reg_addr;
//     bool        rd_mem;
//     bool        wrt_mem; 
//     bool        wrt_enable;    
//     bool        nop;    
// };
struct WBStruct {
    int  Wrt_data;
	string  Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};
struct stateStruct {
    IF1Struct    IF1;
    IF2Struct    IF2;
    ID1Struct    ID1;
    ID2Struct    ID2;
    RRStruct     RR;
    EXStruct     EX;
    MM1Struct   MM1;
    MM2Struct   MM2;
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
void store_data(int address,int reg){
	dataMem[address]=reg;
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
    cout<<state.IF1.nop<<" "<<state.IF1.PC<<"\n";
    cout<<state.IF2.nop<<" "<<state.IF2.PC<<"\n";
    cout<<state.ID1.nop<<" ";
    for(auto v:state.ID1.Instr)cout<<v<<" ";
    cout<<"\n";
    cout<<state.ID2.nop<<" ";
    for(auto v:state.ID2.Instr)cout<<v<<" ";
    cout<<"\n";
    cout<<state.RR.nop<<" "<<state.RR.op<<" "<<state.RR.reg1<<" "<<state.RR.reg2<<" "<<state.RR.reg3<<" "<<state.RR.imm<<" ";
    cout<<state.RR.rd_mem<<" "<<state.RR.wrt_mem<<" "<<state.RR.wrt_enable<<" "<<state.RR.brnch<<"\n";
    cout<<state.EX.nop<<" "<<state.EX.op<<" "<<state.EX.val1<<" "<<state.EX.val2<<" "<<state.EX.val3<<" ";
    cout<<state.EX.rd_mem<<" "<<state.EX.wrt_mem<<" "<<state.EX.wrt_enable<<" "<<state.EX.Wrt_reg_addr<<"\n";
    cout<<state.MM1.nop<<" "<<state.MM1.op<<" "<<state.MM1.val1<<" "<<state.MM1.val2<<" ";
    cout<<state.MM1.rd_mem<<" "<<state.MM1.wrt_mem<<" "<<state.MM1.wrt_enable<<" "<<state.MM1.Wrt_reg_addr<<"\n";
    cout<<state.MM2.nop<<" "<<state.MM2.op<<" "<<state.MM2.val1<<" "<<state.MM2.val2<<" ";
    cout<<state.MM2.rd_mem<<" "<<state.MM2.wrt_mem<<" "<<state.MM2.wrt_enable<<" "<<state.MM2.Wrt_reg_addr<<"\n";
    cout<<state.WB.nop<<" "<<state.WB.wrt_enable<<" "<<state.WB.Wrt_data<<" "<<state.WB.Wrt_reg_addr<<"\n";
    // cout<<state.EX.nop<<" "<<state.EX.op<<" "<<state.EX.Read_data1<<" "<<state.EX.Read_data2<<" ";
    // cout<<state.EX.rd_mem<<" "<<state.EX.wrt_enable<<" "<<state.EX.wrt_mem<<" "<<state.EX.Wrt_reg_addr<<" "<<state.EX.brnch<<"\n";
    // cout<<state.MEM.nop<<" "<<state.MEM.ALUresult<<" "<<state.MEM.Wrt_reg_addr<<" ";
    // cout<<state.MEM.wrt_enable<<" "<<state.MEM.wrt_mem<<" "<<state.MEM.rd_mem<<"\n";
    // cout<<state.WB.nop<<" "<<state.WB.Wrt_data<<" "<<state.WB.Wrt_reg_addr<<" "<<state.WB.wrt_enable<<"\n";
}
int main(){
    std::ifstream file("input.asm");
    // ofstream filw("result.txt");
    std::string line;
	while (getline(file, line))parseCommand(line);
	file.close();
	stateStruct state, newState,temp;
    state.IF1.PC=0;
    state.IF1.nop=0;
    state.IF2.PC=0;
    state.IF2.nop=1;
    state.ID1.nop=1;
    state.ID1.Instr={}; 
    state.ID2.nop=1;
    state.ID2.Instr={};
    state.RR.imm=0;
    state.RR.nop=1;
    state.RR.op="";
    state.RR.rd_mem=state.RR.wrt_enable=state.RR.wrt_mem=0;
    state.RR.reg1=state.RR.reg2=state.RR.reg3="";
	state.EX.nop=1;
    state.EX.op=state.EX.Wrt_reg_addr="";
    state.EX.val1=state.EX.val2=state.EX.val3=state.EX.rd_mem=state.EX.wrt_enable=state.EX.wrt_mem=0;
    state.EX.brnch="";
    state.MM1.nop=1,state.MM1.val1=state.MM1.val2=0;
    state.MM2.nop=1,state.MM1.val2=state.MM2.val2=0;
    state.MM1.rd_mem=state.MM1.wrt_enable=state.MM1.wrt_mem=0;
    state.MM2.rd_mem=state.MM2.wrt_enable=state.MM2.wrt_mem=0;
    state.MM1.op=state.MM1.Wrt_reg_addr=state.MM2.op=state.MM2.Wrt_reg_addr="";
    state.WB.nop=1;
    state.WB.Wrt_data=0;
    state.WB.wrt_enable=0;
    state.WB.Wrt_reg_addr="";
	// state.EX.Read_data1=0;
	// state.EX.Read_data2=0;
    // state.EX.brnch="";
	// state.EX.op="";
	// state.EX.wrt_enable=0;
	// state.EX.wrt_mem=0;
	// state.EX.Wrt_reg_addr="";
	// state.WB.Wrt_data=0;
	// state.WB.wrt_enable=0;
	// state.WB.Wrt_reg_addr="";
    init_reg();
	int cycle=0;
	vector<string>instr;
    // for(auto v:commands){
    //     for(auto u:v)cout<<u<<" ";
    //     cout<<"\n";
    // }
    // for(auto v:address)cout<<v.first<<" "<<v.second<<"\n";
	// cout<<"cycle number is: "<<cycle<<"\n";
    for(int i=0;i<32;i++)cout<<registers[i]<<" ";
	cout<<"\n"<<"0\n";
    while(true){
        int haz=0,remvlk=-1;
        if (state.IF1.nop && state.IF2.nop && state.ID1.nop && state.ID2.nop && state.RR.nop && 
        state.EX.nop && state.MM1.nop && state.MM2.nop && state.WB.nop){
            break;	
		}
        newState=state;
        // cout<<state.IF1.nop<<" "<<state.IF2.nop<<" "<<state.ID1.nop<<" "<<state.ID2.nop<<" "<<state.RR.nop<<" "<<state.EX.nop<<" ";
        // cout<<state.MM1.nop<<" "<<state.MM2.nop<<" "<<state.WB.nop<<"\n";
        // <<state.ID.nop<<" "<<state.EX.nop<<" "<<state.MEM.nop<<" "<<state.WB.nop<<"\n";
        cycle++;
		// cout<<"cycle number is: "<<cycle<<"\n";
        // printstate(state);
        vector<pair<int,int>> memc;
        /* --------------------- WB stage --------------------- */
		if ((!state.WB.nop)){
            if(state.WB.wrt_enable){
            // cout<<"rite in "<<cycle<<"\n";
			remvlk=registerMap[state.WB.Wrt_reg_addr];
			registers[registerMap[state.WB.Wrt_reg_addr]] = state.WB.Wrt_data;
            }
            newState.WB.nop=1;
		}
        /* --------------------- MEM stage --------------------- */
		// newState.W2.nop = state.MM2.nop;
		if (!state.MM2.nop){
            newState.WB.nop=0;
            newState.WB.Wrt_reg_addr=state.MM2.Wrt_reg_addr;
            newState.WB.wrt_enable=state.MM2.wrt_enable;
			if (state.MM2.wrt_mem){
				store_data(state.MM1.val2/4, state.MM1.val1);
				memc.push_back({state.MM1.val2/4,state.MM1.val1});
			}
			else if (state.MM2.rd_mem){
                newState.WB.Wrt_data = dataMem[state.MM2.val2/4];
            }
            newState.MM2.nop=1;
		}
        if(!state.MM1.nop){
            newState.MM2.nop=state.MM1.nop;
            newState.MM2.op=state.MM1.op;
            newState.MM2.rd_mem=state.MM1.rd_mem;
            newState.MM2.val1 = state.MM1.val1;
            newState.MM2.val2 = state.MM1.val2;
            newState.MM2.wrt_enable= state.MM1.wrt_enable;
            newState.MM2.wrt_mem = state.MM1.wrt_mem;
            newState.MM2.Wrt_reg_addr = state.MM1.Wrt_reg_addr;
            newState.MM1.nop=1;
        }
        /* --------------------- EX stage --------------------- */
		if (!state.EX.nop){
            newState.EX.nop=1;
            if(state.EX.rd_mem==0 && state.EX.wrt_mem==0 && state.EX.wrt_enable==0){
                if(state.MM1.nop==0 || state.MM2.nop==0){
                    haz=-1;
                    newState.EX=state.EX;
                }
                else{
                    newState.WB.nop=0;
                    newState.WB.Wrt_data=0;
                    newState.WB.wrt_enable=0;
                    newState.WB.Wrt_reg_addr=state.EX.Wrt_reg_addr;
                    haz=-1;
                    newState.IF1.nop=0;
                    if(state.EX.op=="beq"){
                        if(state.EX.val2==state.EX.val3)newState.IF1.PC=address[state.EX.brnch];
                        else newState.IF1.PC=state.IF2.PC;
                    }
                    else{
                        if(state.EX.val3!=state.EX.val2)newState.IF1.PC=address[state.EX.brnch];
                        else newState.IF1.PC=state.IF2.PC;
                    }
                }
            }
            if(state.EX.rd_mem){
                newState.MM1.nop = 0;
                newState.MM1.val2=state.EX.val3+state.EX.val2;
                newState.MM1.rd_mem=state.EX.rd_mem;
                newState.MM1.wrt_mem=state.EX.wrt_mem;
                newState.MM1.wrt_enable=state.EX.wrt_enable;
                newState.MM1.Wrt_reg_addr=state.EX.Wrt_reg_addr;
            }
            else if(state.EX.wrt_mem){
                newState.MM1.nop = 0;
                newState.MM1.val2=state.EX.val3+state.EX.val2;
                newState.MM1.val1=state.EX.val1;
                newState.MM1.rd_mem=state.EX.rd_mem;
                newState.MM1.wrt_mem=state.EX.wrt_mem;
                newState.MM1.wrt_enable=state.EX.wrt_enable;
            }
            else{
                if(state.MM1.nop==0 || state.MM2.nop==0){
                    haz=-1;
                    newState.EX=state.EX;
                }
                else if(state.EX.op=="addi"){
                    newState.WB.nop=0;
                    newState.WB.Wrt_data=state.EX.val2+state.EX.val3;
                    newState.WB.wrt_enable=1;
                    newState.WB.Wrt_reg_addr=state.EX.Wrt_reg_addr;
                }
                else if(state.EX.op=="add" || state.EX.op=="sub" || state.EX.op=="mul"){
                    newState.WB.nop=0;
                    newState.WB.Wrt_data=ans(state.EX.op,state.EX.val2,state.EX.val3);
                    newState.WB.wrt_enable=1;
                    newState.WB.Wrt_reg_addr=state.EX.Wrt_reg_addr;
                }
            }
		}
        /*---------------------- RR stage ----------------------*/
        if(haz==0){
            newState.EX.nop=state.RR.nop;
            if(!state.RR.nop){
                if(state.RR.op=="addi"){
                    if(!haz_poss[registerMap[state.RR.reg2]]){
                        newState.EX.op=state.RR.op;
                        newState.EX.val3=state.RR.imm;
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else if(haz_poss[registerMap[state.RR.reg2]]==1 && state.RR.reg2==state.RR.reg1){
                        newState.EX.op=state.RR.op;
                        newState.EX.val3=state.RR.imm;
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else{
                        haz++;
                        newState.RR=state.RR;
                        newState.EX.nop=1;
                    }
                }
                else if(state.RR.op=="add"|| state.RR.op=="mul"|| state.RR.op=="sub"){
                    if((!haz_poss[registerMap[state.RR.reg2]] && !haz_poss[registerMap[state.RR.reg3]])){
                        newState.EX.op=state.RR.op;
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.val3=registers[registerMap[state.RR.reg3]];
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else if(state.RR.reg1==state.RR.reg2 && state.RR.reg1==state.RR.reg3 && haz_poss[registerMap[state.RR.reg1]]==1){
                        newState.EX.op=state.RR.op;
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.val3=registers[registerMap[state.RR.reg3]];
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else if(state.RR.reg1==state.RR.reg2 && haz_poss[registerMap[state.RR.reg3]]==0 && haz_poss[registerMap[state.RR.reg1]]==1 ){
                        newState.EX.op=state.RR.op;
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.val3=registers[registerMap[state.RR.reg3]];
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else if(state.RR.reg1==state.RR.reg3 && haz_poss[registerMap[state.RR.reg2]]==0 && haz_poss[registerMap[state.RR.reg1]]==1) {
                        newState.EX.op=state.RR.op;
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.val3=registers[registerMap[state.RR.reg3]];
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else{
                        haz++;
                        newState.RR=state.RR;
                        newState.EX.nop=1;
                    }
                }
                else if(state.RR.op=="lw"){
                    if(!haz_poss[registerMap[state.RR.reg2]]){
                        newState.EX.op=state.RR.op;
                        newState.EX.Wrt_reg_addr=state.RR.reg1;
                        newState.EX.val3=state.RR.imm;
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else{
                        haz++;
                        newState.RR=state.RR;
                        newState.EX.nop=1;
                    }
                }
                else if(state.RR.op=="sw"){
                    if(!haz_poss[registerMap[state.RR.reg2]] && !haz_poss[registerMap[state.RR.reg1]]){
                        newState.EX.op=state.RR.op;
                        newState.EX.val3=state.RR.imm;
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.EX.val1=registers[registerMap[state.RR.reg1]];
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.RR.nop=1;
                    }
                    else{
                        haz++;
                        newState.RR=state.RR;
                        newState.EX.nop=1;
                    }
                }
                else{
                    if(!haz_poss[registerMap[state.RR.reg2]] && !haz_poss[registerMap[state.RR.reg3]]){
                        newState.EX.op=state.RR.op;
                        newState.EX.val3=registers[registerMap[state.RR.reg3]];
                        newState.EX.val2=registers[registerMap[state.RR.reg2]];
                        newState.RR.nop=1;
                        newState.EX.rd_mem=state.RR.rd_mem;
                        newState.EX.wrt_enable=state.RR.wrt_enable;
                        newState.EX.wrt_mem=state.RR.wrt_mem;
                        newState.EX.brnch=state.RR.brnch;
                    }
                    else{
                        haz++;
                        newState.RR=state.RR;
                        newState.EX.nop=1;
                    }
                }
            }
        
        }
        /* --------------------- ID stage --------------------- */
        if(haz==0){
            newState.RR.nop=state.ID2.nop;
            if(!state.ID2.nop){
                instr=state.ID2.Instr;
                newState.RR.op=instr[0];
                newState.RR.rd_mem=newState.RR.wrt_enable=newState.RR.wrt_mem=0;
                if(instr[0]=="addi"){
                    newState.RR.reg1=instr[1];
                    newState.RR.reg2=instr[2];
                    newState.RR.imm=stoi(instr[3]);
                    newState.RR.wrt_enable=1;
                }
                else if(instr[0]=="add" || instr[0]=="sub" || instr[0]=="mul"){
                    newState.RR.reg1=instr[1];
                    newState.RR.reg2=instr[2];
                    newState.RR.reg3=instr[3];
                    newState.RR.wrt_enable=1;
                }
                else if(instr[0]=="lw"){
                    pair<int,string> ab=locateAddress(instr[2]);
                    newState.RR.reg1=instr[1];
                    newState.RR.reg2=ab.second;
                    newState.RR.imm=ab.first;
                    newState.RR.wrt_enable=1;
                    newState.RR.rd_mem=1;
                }
                else if(instr[0]=="sw"){
                    pair<int,string> ab=locateAddress(instr[2]);
                    newState.RR.reg1=instr[1];
                    newState.RR.reg2=ab.second;
                    newState.RR.imm=ab.first;
                    newState.RR.wrt_mem=1;
                }
                else if(instr[0]=="j"){
                    newState.IF1.nop=0;
                    newState.IF1.PC=address[instr[1]];
                }
                else{
                    // cout<<"hello\n";
                    newState.RR.reg2=instr[1];
                    newState.RR.reg3=instr[2];
                    newState.RR.brnch=instr[3];
                    // cout<<newState.RR.brnch<<"\n";
                } 
                if(newState.RR.wrt_enable)haz_poss[registerMap[newState.RR.reg1]]++;
            }
            // if(newState.RR.wrt_enable)haz_poss[registerMap[newState.RR.reg1]]++;
            newState.ID2.nop=state.ID1.nop;
            if(!state.ID1.nop){
                newState.ID2.Instr=state.ID1.Instr;
                if(state.ID1.Instr[0]=="beq" || state.ID1.Instr[0]=="bne" || state.ID1.Instr[0]=="j"){
                    state.IF1.nop=1;
                    state.IF2.nop=1;
                    newState.IF1.nop=1;
                    // newState.IF2.nop=1;
                    // newState.ID1.nop=1;
                }
            }
        }
        else if(haz!=-1){
            newState.ID1=state.ID1;
            newState.ID2=state.ID2;
        }
        // newState.EX.nop=state.ID.nop;
        // if(!state.ID.nop){
        //     instr=state.ID.Instr;		
		// 	else if(instr[0]=="beq" || instr[0]=="bne"){
		// 		newState.EX.op=instr[0];
        //         newState.EX.brnch=instr[3];
        //         state.IF.nop=1;
		// 		newState.EX.Wrt_reg_addr = "";
		// 		newState.EX.Read_data1 = registers[registerMap[instr[1]]];
		// 		newState.EX.Read_data2 = registers[registerMap[instr[2]]];
		// 		newState.EX.rd_mem = false;
		// 		newState.EX.wrt_mem = false;
		// 		newState.EX.wrt_enable = false;
        //         if(haz_poss[registerMap[instr[1]]] ||haz_poss[registerMap[instr[2]]]){
		// 			newState.EX.nop=1;
        //             newState.ID=state.ID;
		// 			// newState.ID.nop=1;
        //             haz++;
		// 		}
		// 	}
        //     else{
        //         state.IF=temp.IF;
        //         newState.EX.brnch=instr[1];
        //     }
		// 	if(newState.EX.nop==0){
        //         if(newState.EX.wrt_enable==true){
        //             haz_poss[registerMap[newState.EX.Wrt_reg_addr]]=1;
        //         }
        //     }
		// }

         /* --------------------- IF stage --------------------- */
        // if(cycle==6)cout<<newState.RR.brnch<<"\n";
        if(haz==0){
            newState.ID1.nop=state.IF2.nop;
            if(!state.IF2.nop){
                newState.ID1.Instr=commands[state.IF2.PC];
                newState.IF2.PC=state.IF1.PC;
                if (newState.IF2.PC>=commands.size()){  
                    newState.IF2.PC=state.IF2.PC; //PC remains the same//
                    newState.IF2.nop=1;
                }
            }
            newState.IF2.nop=state.IF1.nop;

            if(!state.IF1.nop){
                newState.IF2.PC=state.IF1.PC;
                newState.IF1.PC=state.IF1.PC+1;
                if (newState.IF1.PC>=commands.size()||(newState.ID1.Instr.size()>0&& newState.ID1.Instr[0]=="exit")){  
                    newState.IF1.PC=state.IF1.PC; //PC remains the same//
                    newState.IF1.nop=1;
                }   
            }

        }
        else if(haz!=-1)newState.IF1=state.IF1,newState.IF2=state.IF2;
        // else if(haz!=-1)newState.IF=state.IF;
        //  /* --------------------- IF1 stage --------------------- */
        
        // if(haz==0){
        //     newState.ID.nop=state.IF.nop;
        //     if(!state.IF.nop){
        //         newState.ID.Instr=commands[state.IF.PC];
        //         newState.IF.PC=state.IF.PC+1;
        //         if (newState.IF.PC>=commands.size()||newState.ID.Instr[0]=="exit"){  
        //             newState.IF.PC=state.IF.PC; //PC remains the same//
        //             newState.IF.nop=1;
        //         }
        //     }
        // }
        // else if(haz!=-1)newState.IF=state.IF;
        state=newState;
        if(remvlk!=-1)haz_poss[remvlk]--;
		for(int i=0;i<32;i++)cout<<registers[i]<<" ";
		cout<<"\n";
		cout<<memc.size()<<" ";
		for(auto v:memc)cout<<v.first<<" "<<v.second<<" ";
		cout<<"\n";    
        if(cycle==1000)break;
    }
}
