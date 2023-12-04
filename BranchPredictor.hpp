#ifndef __BRANCH_PREDICTOR_HPP__
#define __BRANCH_PREDICTOR_HPP__

#include <vector>
#include <bitset>
#include <cassert>
using namespace std;

struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) = 0;
};

struct SaturatingBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> table;
    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}

    bool predict(uint32_t pc) {
        uint32_t index = pc & (table.size()-1);
        return table[index].to_ulong()>=2;
    }

    void update(uint32_t pc, bool taken) {
        uint32_t index = pc & (table.size()-1);
        if (taken && table[index].to_ulong() < 3)table[index]=bitset<2>(table[index].to_ulong() + 1);
        else if(!(taken) && (table[index].to_ulong()>0))table[index]=bitset<2>(table[index].to_ulong()-1);
    }
};

struct BHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    BHRBranchPredictor(int value) : bhrTable(1 << 2, value), bhr(value) {}
    bool predict(uint32_t pc) {
        return bhrTable[bhr.to_ulong()].to_ulong()>=2;
    }

    void update(uint32_t pc, bool taken) {
        int bhrInd=bhr.to_ulong();
        if(taken && bhrTable[bhrInd].to_ulong()<3)bhrTable[bhrInd]=bitset<2> (bhrTable[bhrInd].to_ulong()+1);
        else if(!taken && bhrTable[bhrInd].to_ulong()>0)bhrTable[bhrInd]=bitset<2> (bhrTable[bhrInd].to_ulong()-1);
        bhr<<=1;
        bhr[0]=taken;
    }
};

struct SaturatingBHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    std::vector<std::bitset<2>> table;
    std::vector<std::bitset<2>> combination;
    SaturatingBHRBranchPredictor(int value,int size):bhrTable(1<<2,value),bhr(value),table(1<<14,value),combination(size, value) {
        assert(size <= (1 << 16));
    }

    bool predict(uint32_t pc) {
       uint32_t index = (pc ^ bhr.to_ulong()) % table.size();
        uint32_t combIndex = (pc ^ bhr.to_ulong()) % combination.size();
        std::bitset<2> counter = table[index];
        std::bitset<2> combinationCounter = combination[combIndex];

        // Combine bhrTable, counter, and combinationCounter to make the final prediction
        std::bitset<2> prediction = bhrTable[bhr.to_ulong()] | counter | combinationCounter;

        return (prediction[1] == 1);
    }

    void update(uint32_t pc, bool taken) {
        uint32_t index = (pc ^ bhr.to_ulong()) % table.size();
        uint32_t combIndex = (pc ^ bhr.to_ulong()) % combination.size();
        std::bitset<2>& counter = table[index];
        std::bitset<2>& combinationCounter = combination[combIndex];

        // Update bhrTable
        if (taken) {
            if (bhrTable[bhr.to_ulong()][1] != 1) {
                bhrTable[bhr.to_ulong()] = bhrTable[bhr.to_ulong()].to_ulong() + 1;
            }
        } else {
            if (bhrTable[bhr.to_ulong()][0] != 1) {
                bhrTable[bhr.to_ulong()] = bhrTable[bhr.to_ulong()].to_ulong() - 1;
            }
        }

        // Update counter
        if (taken) {
            if (counter[1] != 1) {
                counter = counter.to_ulong() + 1;
            }
        } else {
            if (counter[0] != 1) {
                counter = counter.to_ulong() - 1;
            }
        }

        // Update combinationCounter
        if (taken) {
            if (combinationCounter[1] != 1) {
                combinationCounter = combinationCounter.to_ulong() + 1;
            }
        } else {
            if (combinationCounter[0] != 1) {
                combinationCounter = combinationCounter.to_ulong() - 1;
            }
        }

        // Update bhr
        bhr <<= 1;
        if (taken) {
            bhr.set(0, 1);
        }
    
    }
};

#endif