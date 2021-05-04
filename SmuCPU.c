/****************************************************************
*	Copyright (C) 2021 ����
*?? ���ߣ�xwang
*?? ����ʱ�䣺2021-5-4 ���� 03:36:28
*
*?? ����˵����
*
*?? �޸���ʷ��2021-5-4 ���� 03:36:28
*
*
*****************************************************************/


/*********************************************************************
*					���ļ�
*********************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h> 


/*********************************************************************
*					��ָ��
*********************************************************************/

#define SIZE_CODE 2047
#define SIZE_DATA 255
//����8086CPU,FLAG�У�OF->11,DF->10,IF->9,TF->8,SF->7,ZF->6,AF->4,PF->2,CF->0
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
*					��������
*********************************************************************/
//���峣�üĴ���
short int AX = 0, BX = 0, CX = 0, DX = 0,
FLAG = 0,
DS = 0,
SS = SIZE_DATA,
SP = 0;

//Immediate Data
short int ImData;
//��λÿ��ָ���λ��
char* IP;
//ָ���ʶ��ĩβ��ʵ�ֱ��빦�ܣ�
short int TAG_last = 0;


//ָ�
char OrderSet[14][5] = { "MOV","ADD","SUB","INC","DEC","MUL","DIV","XOR",
"PUSH","POP","CMP","JE","CALL","RET" };

//������ʶ�����飬ʵ�����ݶ����Զ����ʶ�������������
struct TAG {
	char tag[10];
	short int arr;   //�����׵�ַ
}TagNode[5];

/*********************************************************************
*					����˵��
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
	//ʵ�ֽ���DW�������ݴ���������
	˼·����ȡ�Զ����ַ�����뵽�ṹ���У�ͬʱ�����������������д�ŵ��׵�ַ
*/
void GetWArray(char* buff, short int* data, short int* data_offset) {
	char* wbuff = buff;
	short int cursor = 0;
	short int sum = 0;
	char temp[20];
	wbuff++;

	//�Կո�Ϊ��λ������ȡ�Զ����ַ�
	while (*wbuff != ' ') {
		temp[cursor] = *wbuff;
		cursor++;
		wbuff++;
	}
	temp[cursor] = '\0';

	/*
	�û��Զ����ʶ�����ַ��
	�������ڱ��빦�ܵ����ã�CPU�޷�ʵ�֣��˴����ܲ����˽ṹ��ʵ�֣�
	�γɹ�ϵ�����Զ����ʶ���ɶ�λ������������������
	*/
	TagNode[TAG_last].arr = *data_offset;
	strcpy(TagNode[TAG_last].tag, temp);
	TAG_last++;

	//�����ݴ��뵽������
	wbuff = wbuff + 4;

	/*
		���ļ���ASCII����ʽ��ŵ�����תΪ����
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
�����������ݣ��ֱ�洢������Ĵ�������������
˼·���ȶԹؼ���->��ת����Ӧ�洢ָ��
	CX��DX��ʼ����Ϊ0���ʴ˴��������üĴ���CX��DX��Ϊ���������������Ŀ���
*/
void Storage(char* buff, char* code, short int* code_offset, short int* data, short int* data_offset) {

	//�洢����
	if (strstr(buff, "SSEG SEGMENT") != NULL) {
		DX++;
	}


	if (DX == 1) {
		/*
			�˴�Ϊ�˱���16λ�Ĵ���������ͳһ�����ݶ���ΪDW��ʽ
			����ʵ�ֲ�ͬ��λ��չ��ֻ���ټ�����չ�Ĵ���������DD��DB��������
		*/
		if (strstr(buff, "DW") != NULL) {
			GetWArray(buff, data, data_offset);
		}
	}

	if (strstr(buff, "SSEG ENDS") != NULL) {
		DX--;
	}


	//�洢����
	if (strstr(buff, "CSEG SEGMENT") != NULL) {
		CX++;
	}

	if (CX == 1) {
		//ֱ�ӽ��ļ��ж����Ĵ�����ASCII�����ʽ��������Ĵ�����
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

//��.txt�ļ������ݼ��ص�������
void load(char* code, short int* code_offset, short int* data, short int* data_offset) {
	FILE* fp = NULL;
	//���建������ʵ�ְ��д�ȡ
	char buff[255];


	fp=fopen("test.txt", "r");
	while (1) {
		//�������ֹͣ�ļ���ȡ
		if (feof(fp)) {  //feof()�ļ��ǿ�Ϊ0
			break;
		}
		else {
			//���ж��뻺����
			fgets(buff, 255, (FILE*)fp);
			Storage(buff, code, code_offset, data, data_offset);
		}
	}
	if (fclose(fp));
}

/*
	�����ؼ��ʣ��Ĵ���/�洢��Ԫ��
	�ȶ�
	���� �Ĵ���/�洢��Ԫ �ĵ�ַ
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

	//temp[]��Ϊ�Ĵ���/�洢��Ԫ�ı�ʶ���������ڼĴ���������ҼĴ����ĵ�ַ
	char temp[10] = { '\0' };

	char* p = NULL;

	//ָ�����յļĴ������ߴ洢��Ԫ�ĵ�ַ
	short int* result = NULL;

	/*
		��һ��������Ԫ��p�� ' '��λ
		Ȼ��p�������һλ��ָ���ַ�����һ���ַ�
	*/
	p = strchr(code_run_buff, ' ');
	p++;

	//����������֣��ͷ��ڼĴ�������ַ���˵�������õ��˼Ĵ��������ҳ���Ӧ�Ĵ���
	/*
		pָ���ַ�����һ���ַ�
		�ж�p������
			���������
				�ͷ����������Ĵ�����,�����������Ĵ�����ַ
			����
				����SearchReg���������ҸüĴ������ߴ洢��Ԫ
				�����ؼĴ���/�洢��Ԫ�ĵ�ַ
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

	//temp[]��Ϊ�Ĵ���/�洢��Ԫ�ı�ʶ���������ڼĴ���������ҼĴ����ĵ�ַ
	char temp[10] = { '\0' };

	char* p = NULL;

	//ָ�����յļĴ������ߴ洢��Ԫ�ĵ�ַ
	short int* result = NULL;

	/*
		��һ��������Ԫ��p�� ' '��λ
		Ȼ��p�������һλ��ָ���ַ�����һ���ַ�
	*/
	p = strchr(code_run_buff, ',');
	p++;

	//����������֣��ͷ��ڼĴ�������ַ���˵�������õ��˼Ĵ��������ҳ���Ӧ�Ĵ���
	/*
		pָ���ַ�����һ���ַ�
		�ж�p������
			���������
				�ͷ����������Ĵ�����,�����������Ĵ�����ַ
			����
				����SearchReg���������ҸüĴ������ߴ洢��Ԫ
				�����ؼĴ���/�洢��Ԫ�ĵ�ַ
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

	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ��ͷ�Ƕ��ţ�ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	*pFData = *pSData;

}

void XOR(char code_run_buff[], short int* data) {
	//XOR [pFData],[pSData]
	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ��ͷ�Ƕ��ţ�ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) ^ (*pSData);

}

void MUL(char code_run_buff[], short int* data) {
	//MUL [pFData],[pSData]
	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ��ͷ�Ƕ��ţ�ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) * (*pSData);

}

void DIV(char code_run_buff[], short int* data) {
	//DIV [pFData],[pSData]
	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ��ͷ�Ƕ��ţ�ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	//�����ж�
	if ((*pSData) != 0) {
		(*pFData) = (*pFData) / (*pSData);
	}
	else {
		printf("Error:0 is not divisible!");
	}

}

void ADD(char code_run_buff[], short int* data) {
	//ADD [pFData],[pSData]
	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ��ͷ�Ƕ��ţ�ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) + (*pSData);
}

void SUB(char code_run_buff[], short int* data) {
	//SUB [pFData],[pSData]
	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ��ͷ�Ƕ��ţ�ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	(*pFData) = (*pFData) - (*pSData);
}

void INC(char code_run_buff[], short int* data) {
	//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pFData = NULL;

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);

	(*pFData)++;
}

void DEC(char code_run_buff[], short int* data) {
	//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pFData = NULL;

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);

	(*pFData)--;
}


//����8086CPU,OF->11,DF->10,IF->9,TF->8,SF->7,ZF->6,AF->4,PF->2,CF->0

/*
	ͨ��λ����ʵ�ֶ�FLAG�Ĵ�����д��Ͷ���
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

	short int siTemp;//�����ݴ�oprd1-oprd2
	short int* pFData = NULL;//Ŀ�Ĳ���������ͷ�ǿո�ĩβ�Ƕ���
	short int* pSData = NULL;//Դ������ ĩβΪ���з�

	//��ȡ�����ַ�����Ĵ��������Ƚϣ�ƥ����ָ��->�Ĵ���/�洢��Ԫ
	pFData = FData(code_run_buff, data);
	pSData = SData(code_run_buff, data);

	//����󣬸��ݲ�ֵ����FLAG�Ĵ���
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
	�ڻ���У����������ݶκͶ�ջ��
		����������
		<<<<<<<[Stack]********[Data]>>>>>>>>>>
	��Ϊ��������
	�����ｫ���ݶκͶ�ջ�ι��������ڵ�ͬһƬ������
		����������
		[Data]>>>>>>>>>>********<<<<<<<[Stack]
		��������ҪԤ����Դ�һЩ�洢�ռ�
*/
void PUSH(char code_run_buff[], short int* data) {
	short int* pFData = NULL;
	pFData = FData(code_run_buff, data);
	*(data + (SS + SP)) = (*pFData);
	SP--;
}

/*
	PUSH register;
	PUSH_N imm��
*/
void PUSH_N(short int num, short int* data) {
	data[SS + SP] = num;
	SP--;
}

/*
	POP register;
	POP_N imm��
*/
void POP(char code_run_buff[], short int* data) {
	short int* pFData = NULL;
	pFData = FData(code_run_buff, data);
	SP++;
	(*pFData) = data[SS + SP];
}

/*
	POP register;
	POP_N imm��
*/
short int POP_N(short int* data) {
	SP++;
	return data[SS + SP];
}

//�Ӻ��������У���ȡ�Ӻ�����ʶ��������λ
char* SearchFlag(char code_run_buff[], char* code) {
	char temp[20] = { '\0' };
	char* p = NULL;
	char* result = NULL;//ָ�����յļĴ�����������
	p = strchr(code_run_buff, ' ');
	p++;

	//pָ���ַ�����һ���ַ�����������֣��ͷ��ڼĴ�������ַ���˵�������õ��˼Ĵ��������ҳ���Ӧ�Ĵ���

	//��ȡ���Ӻ�����ʶ������������
	short int iTemp = 0;
	for (; *p != ',' && *p != '\n'; iTemp++, p++) {
		temp[iTemp] = *p;
	}

	//�ԡ�������׺��ʽ��λ�����
	short int iiTemp = iTemp;  //��ʱ����������������ʱ����Ӱ��Ϊ����һ����PROC����׺����
	temp[iiTemp++] = ':';
	if (result = strstr(code, temp)) {
		return result;
	}

	//�ԡ�PROC����׺��ʽ��λ�����
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
	���غ�����ʶ�����ڵ��׵�ַ
	���ú�ֵ��PC��ʵ��ת��ָ��
*/
char* JMP(char code_run_buff[], char* code) {

	return SearchFlag(code_run_buff, code);

}

/*
	���FLAG�Ĵ�����ZF��ֵ
	��CMPָ����ϣ�������������ȣ���ZF��־λΪ1����ת�Ƶ�JE ����Ӻ���
*/
char* JE(char code_run_buff[], char* code) {
	if (FlagRead(ZF)) {
		return JMP(code_run_buff, code);
	}
	return NULL;
}

/*
	ִ�д�����
	˼·��
		���ݹؼ��ʶ�λ������MAIN
		������δӴ���������ȡ��
		��һ�ȶԹؼ��ʣ�ȷ�ϸ���ָ���
		�����Ӧ�Ĳ���
*/
void run(char* code, short int* data) {
	//���建����
	char code_run_buff[255] = { '\0' };  //���ַ�����������Ϊ�����
	char* pcode_run_buff = code_run_buff;

	//CS+ip��̬�ض�λ
	char* CS = code;
	char* IP = strstr(code, "MAIN PROC");
	short int ip = 0;
	char* pcode = IP;

	//ִ�г���
	while (*pcode != '\0') {
		for (; *pcode != '\n'; pcode++) {
			*pcode_run_buff = *pcode;
			pcode_run_buff++;
		}
		*pcode_run_buff = '\n';

		//�ȶԹؼ���
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

		//PCָ����һ��
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
				//��α�����Զϵ����ѵ㣬��ret���

				/*
					�ȱ���ϵ�
					Ȼ���ӳ�����������׵�ַ����pcode
				*/
				IP = pcode;
				PUSH_N(IP - CS, data);
				pcode = JMP(code_run_buff, code);

			}
		}

		if (IP = strstr(code_run_buff, "RET")) {

			/*
				POP(IP);
				����Ӷϵ㴦��ʼ��������
			*/
			ip = POP_N(data);
			pcode = CS + ip;

		}

		//���������ⲿ��buffer�ռ䣬������һ�г���
		memset(code_run_buff, '\0', 255);
		pcode_run_buff = code_run_buff;
	}
}

/*
	void TestMemory(char* code, short int* data)
	չʾCPU�Ĵ�������������������������Լ��Զ����ַ��Ĺ�����
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
	//���������д�����������������С���ں궨�����޸�
	char code[SIZE_CODE + 1] = { '\0' };
	short int data[SIZE_DATA + 1] = { 0 };
	short int code_offset = 0;
	short int data_offset = 0;

	//���أ�Դ�ļ�->���棩
	load(code, &code_offset, data, &data_offset);

	//չʾ����ǰ��������
	TestMemory(code, data);

	//���������еĳ���
	run(code, data);

	//չʾ���к���������
	TestMemory(code, data);
}
