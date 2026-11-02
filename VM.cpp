#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>

enum {
	MOV_II_VC,

	ADD_III_VVC,
	ADD_III_VVV,

	CALL,

	RET,

	RET_ADD_III_VVV,

	IF_LE_II_VC,

	PRINT,

	PAUSE,
	eOF,

	OP_COUNT
};

enum {
	VAR_I,
	VAR_S,
	LIT_I,
	LIT_S,
};

struct Command {
	intptr_t op;
	intptr_t mem1;
	intptr_t mem2;
	intptr_t mem3;
};


class VMemory {
private:
	intptr_t* data;
	intptr_t cur = 0;
	const intptr_t CAPACITY = 10000000;

public:
	void SetCur(intptr_t size) {

		cur = size;

	}

	void Up(intptr_t size) {
		cur += size;
	}

	void Down(intptr_t size) {
		cur -= size;
	}

	void SetRelative(intptr_t data_pos, intptr_t value) {
		data[cur + data_pos] = static_cast<uint64_t>(value);
	}

	intptr_t GetRelative(intptr_t data_pos) {
		return data[cur + data_pos];
	}


	void Push(intptr_t value) {

		SetRelative(1, value);
		Up(1);
	}

	intptr_t Pop() {
		intptr_t result = GetRelative(0);
		Down(1);
		return result;
	}
	VMemory() {
		cur = 0;
		data = new intptr_t[CAPACITY];
	}

	~VMemory() {
		delete[] data;
	}
};

std::vector<std::string> str = {
	"Result: ",
	"\n",
};


#define NEXT goto *dispatch_table[*cur];

int main() {
	VMemory StackFrame;
	VMemory CallFrame;

	std::ios_base::sync_with_stdio(false);

	Command  pushcode[] =
	{
		{LIT_S,0,0,0},
		{VAR_I,0,0,0},
		{LIT_S,1,0,0},
		{eOF,0,0,0},
	};


	intptr_t bytecode[] =
	{
	MOV_II_VC,    1,  30 ,
	CALL,         10, 2,  0 ,
	PRINT,        0 ,
	eOF ,

	IF_LE_II_VC,  16, -1, 1 ,
	RET,          -1,
	ADD_III_VVC,  3,  -1, -1 ,
	CALL,         10, 4,  0 ,
	ADD_III_VVC,  3,  -1, -2 ,
	CALL,         10, 4,  1 ,
	ADD_III_VVV,  2, 1 ,  0,
	RET,  2,
	};

	static void* dispatch_table[OP_COUNT] = {
		&& L_MOV_II_VC,

		&& L_ADD_III_VVC,
		&& L_ADD_III_VVV,

		&& L_CALL,

		&& L_RET,
		&& L_RET_ADD_III_VVV,

		&& L_IF_LE_II_VC,

		&& L_PRINT,

		&& L_PAUSE,
		&& L_eOF
	};

	intptr_t* cur = bytecode;

	float bavg = 0;
	float bmin = 10000000;
	float bmax = 0;

	int bcount = 1000;

	for (int i = 1; i < bcount;i++) {

		auto start = std::chrono::steady_clock::now();

		NEXT;

	L_MOV_II_VC:
		{
			StackFrame.SetRelative(cur[1], cur[2]);
			cur += 3;
			NEXT;
		}
	L_IF_LE_II_VC:
		{
			bool cond = StackFrame.GetRelative(cur[2]) <= cur[3];

			if (cond) {
				cur += 4;
			}
			else {
				cur = bytecode + cur[1];
			}

			NEXT;
		}
	L_ADD_III_VVC:
		{
			StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[2]) + cur[3]);
			cur += 4;

			NEXT;
		}
	L_ADD_III_VVV:
		{
			StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[2]) + StackFrame.GetRelative(cur[3]));
			cur += 4;

			NEXT;
		}
	L_PRINT:
		{
			if (i == bcount - 1) {
				int i = cur[1];

				while (true) {
					if (pushcode[i].op == VAR_I) {
						std::cout << StackFrame.GetRelative(pushcode[i].mem1);

					}
					else if (pushcode[i].op == LIT_S) {
						std::cout << str[pushcode[i].mem1];

					}
					else {
						break;

					}
					i++;
				}
			}
			cur += 2;
			NEXT;
		}
	L_CALL:
		{
			intptr_t mem2 = cur[2];

			CallFrame.Push((cur - bytecode) + 4);
			CallFrame.Push(cur[2]);
			CallFrame.Push(cur[3]);

			cur = bytecode + cur[1];

			StackFrame.Up(mem2);

			NEXT;
		}
	L_RET:
		{

			intptr_t result = StackFrame.GetRelative(cur[1]);

			intptr_t destination = CallFrame.Pop();
			intptr_t popp = CallFrame.Pop();
			intptr_t ret_idx = CallFrame.Pop();

			StackFrame.Down(popp);

			cur = bytecode + ret_idx;

			StackFrame.SetRelative(destination, result);

			NEXT;
		}
	L_RET_ADD_III_VVV:
		{

			intptr_t result = StackFrame.GetRelative(cur[1]) + StackFrame.GetRelative(cur[2]);

			intptr_t destination = CallFrame.Pop();
			intptr_t popp = CallFrame.Pop();
			intptr_t ret_idx = CallFrame.Pop();

			StackFrame.Down(popp);

			cur = bytecode + ret_idx;

			StackFrame.SetRelative(destination, result);

			NEXT;
		}
	L_PAUSE:
		{
			//	__debugbreak();
			NEXT;
		}
	L_eOF:
		{
			StackFrame.SetCur(0);
			CallFrame.SetCur(0);
			cur = bytecode;
			goto eof;
		}

	eof:

		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		double ms = elapsed.count();

		if (i > 10) {
			bavg = bavg + (ms - bavg) / ((float)i - 10);

			if (ms > bmax) {
				bmax = ms;
			}
			if (ms < bmin) { //bmin == 10000000
				bmin = ms;
			}
		}
		
	}
	std::cout << "count: " << bcount << std::endl;

	std::cout << "avg: " << bavg<< " ms" << std::endl;
	std::cout << "min: " << bmin << " ms" << std::endl;
	std::cout << "max: " << bmax << " ms" << std::endl;
	system("pause");  
	return 0;
}