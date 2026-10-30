#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>

enum : uint8_t {
    MOV_II_VV,
    MOV_II_VC,

    MOV_IN_ARG,
    MOV_IN_ARG_CONST,

    MOV_OUT_ARG,

    ADD_III_VVV,
    ADD_III_VVC,

    SUB_III_VVC,

    CALL,
    CALL_C,
    CALL_V,

    RET_I_V,
    RET_I_C,

    RET_ADD_II_VV,


    ALLOC,
    DEALLOC,

    IF_LE_II_VC,

    PRINT,

    GOTO,

    eOF,
    OP_COUNT
};

enum : uint8_t {
    VAR_I,
    VAR_S,
    LIT_I,
    LIT_S,
};

struct Command {
    int16_t op;
    intptr_t mem1;
    intptr_t mem2;
    intptr_t mem3;
};

class VMemory {
private:
    uint64_t* data;
    intptr_t cur = 0;
    const intptr_t CAPACITY = 10000000;

public:
    void Alloc(intptr_t size) {
        cur += size;
    }

    void DeAlloc(intptr_t size) {
        cur -= size;
    }

    void Set(intptr_t data_pos, intptr_t value) {
        data[cur - data_pos] = static_cast<uint64_t>(value);
    }

    intptr_t Get(intptr_t data_pos) {
        return data[cur - data_pos];
    }

    void Push(intptr_t value) {
        Alloc(1);
        Set(1, value);
    }

    intptr_t Pop() {
        intptr_t result = Get(1);
        DeAlloc(1);
        return result;
    }

    VMemory() {
        cur = 0;
        data = new uint64_t[CAPACITY];
    }

    ~VMemory() {
        delete[] data;
    }
};

std::vector<std::string> str = {
    "Result: ",
    "\n",
};


#define NEXT uint8_t next_op = static_cast<uint8_t>(bytecode[cur].op); if (next_op >= OP_COUNT) goto L_DEFAULT; goto *dispatch_table[next_op]; 

int main() {
    VMemory StackFrame;
    VMemory CallFrame;
    VMemory ArgFrame;


    Command  pushcode[] =
    {
        {LIT_S,0,0,0},
        {VAR_I,1,0,0},
        {LIT_S,1,0,0},
        {eOF,0,0,0},
    };

    Command  bytecode[] =
    {
        {ALLOC,10,0,0},
        {MOV_II_VC,9,30,0},
        {CALL_V,8,10,9},
        {MOV_OUT_ARG,1,1,0},
        {PRINT,0,0,0},
        {ADD_III_VVC,9,9,1},
        {DEALLOC,10,0,0},
        {eOF,0,0,0},

        {MOV_OUT_ARG,1,1,0},
        {IF_LE_II_VC,11,1,1},
        {RET_I_V,10,1,0},
        {SUB_III_VVC,2,1,1},
        {CALL_V,8,10,2},
        {MOV_OUT_ARG,4,1,0},
        {SUB_III_VVC,3,1,2},
        {CALL_V,8,10,3},
        {MOV_OUT_ARG,5,1,0},
        {RET_ADD_II_VV,10,4,5},
    };


    ArgFrame.Alloc(65536);


    intptr_t cur = 0;


    static void* dispatch_table[OP_COUNT] = {
        && L_MOV_II_VV,
        && L_MOV_II_VC,

        && L_MOV_IN_ARG,
        && L_MOV_IN_ARG_CONST,

        && L_MOV_OUT_ARG,

        && L_ADD_III_VVV,
        && L_ADD_III_VVC,

        && L_SUB_III_VVC,

        && L_CALL,
        && L_CALL_C,
        && L_CALL_V,

        && L_RET_I_V,
        && L_RET_I_C,
        && L_RET_ADD_II_VV,

        && L_ALLOC,

        && L_DEALLOC,

        && L_IF_LE_II_VC,

        && L_PRINT,

        && L_GOTO,

        && L_eOF
    };


    auto start = std::chrono::steady_clock::now();

    NEXT;

L_ALLOC:
    {
        StackFrame.Alloc(bytecode[cur].mem1);
        cur++;
        NEXT;
    }
L_DEALLOC:
    {
        StackFrame.DeAlloc(bytecode[cur].mem1);
        cur++;
        NEXT;
    }
L_MOV_II_VV:
    {
        cur++;
        NEXT;
    }
L_MOV_II_VC:
    {
        const Command& cmd = bytecode[cur];
        StackFrame.Set(cmd.mem1, cmd.mem2);
        cur++;
        NEXT;
    }
L_MOV_IN_ARG:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(cmd.mem1, StackFrame.Get(cmd.mem2));
        cur++;
        NEXT;
    }
L_MOV_IN_ARG_CONST:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(cmd.mem1, cmd.mem2);
        cur++;
        NEXT;
    }
L_MOV_OUT_ARG:
    {
        const Command& cmd = bytecode[cur];
        StackFrame.Set(cmd.mem1, ArgFrame.Get(cmd.mem2));
        cur++;
        NEXT;
    }

L_IF_LE_II_VC:
    {
        const Command& cmd = bytecode[cur];
        bool cond = StackFrame.Get(cmd.mem2) <= cmd.mem3;

        if (cond) {
            cur = cur + 1;
        }
        else {
            cur = cmd.mem1;
        }

        NEXT;
    }

L_ADD_III_VVV:
    {
        const Command& cmd = bytecode[cur];
        StackFrame.Set(cmd.mem1, StackFrame.Get(cmd.mem2) + StackFrame.Get(cmd.mem3));
        cur++;

        NEXT;
    }
L_ADD_III_VVC:
    {
        const Command& cmd = bytecode[cur];
        StackFrame.Set(cmd.mem1, StackFrame.Get(cmd.mem2) + cmd.mem3);
        cur++;

        NEXT;
    }
L_SUB_III_VVC:
    {
        const Command& cmd = bytecode[cur];
        StackFrame.Set(cmd.mem1, StackFrame.Get(cmd.mem2) - cmd.mem3);
        cur++;
        NEXT;
    }
L_PRINT:
    {
        const Command& cmd = bytecode[cur];
        int i = cmd.mem1;
        while (true) {
            if (pushcode[i].op == VAR_I) {
                std::cout << StackFrame.Get(pushcode[i].mem1);

            }
            else if (pushcode[i].op == LIT_S) {
                std::cout << str[pushcode[i].mem1];

            }
            else {
                break;

            }
            i++;
        }

        cur++;
        NEXT;
    }
L_CALL:
    {
        const Command& cmd = bytecode[cur];
        CallFrame.Push(cur + 1);
        cur = cmd.mem1;
        StackFrame.Alloc(cmd.mem2);
        NEXT;
    }
L_CALL_C:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(1, cmd.mem3);
        CallFrame.Push(cur + 1);
        cur = cmd.mem1;
        StackFrame.Alloc(cmd.mem2);
        NEXT;
    }
L_CALL_V:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(1, StackFrame.Get(cmd.mem3));
        CallFrame.Push(cur + 1);
        cur = cmd.mem1;
        StackFrame.Alloc(cmd.mem2);
        NEXT;
    }
L_RET_I_V:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(1, StackFrame.Get(cmd.mem2));
        cur = CallFrame.Pop();
        StackFrame.DeAlloc(cmd.mem1);
        NEXT;
    }
L_RET_I_C:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(1, cmd.mem2);
        cur = CallFrame.Pop();
        StackFrame.DeAlloc(cmd.mem1);
        NEXT;
    }
L_RET_ADD_II_VV:
    {
        const Command& cmd = bytecode[cur];
        ArgFrame.Set(1, StackFrame.Get(cmd.mem2) + StackFrame.Get(cmd.mem3));
        cur = CallFrame.Pop();
        StackFrame.DeAlloc(cmd.mem1);
        NEXT;
    }
L_GOTO:
    {
        const Command& cmd = bytecode[cur];
        cur = cmd.mem1;
        NEXT;
    }
L_eOF:
    {
        goto eof;
    }
L_DEFAULT:
    {
        std::cout << "CRUSH: cur = " << cur << std::endl;
        goto eof;
    }

eof:
    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "elapsed: " << ms << " ms" << std::endl;

    return 0;
}