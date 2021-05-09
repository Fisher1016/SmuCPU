/****************************************************************
*	Copyright (C) 2021 Fisher1016 
*	Author：xwang
*	Creation time：2021-5-4  14:36:28
*
*	Description：Implement assembly program through C language simulation CPU
*
*	Modified history ：
* 		1.2021-5-9 下午 03:36:28
* 			The code replaces "\n" with "\ r" for the newline parsing of a text document.
* 			Because the text document's newline character is a combination of "\r\n"
*			Note in English
*
*  
*****************************************************************/


/*********************************************************************
*					Lib File
*********************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h> 


/*********************************************************************
*					Macro Instruction
*********************************************************************/

#define SIZE_CODE 2047
#define SIZE_DATA 255
//According to a 8086 CPU, the FLAG:	OF->11,DF->10,IF->9,TF->8,SF->7,ZF->6,AF->4,PF->2,CF->0
#define OF 11
#define DF 10
#define IF 9
#define TF 8
#define SF 7
#define ZF 6
#define AF 4
#define PF 2
#define CF 0

/*********************************************************************
*					Var Declaration
*********************************************************************/
//定义常用寄存器
short int AX = 0, BX = 0, CX = 0, DX = 0,
FLAG = 0,
DS = 0,
SS = SIZE_DATA,
SP = 0;

//Immediate Data
short int ImData;
//Locate the location of each instruction
char* IP;
//Point to the end of the identifier (to enable compilation)
short int TAG_last = 0;


//RISC Order Set
char OrderSet[14][5] = { "MOV","ADD","SUB","INC","DEC","MUL","DIV","XOR",
"PUSH","POP","CMP","JE","CALL","RET" };

//Create an array of identifiers to achieve custom identifiers associated with data in the data segment
struct TAG {
	char tag[10];
	short int arr;   //Array header address
}TagNode[5];

/*********************************************************************
*					Function Declaration
*********************************************************************/

void GetWArray(char* buff, short int* data, short int* data_offset);

void Storage(char* buff, char* code, short int* code_offset, short int* data, short int* data_offset);

void load(char* code, short int* code_offset, short int* data, short int* data_offset);

short int* SearchReg(char temp[], short int* data);

short int* FData(char code_run_buff[], short int* data);

short int* SData(char code_run_buff[], short int* data);

void MOV(char code_run_buff[], short int* data);

void XOR(char code_run_buff[], short int* data);

void MUL(char code_run_buff[], short int* data);

void DIV(char code_run_buff[], short int* data);

void ADD(char code_run_buff[], short int* data);

void SUB(char code_run_buff[], short int* data);

void INC(char code_run_buff[], short int* data);

void DEC(char code_run_buff[], short int* data);

char* JMP(char code_run_buff[], char* code);

char* JE(char code_run_buff[], char* code);

void CMP(char code_run_buff[], short int* data);

void PUSH(char code_run_buff[], short int* data);

void PUSH_N(short int num, short int* data);

void POP(char code_run_buff[], short int* data);

short int POP_N(short int* data);

void FlagSet(short int bit, short int var);

short int FlagRead(short int bit);

char* SearchFlag(char code_run_buff[], char* code);

void run(char* code, short int* data);

void TestMemory(char* code, short int* data);


/*
	Implements storing (DW) type data into a data area
	Read from a defined character and store it in the structure,
	 and store the first address of its data in main memory
*/
void GetWArray(char* buff, short int* data, short int* data_offset) {
	char* wbuff = buff;
	short int cursor = 0;
	short int sum = 0;
	char temp[20];
	wbuff++;

	//With space as the locator, the read is taken from the definition character
	while (*wbuff != ' ') {
		temp[cursor] = *wbuff;
		cursor++;
		wbuff++;
	}
	temp[cursor] = '\0';

	/*
	User-defined identifiers are bound to addresses
(This is the function of the compilation function, CPU can not achieve, here the function uses the structure to achieve)
Forms relational tables that allow custom identifiers to locate their data in the data area
	*/
	TagNode[TAG_last].arr = *data_offset;
	strcpy(TagNode[TAG_last].tag, temp);
	TAG_last++;

	//Put the data in an array
	wbuff = wbuff + 4;

	/*
		Converts ASCII data in a file to an array
	*/
	for (; *wbuff != '\r'; wbuff++) {
		if (*wbuff == ',') {
			data[*data_offset] = sum;
			(*data_offset)++;
			sum = 0;
		}
		else {

			sum = sum * 10 + *wbuff - '0';
		}

	}
	data[*data_offset] = sum;
	(*data_offset)++;
	sum = 0;
}

/*
Analyze the content of the program and store it in the main memory of the code area and the data area.
Jumps to the corresponding storage instructions CX and DX are both initialized to 0, 
so registers CX and DX can be used here as switches for code and data areas
*/
void Storage(char* buff, char* code, short int* code_offset, short int* data, short int* data_offset) {

	//storage data 
	if (strstr(buff, "SSEG SEGMENT") != NULL) {
		DX++;
	}


	if (DX == 1) {
		/*
			In order to facilitate the processing of the 16-bit register, 
			the data is uniformly defined in the DW format
			If you need to realize the expansion of different bits, 
			you only need to add the expansion register and add DD and DB functions
		*/
		if (strstr(buff, "DW") != NULL) {
			GetWArray(buff, data, data_offset);
		}
	}

	if (strstr(buff, "SSEG ENDS") != NULL) {
		DX--;
	}


	//store code
	if (strstr(buff, "CSEG SEGMENT") != NULL) {
		CX++;
	}

	if (CX == 1) {
		//To directly read the code in a file in the form of ASCII code into the main memory code area
		for (; *buff != '\n'; (*code_offset)++) {
			code[*code_offset] = *buff;
			buff++;
		}
		code[*code_offset] = '\n';
		(*code_offset)++;
	}

	if (strstr(buff, "END START") != NULL) {
		CX--;
	}
}

//Loads the contents of the.txt file into main memory
void load(char* code, short int* code_offset, short int* data, short int* data_offset) {
	FILE* fp = NULL;
	//Define buffers for line access
	char buff[255];


	fp=fopen("test.txt", "r");
	while (1) {
		//When you finish reading, stop reading the file
		if (feof(fp)) {  //The feof() file is zero if it is not empty
			break;
		}
		else {
			//Read buffer line by line
			fgets(buff, 255, (FILE*)fp);
			Storage(buff, code, code_offset, data, data_offset);
		}
	}
	if (fclose(fp));
}

/*
	Give key words (register/memory unit)
	Compare
	Returns the address of the register/storage unit
*/
short int* SearchReg(char temp[], short int* data) {
	if (strstr(temp, "AX")) {
		return &AX;
	}
	else if (strstr(temp, "BX")) {
		return &BX;
	}
	else if (strstr(temp, "CX")) {
		return &CX;
	}
	else if (strstr(temp, "DX")) {
		return &DX;
	}
	else if (strstr(temp, TagNode[0].tag)) {
		return &data[TagNode[0].arr];
	}
	else if (strstr(temp, TagNode[1].tag)) {
		return &data[TagNode[1].arr];
	}
	else if (strstr(temp, TagNode[2].tag)) {
		return &data[TagNode[2].arr];
	}
	else if (strstr(temp, TagNode[3].tag)) {
		return &data[TagNode[3].arr];
	}
	else if (strstr(temp, TagNode[4].tag)) {
		return &data[TagNode[4].arr];
	}
	else if (strstr(temp, "SSEG")) {
		return data;
	}
	return NULL;
}


short int* FData(char code_run_buff[], short int* data) {
	/*
	TEMP [] is used as a register/memory unit identifier to find the address of a register in a register group	
	*/
	
	char temp[10] = { '\0' };

	char* p = NULL;

	//The address to the final register or storage location
	short int* result = NULL;

	/*
		The first operation unit, p, is positioned with"
		And then p moves back one bit to refer to the first character in the string
	*/
	p = strchr(code_run_buff, ' ');
	p++;

	/*
		If it is a number, it is placed in a register. 
		If it is a string, it means a register is used
		p refers to the first character of the string
		Judge the content of p
			If it's a number
				Returns the address of the immediate register
			Otherwise,
				Call the SearchReg function to find the register or storage location
				And returns the address of the register/storage unit
	*/

	if (*p >= '0' && (*p) <= '9') {
		ImData = (*p) - '0';
		result = &ImData;
		return result;
	}
	else {
		short int iTemp = 0;
		for (; *p != ',' && *p != '\n'; iTemp++, p++) {
			temp[iTemp] = *p;
		}
		result = SearchReg(temp, data);
		return result;
	}

}

short int* SData(char code_run_buff[], short int* data) {

	//temp[] is used as a register/memory unit identifier to find the address of a register in a register group
	char temp[10] = { '\0' };

	char* p = NULL;

	//The address to the final register or storage location
	short int* result = NULL;

	/*
		The first operation unit, p, is positioned with"
		And then p moves back one bit to refer to the first character in the string
	*/
	p = strchr(code_run_buff, ',');
	p++;

	/*
		Let's look at the last function(same as the FData();
	*/

	if (*p >= '0' && (*p) <= '9') {
		ImData = (*p) - '0';
		result = &ImData;
		return result;
	}
	else {
		short int iTemp = 0;
		for (; *p != ',' && *p != '\n'; iTemp++, p++) {
			temp[iTemp] = *p;
		}
		result = SearchReg(temp, data);
		return result;
	}
}

void MOV(char code_run_buff[], short int* data) {
	//MOV [pFData],[pSData]

	short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	*pFData = *pSData;

}

void XOR(char code_run_buff[], short int* data) {
	//XOR [pFData],[pSData]
short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) ^ (*pSData);

}

void MUL(char code_run_buff[], short int* data) {
	//MUL [pFData],[pSData]
	short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) * (*pSData);

}

void DIV(char code_run_buff[], short int* data) {
	//DIV [pFData],[pSData]
	short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	//非零判断
	if ((*pSData) != 0) {
		(*pFData) = (*pFData) / (*pSData);
	}
	else {
		printf("Error:0 is not divisible!");
	}

}

void ADD(char code_run_buff[], short int* data) {
	//ADD [pFData],[pSData]
	short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) + (*pSData);
}

void SUB(char code_run_buff[], short int* data) {
	//SUB [pFData],[pSData]
	short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) - (*pSData);
}

void INC(char code_run_buff[], short int* data) {

	short int* pFData = NULL;

	pFData = FData(code_run_buff, data);

	(*pFData)++;
}

void DEC(char code_run_buff[], short int* data) {

	short int* pFData = NULL;

	pFData = FData(code_run_buff, data);

	(*pFData)--;
}


//According to 8086CPU,OF->11,DF->10,IF->9,TF->8,SF->7,ZF->6,AF->4,PF->2,CF->0

/*
	The FLAG register can be written and read by bit operation
*/
void FlagSet(short int bit, short int var) {
	if (var == 1) {
		FLAG = FLAG | (1 << bit);
	}
	else if (var == 0) {
		FLAG = FLAG & (~(1 << bit));
	}
}

short int FlagRead(short int bit) {
	short int var;
	var = (FLAG & (1 << bit)) >> bit;
	return var;
}

void CMP(char code_run_buff[], short int* data) {

	short int siTemp;//used to store oprd1-oprd2
	short int* pFData = NULL;
	short int* pSData = NULL;

	//The extracted string is compared to the set of registers, and the pointer - > is matched;Register/storage unit
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	//After the difference, set the FLAG register according to the difference
	siTemp = (*pFData) - (*pSData);
	if (siTemp > 0) {
		FlagSet(SF, 0);
		FlagSet(OF, 0);
	}
	else if (siTemp < 0) {
		FlagSet(SF, 1);
		FlagSet(OF, 0);
	}
	else {
		FlagSet(ZF, 1);
	}

}

/*
	In assembly, data segments and stack segments are often set
		Something like that
		<<<<<<<[Stack]********[Data]>>>>>>>>>>
	Because of the small amount of data
	I'm going to share the same data area in main memory with the stack segment
		Something like that
		[Data]>>>>>>>>>>********<<<<<<<[Stack]
		So here we need to reserve a relatively large amount of storage space
*/
void PUSH(char code_run_buff[], short int* data) {
	short int* pFData = NULL;
	pFData = FData(code_run_buff, data);
	*(data + (SS + SP)) = (*pFData);
	SP--;
}

/*
	PUSH register;
	PUSH_N imm；
*/
void PUSH_N(short int num, short int* data) {
	data[SS + SP] = num;
	SP--;
}

/*
	POP register;
	POP_N imm；
*/
void POP(char code_run_buff[], short int* data) {
	short int* pFData = NULL;
	pFData = FData(code_run_buff, data);
	SP++;
	(*pFData) = data[SS + SP];
}

/*
	POP register;
	POP_N imm；
*/
short int POP_N(short int* data) {
	SP++;
	return data[SS + SP];
}

//In a subfunction call, extract the subfunction identifier and locate it
char* SearchFlag(char code_run_buff[], char* code) {
	char temp[20] = { '\0' };
	char* p = NULL;
	char* result = NULL;//Point to the final register or data
	p = strchr(code_run_buff, ' ');
	p++;
	
	/*
	P refers to the first character of the string, 
	and if it's a number, it's in a register, 
	and if it's a string, it means a register is used, 
	and then it finds the corresponding register
	*/

	//Extract the subfunction identifier (function name)
	short int iTemp = 0;
	for (; *(p) != '\r'; iTemp++, p++) {
		temp[iTemp] = *p;
	}

	//Locate code snippets with the ":" suffix
	short int iiTemp = iTemp;  //The temporary variable, ":", does not affect the search for the next "PROC" suffix
	temp[iiTemp++] = ':';
	if (result = strstr(code, temp)) {
		return result;
	}

	//Locate code snippets with the "PROC" suffix
	temp[iTemp++] = ' ';
	temp[iTemp++] = 'P';
	temp[iTemp++] = 'R';
	temp[iTemp++] = 'O';
	temp[iTemp++] = 'C';
	if (result = strstr(code, temp)) {
		return result;
	}
	return result;

}


/*
	Returns the first address of the function identifier
	After the call to assign a value to the PC, to implement the transfer instruction
*/
char* JMP(char code_run_buff[], char* code) {

	return SearchFlag(code_run_buff, code);

}

/*
	Checks the value of ZF in the FLAG register
	In conjunction with the CMP instruction, 
	if the two operands are equal (that is, the ZF flag bit is 1), 
	the subfunction after JE is transferred
*/
char* JE(char code_run_buff[], char* code) {
	if (FlagRead(ZF)) {
		return JMP(code_run_buff, code);
	}
	return NULL;
}

/*
	Execution code area
	Ideas:
		Locate the MAIN function based on the keyword
		Extract code snippets line by line from the code area
		Compare the key words one by one, after confirming the line instruction
		Complete the corresponding operation
*/
void run(char* code, short int* data) {
	//Define Buffers
	char code_run_buff[255] = { '\0' };  //Use the end of the string as the delimiter
	char* pcode_run_buff = code_run_buff;

	//CS+ip   dynamic relocation
	char* CS = code;
	char* IP = strstr(code, "MAIN PROC");
	short int ip = 0;
	char* pcode = IP;

	//Execute a program
	while (*pcode != '\0') {
		for (; *pcode != '\n'; pcode++) {
			*pcode_run_buff = *pcode;
			pcode_run_buff++;
		}
		*pcode_run_buff = '\n';

		//Compare keywords
		if (IP = strstr(code_run_buff, "MOV")) {
			MOV(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "ADD")) {
			ADD(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "SUB")) {
			SUB(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "INC")) {
			INC(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "DEC")) {
			DEC(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "MUL")) {
			MUL(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "DIV")) {
			DIV(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "XOR")) {
			XOR(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "PUSH")) {
			PUSH(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "POP")) {
			POP(code_run_buff, data);
		}

		else if (IP = strstr(code_run_buff, "CMP")) {
			CMP(code_run_buff, data);
		}

		//PC points to the next sentence
		pcode++;

		if (IP = strstr(code_run_buff, "JE")) {
			if (JE(code_run_buff, code)) {
				pcode = JE(code_run_buff, code);
			}
		}

		if (IP = strstr(code_run_buff, "CALL")) {
			if (!JMP(code_run_buff, code)) {
				printf("Error:can't find sub code!");
			}
			else {
				//How to preserve the relative breakpoints is the difficulty. Work with RET

				/*
					Save the breakpoint
					Then put the first address of the subroutine code into pcode
				*/
				IP = pcode;
				PUSH_N(IP - CS, data);
				pcode = JMP(code_run_buff, code);

			}
		}

		if (IP = strstr(code_run_buff, "RET")) {

			/*
				POP(IP);
				The program continues running from the breakpoint
			*/
			ip = POP_N(data);
			pcode = CS + ip;

		}

		//Reuse this buffer space and import the next line
		memset(code_run_buff, '\0', 255);
		pcode_run_buff = code_run_buff;
	}
}

/*
	void TestMemory(char* code, short int* data)
	Shows CPU registers, main memory code areas, and data areas, as well as associated tables for custom characters
*/
void TestMemory(char* code, short int* data) {
	short int* p = data;
	int i = 0;

	printf("\n**********************Register**********************\n");
	printf("AX=%d\nBX=%d\nCX=%d\nDX=%d\nFLAG=%d\nSS=%d\nSP=%d\n", AX, BX, CX, DX, FLAG, SS, SP);

	printf("\n**********************Code**********************\n");
	for (i = 0; code[i] != '\0'; i++) {

		printf("%c", code[i]);
	}

	printf("\n**********************Data**********************\n");
	for (i = 0; i < 5; i++) {
		printf("%d\n", data[i]);
	}

	printf("\n**********************Tag**********************\n");
	for (i = 0; i < 5; i++) {
		printf("%s.arr:%d\n", TagNode[i].tag, TagNode[i].arr);
	}

}


int main() {
	//Defines code and data areas in main memory, the size of which can be changed in the macro definition
	char code[SIZE_CODE + 1] = { '\0' };
	short int data[SIZE_DATA + 1] = { 0 };
	short int code_offset = 0;
	short int data_offset = 0;

	//Load (source file - >;Main memory)
	load(code, &code_offset, data, &data_offset);

	//Display pre-run main memory data
	TestMemory(code, data);

	//Run a program in main memory
	run(code, data);

	//Display main memory data after running
	TestMemory(code, data);
}
