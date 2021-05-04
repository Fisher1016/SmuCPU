/****************************************************************
*	Copyright (C) 2021 王旭
*?? 作者：xwang
*?? 创建时间：2021-5-4 下午 03:36:28
*
*?? 描述说明：
*
*?? 修改历史：2021-5-4 下午 03:36:28
*
*
*****************************************************************/


/*********************************************************************
*					库文件
*********************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h> 


/*********************************************************************
*					宏指令
*********************************************************************/

#define SIZE_CODE 2047
#define SIZE_DATA 255
//根据8086CPU,FLAG中，OF->11,DF->10,IF->9,TF->8,SF->7,ZF->6,AF->4,PF->2,CF->0
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
*					变量声明
*********************************************************************/
//定义常用寄存器
short int AX = 0, BX = 0, CX = 0, DX = 0,
FLAG = 0,
DS = 0,
SS = SIZE_DATA,
SP = 0;

//Immediate Data
short int ImData;
//定位每个指令的位置
char* IP;
//指向标识符末尾（实现编译功能）
short int TAG_last = 0;


//指令集
char OrderSet[14][5] = { "MOV","ADD","SUB","INC","DEC","MUL","DIV","XOR",
"PUSH","POP","CMP","JE","CALL","RET" };

//建立标识符数组，实现数据段中自定义标识符与数据相关联
struct TAG {
	char tag[10];
	short int arr;   //数组首地址
}TagNode[5];

/*********************************************************************
*					函数说明
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
	//实现将（DW）型数据存入数据区
	思路：读取自定义字符后存入到结构体中，同时保存其数据在主存中存放的首地址
*/
void GetWArray(char* buff, short int* data, short int* data_offset) {
	char* wbuff = buff;
	short int cursor = 0;
	short int sum = 0;
	char temp[20];
	wbuff++;

	//以空格为定位符，读取自定义字符
	while (*wbuff != ' ') {
		temp[cursor] = *wbuff;
		cursor++;
		wbuff++;
	}
	temp[cursor] = '\0';

	/*
	用户自定义标识符与地址绑定
	（这属于编译功能的作用，CPU无法实现，此处功能采用了结构体实现）
	形成关系表让自定义标识符可定位到其在数据区的数据
	*/
	TagNode[TAG_last].arr = *data_offset;
	strcpy(TagNode[TAG_last].tag, temp);
	TAG_last++;

	//把数据存入到数组中
	wbuff = wbuff + 4;

	/*
		将文件中ASCII码形式存放的数据转为数组
	*/
	for (; *wbuff != '\n'; wbuff++) {
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
解析程序内容，分别存储到主存的代码区和数据区
思路：比对关键词->跳转到对应存储指令
	CX和DX初始化均为0，故此处可以利用寄存器CX和DX作为代码区和数据区的开关
*/
void Storage(char* buff, char* code, short int* code_offset, short int* data, short int* data_offset) {

	//存储数据
	if (strstr(buff, "SSEG SEGMENT") != NULL) {
		DX++;
	}


	if (DX == 1) {
		/*
			此处为了便于16位寄存器处理，故统一将数据定义为DW格式
			若需实现不同字位拓展，只需再加入拓展寄存器后，增加DD、DB函数即可
		*/
		if (strstr(buff, "DW") != NULL) {
			GetWArray(buff, data, data_offset);
		}
	}

	if (strstr(buff, "SSEG ENDS") != NULL) {
		DX--;
	}


	//存储代码
	if (strstr(buff, "CSEG SEGMENT") != NULL) {
		CX++;
	}

	if (CX == 1) {
		//直接将文件中读出的代码以ASCII码的形式存入主存的代码区
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

//将.txt文件的内容加载到主存中
void load(char* code, short int* code_offset, short int* data, short int* data_offset) {
	FILE* fp = NULL;
	//定义缓冲区，实现按行存取
	char buff[255];


	fp=fopen("test.txt", "r");
	while (1) {
		//读完后则停止文件读取
		if (feof(fp)) {  //feof()文件非空为0
			break;
		}
		else {
			//按行读入缓冲区
			fgets(buff, 255, (FILE*)fp);
			Storage(buff, code, code_offset, data, data_offset);
		}
	}
	if (fclose(fp));
}

/*
	给出关键词（寄存器/存储单元）
	比对
	返回 寄存器/存储单元 的地址
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

	//temp[]作为寄存器/存储单元的标识符，用于在寄存器组里查找寄存器的地址
	char temp[10] = { '\0' };

	char* p = NULL;

	//指向最终的寄存器或者存储单元的地址
	short int* result = NULL;

	/*
		第一个操作单元，p用 ' '定位
		然后p再向后移一位，指向字符串第一个字符
	*/
	p = strchr(code_run_buff, ' ');
	p++;

	//，如果是数字，就放在寄存器里，是字符串说明这里用到了寄存器，则找出对应寄存器
	/*
		p指向字符串第一个字符
		判断p的内容
			如果是数字
				就放在立即数寄存器里,返回立即数寄存器地址
			否则
				调用SearchReg函数，查找该寄存器或者存储单元
				并返回寄存器/存储单元的地址
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

	//temp[]作为寄存器/存储单元的标识符，用于在寄存器组里查找寄存器的地址
	char temp[10] = { '\0' };

	char* p = NULL;

	//指向最终的寄存器或者存储单元的地址
	short int* result = NULL;

	/*
		第一个操作单元，p用 ' '定位
		然后p再向后移一位，指向字符串第一个字符
	*/
	p = strchr(code_run_buff, ',');
	p++;

	//，如果是数字，就放在寄存器里，是字符串说明这里用到了寄存器，则找出对应寄存器
	/*
		p指向字符串第一个字符
		判断p的内容
			如果是数字
				就放在立即数寄存器里,返回立即数寄存器地址
			否则
				调用SearchReg函数，查找该寄存器或者存储单元
				并返回寄存器/存储单元的地址
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

	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 开头是逗号，末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	*pFData = *pSData;

}

void XOR(char code_run_buff[], short int* data) {
	//XOR [pFData],[pSData]
	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 开头是逗号，末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) ^ (*pSData);

}

void MUL(char code_run_buff[], short int* data) {
	//MUL [pFData],[pSData]
	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 开头是逗号，末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) * (*pSData);

}

void DIV(char code_run_buff[], short int* data) {
	//DIV [pFData],[pSData]
	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 开头是逗号，末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
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
	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 开头是逗号，末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) + (*pSData);
}

void SUB(char code_run_buff[], short int* data) {
	//SUB [pFData],[pSData]
	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 开头是逗号，末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) - (*pSData);
}

void INC(char code_run_buff[], short int* data) {
	//目的操作数，开头是空格，末尾是逗号
	short int* pFData = NULL;

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);

	(*pFData)++;
}

void DEC(char code_run_buff[], short int* data) {
	//目的操作数，开头是空格，末尾是逗号
	short int* pFData = NULL;

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);

	(*pFData)--;
}


//根据8086CPU,OF->11,DF->10,IF->9,TF->8,SF->7,ZF->6,AF->4,PF->2,CF->0

/*
	通过位运算实现对FLAG寄存器的写入和读出
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

	short int siTemp;//用来暂存oprd1-oprd2
	short int* pFData = NULL;//目的操作数，开头是空格，末尾是逗号
	short int* pSData = NULL;//源操作数 末尾为换行符

	//提取出的字符串与寄存器集做比较，匹配则将指针->寄存器/存储单元
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	//做差后，根据差值设置FLAG寄存器
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
	在汇编中，常设置数据段和堆栈段
		类似于这样
		<<<<<<<[Stack]********[Data]>>>>>>>>>>
	因为数据量少
	我这里将数据段和堆栈段共享主存内的同一片数据区
		类似于这样
		[Data]>>>>>>>>>>********<<<<<<<[Stack]
		故这里需要预留相对大一些存储空间
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

//子函数调用中，提取子函数标识符，并定位
char* SearchFlag(char code_run_buff[], char* code) {
	char temp[20] = { '\0' };
	char* p = NULL;
	char* result = NULL;//指向最终的寄存器或者数据
	p = strchr(code_run_buff, ' ');
	p++;

	//p指向字符串第一个字符，如果是数字，就放在寄存器里，是字符串说明这里用到了寄存器，则找出对应寄存器

	//提取出子函数标识符（函数名）
	short int iTemp = 0;
	for (; *p != ',' && *p != '\n'; iTemp++, p++) {
		temp[iTemp] = *p;
	}

	//以“：”后缀形式定位代码段
	short int iiTemp = iTemp;  //临时变量，“：”查找时，不影响为了下一步“PROC”后缀查找
	temp[iiTemp++] = ':';
	if (result = strstr(code, temp)) {
		return result;
	}

	//以“PROC”后缀形式定位代码段
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
	返回函数标识符所在的首地址
	调用后赋值给PC，实现转移指令
*/
char* JMP(char code_run_buff[], char* code) {

	return SearchFlag(code_run_buff, code);

}

/*
	检测FLAG寄存器中ZF的值
	与CMP指令配合，两个操作数相等（即ZF标志位为1）则转移到JE 后的子函数
*/
char* JE(char code_run_buff[], char* code) {
	if (FlagRead(ZF)) {
		return JMP(code_run_buff, code);
	}
	return NULL;
}

/*
	执行代码区
	思路：
		根据关键词定位主函数MAIN
		将代码段从代码区逐行取出
		逐一比对关键词，确认该行指令后
		完成相应的操作
*/
void run(char* code, short int* data) {
	//定义缓冲区
	char code_run_buff[255] = { '\0' };  //用字符串结束符作为定界符
	char* pcode_run_buff = code_run_buff;

	//CS+ip动态重定位
	char* CS = code;
	char* IP = strstr(code, "MAIN PROC");
	short int ip = 0;
	char* pcode = IP;

	//执行程序
	while (*pcode != '\0') {
		for (; *pcode != '\n'; pcode++) {
			*pcode_run_buff = *pcode;
			pcode_run_buff++;
		}
		*pcode_run_buff = '\n';

		//比对关键词
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

		//PC指向下一句
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
				//如何保存相对断点是难点，与ret配合

				/*
					先保存断点
					然后将子程序代码所在首地址放入pcode
				*/
				IP = pcode;
				PUSH_N(IP - CS, data);
				pcode = JMP(code_run_buff, code);

			}
		}

		if (IP = strstr(code_run_buff, "RET")) {

			/*
				POP(IP);
				程序从断点处开始继续运行
			*/
			ip = POP_N(data);
			pcode = CS + ip;

		}

		//重新利用这部分buffer空间，导入下一行程序
		memset(code_run_buff, '\0', 255);
		pcode_run_buff = code_run_buff;
	}
}

/*
	void TestMemory(char* code, short int* data)
	展示CPU寄存器、主存代码区和数据区，以及自定义字符的关联表
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
	//定义主存中代码区和数据区，大小可在宏定义中修改
	char code[SIZE_CODE + 1] = { '\0' };
	short int data[SIZE_DATA + 1] = { 0 };
	short int code_offset = 0;
	short int data_offset = 0;

	//加载（源文件->主存）
	load(code, &code_offset, data, &data_offset);

	//展示运行前主存数据
	TestMemory(code, data);

	//运行主存中的程序
	run(code, data);

	//展示运行后主存数据
	TestMemory(code, data);
}
