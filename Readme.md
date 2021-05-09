@[TOC](C语言模拟RS结构CPU)

# 一、说明
C语言实现CPU仿真，可编程，可扩展

编程模板在==template.txt==里

程序默认载入测试程序文档==test.txt==

提供测试文件
1、==test.txt ==斐波那契数列
2、==testFAC.txt==阶乘
## 操作方法

编译运行==SmuCPU.c==文件即可
>运行结果会展示CPU完整的**寄存器组**和主存的**代码区**、**数据区**、**自定义标识符映射表**

## 编程方法
way_1.在==test.txt==文件中编写汇编代码即可
way_2.在==template.txt==写入汇编代码后，在==SmuCPU.c==修改路径即可

```c
	fp=fopen("test.txt", "r");//第231行
```

# 二、指令集
RISC指令集
```c
erum OrderSet={ "MOV","ADD","SUB","INC","DEC","MUL","DIV","XOR",
"PUSH","POP","CMP","JE","CALL","RET" }
```



# 三、测试代码说明（test.txt)
## 1、题目(斐波那契数列——递归子函数）
>递归子程序设计

>给定一个正数N≥1存放在NUM单元中，试编制一递归子程序计算FIB（N），并将结果存入RESULT单元中。
Fibonacci数的定义如下：
     FIB（1）= 1
     FIB（2）= 1
     FIB（n）= FIB（n-2）+ FIB（n-1）  n>2
数据段中至少需要定义以下内容：
（1）ID  db  xxH, yyH, zzH  (说明：以学号2186123456为例，其中的xx为12，yy为34，zz为56)
（2）定义NUM、RESULT的内存单元

## 2、题解
即为test.txt的内容
```c
SSEG SEGMENT
	ID DW 41,26,35
	NUM DW 10
	RESULT DW 0
SSEG ENDS

CSEG SEGMENT
	ASSUME CS:CSEG,DS:SSEG
	FIB PROC NEAR
		CMP BX ,1
		JE FIB1
		CMP BX,2
		JE FIB2
		PUSH BX
		DEC BX
		CALL FIB
		POP BX
		MOV CX,AX
		ADD AX,DX
		MOV DX,CX
		RET
	FIB1:
		MOV AX,1
		RET
	FIB2:
		MOV AX,1
		MOV DX,1
		RET
	FIB ENDP
	
	MAIN PROC far
	START:
		XOR BX,BX
		MOV BX, NUM
		CALL FIB
		MOV RESULT,AX
	MAIN ENDP
CSEG ENDS
END START
```

## 3、DOS系统运行结果
以NUM=5为例子
### 运行前
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210504193859408.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0ZvdXJpZXJGaXNoZXI=,size_16,color_FFFFFF,t_70)

### 运行后
![在这里插入图片描述](https://img-blog.csdnimg.cn/2021050419405357.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0ZvdXJpZXJGaXNoZXI=,size_16,color_FFFFFF,t_70)

## 4、C语言仿真CPU运行结果
### 运行前
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210504194359467.png)

### 运行后

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210504194525796.png)


### NUM的值可以在test.txt文件中自由更改
*eg1：*
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210504194842460.png)
*eg2：*
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210504195018681.png)

## 5、gcc编译
>gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04) 

![在这里插入图片描述](https://img-blog.csdnimg.cn/2021050920130988.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0ZvdXJpZXJGaXNoZXI=,size_16,color_FFFFFF,t_70)

