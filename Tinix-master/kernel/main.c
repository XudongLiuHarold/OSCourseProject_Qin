
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"
#include "stdlib.h"
     
int strcmp(char *str1,char *str2)
{
	int i;
	for (i=0; i<strlen(str1); i++)
	{
		if (i==strlen(str2)) return 1;
		if (str1[i]>str2[i]) return 1;
		else if (str1[i]<str2[i]) return -1;
	}
	return 0; 
}

void strlwr(char *str)
{
	int i;
	for (i=0; i<strlen(str); i++)
	{
		if ('A'<=str[i] && str[i]<='Z') str[i]=str[i]+'a'-'A';
	}
}

void addToQueue(PROCESS* p)
{
	p->state=kRUNNABLE;
	if (p->priority>=10)
	{
		firstQueue[firstLen]=p;
		firstLen++;
		p->ticks=2;
		p->whichQueue=1;
	}
	else if (p->priority>=5)
	{
		secondQueue[secondLen]=p;
		secondLen++;
		p->ticks=p->priority;
		p->whichQueue=2;
	}
	else 
	{
		thirdQueue[thirdLen]=p;
		thirdLen++;
		p->ticks=5;
		p->whichQueue=3;
	}
}

/*======================================================================*
                            tinix_main
 *======================================================================*/
PUBLIC int tinix_main()
{
	//disp_str("-----\"tinix_main\" begins-----\n");
	clearScreen();
	disp_str("         *************************************************************\n");
	disp_str("        *                                                             \n");
	disp_str("       * **********    ******    ***      **    ******    **    **    \n");
	disp_str("      *      **          **      ****     **      **        *  *      \n");
	disp_str("     *       **          **      ** **    **      **         **       \n");
	disp_str("    *        **          **      **  **   **      **         **       \n");
	disp_str("   *         **          **      **   **  **      **         **       \n");
	disp_str("  *          **          **      **    ** **      **         **       \n");
	disp_str(" *           **          **      **     ****      **       *    *     \n");
	disp_str("*            **        ******    **      ***    ******   **      **   \n");
	disp_str("*                                                                     \n");
	disp_str("**********************************************************************\n");
	TASK*		p_task;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	t_16		selector_ldt	= SELECTOR_LDT_FIRST;
	int		i;
	t_8		privilege;
	t_8		rpl;
	int		eflags;
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		if (i < NR_TASKS) {	/* 任务 */
			p_task		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl		= RPL_TASK;
			eflags		= 0x1202;	/* IF=1, IOPL=1, bit 2 is always 1 */
		}
		else {			/* 用户进程 */
			p_task		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl		= RPL_USER;
			eflags		= 0x202;	/* IF=1, bit 2 is always 1 */
		}

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid	= i;			/* pid */

		p_proc->ldt_sel	= selector_ldt;
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;	/* change the DPL */
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;/* change the DPL */
		p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs		= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip	= (t_32)p_task->initial_eip;
		p_proc->regs.esp	= (t_32)p_task_stack;
		p_proc->regs.eflags	= eflags;

		p_proc->nr_tty		= 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	//修改这里的优先级和ticks
	proc_table[0].priority = 15;
	proc_table[1].priority =  5;
	proc_table[2].priority =  2;
	proc_table[3].priority =  3;
	proc_table[4].priority =  7;
	proc_table[5].priority =  10;
	proc_table[6].priority =  10;//calendar
	proc_table[7].priority =  20;//guess
	proc_table[8].priority =  14;//2048


	firstLen=firstHead=secondLen=thirdLen=0;
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		addToQueue(proc_table+i);
	}
	//指定控制台
	proc_table[1].nr_tty = 0;
	proc_table[2].nr_tty = 1;
	proc_table[3].nr_tty = 1;
	proc_table[4].nr_tty = 1;
	proc_table[5].nr_tty = 1;
	proc_table[6].nr_tty = 2; //calendar Alt+F3
    proc_table[7].nr_tty = 3;//guss game Alt+F4
	proc_table[8].nr_tty = 4; //2048 Alt+F5

	k_reenter	= 0;
	ticks		= 0;

	p_proc_ready	= proc_table;

	init_clock();

	restart();

	while(1){}
}

void clearScreen()
{
	int i;
	disp_pos=0;
	for(i=0;i<80*25;i++)
	{
		disp_str(" ");
	}
	disp_pos=0;
}


void help()
{
	printf("           *////////////////////////////////////////////*/\n");
	printf("                   design by Doubi && LiangPuHe         \n");
	printf("           *////////////////////////////////////////////*/\n");
	printf("\n");
	printf("      *////////////////////////////////////////////////////////*\n");
	printf("      *////  help         --------  shwo the help menu     ////*\n");
	printf("      *////  clear        --------  clear screen           ////*\n");
	printf("      *////  alt+F2       --------  show the process run   ////*\n");
	printf("      *////  alt+F3       --------  calendar               ////*\n");
	printf("      *////  kill 2~5     --------  kill the process 2~5   ////*\n");
	printf("      *////  start 2~5    --------  start the process 2~5  ////*\n");
	printf("      *////  show         --------  show the process state ////*\n");
	printf("      *////////////////////////////////////////////////////////*\n");
	printf("\n");
}

void show()
{
	PROCESS* p;
	int i;
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		p=&proc_table[i];
		printf("process%d:",p->pid);
		switch (p->state)
		{
		case kRUNNABLE:
			printf("    Runnable\n");
			break;
		case kRUNNING:
			printf("    Running\n");
			break;
		case kREADY:
			printf("    Ready\n");
			break;
		}
		printf(p->whichQueue);
		printf("\n");
	}
}

void readOneStringAndOneNumber(char* command,char* str,int* number)
{
	int i;
	int j=0;
	for (i=0; i<strlen(command); i++)
	{
		if (command[i]!=' ') break;
	}
	for (; i<strlen(command); i++)
	{
		if (command[i]==' ') break;
		str[j]=command[i];
		j++;
	}
	for (; i<strlen(command); i++)
	{
		if (command[i]!=' ') break;
	}

	*number=0;
	for (; i<strlen(command) && '0'<=command[i] && command[i]<='9'; i++)
	{
		*number=*number*10+(int) command[i]-'0';
	}
}

void dealWithCommand(char* command)
{
	strlwr(command);
	if (strcmp(command,"clear")==0)
	{
		clearScreen();
		sys_clear(tty_table);
		return ;
	}
	if (strcmp(command,"help")==0)
	{
		help();
		return ;
	}
	if (strcmp(command,"show")==0)
	{
		show();
		return ;
	}
	char str[100];
	int number;
	readOneStringAndOneNumber(command,str,& number);
	if (strcmp(str,"kill")==0)
	{
		if (number<0 || number>NR_TASKS+NR_PROCS)
		{
			printf("No found this process!!");
		}
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
		{
			proc_table[number].state=kREADY;
			printf("kill process %d successful\n",number);
		}
		return ;
	}
	if (strcmp(str,"start")==0)
	{
		if (number<0 || number>NR_TASKS+NR_PROCS)
		{
			printf("No found this process!!");
		}
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
		{
			proc_table[number].state=kRUNNABLE;
			printf("start process %d successful\n",number);
		}
		return ;
	}
	printf("can not find this command\n");
}

/*======================================================================*
                               Terminal
 *======================================================================*/
void Terminal()
{
	TTY *p_tty=tty_table;
	p_tty->startScanf=0;
	while(1)
	{
		printf("DB=>");
		//printf("<Ticks:%x>", get_ticks());
		openStartScanf(p_tty);
		while (p_tty->startScanf) ;
		dealWithCommand(p_tty->str);
	}
}


/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0;
	while(1){
		printf("B");
		milli_delay(1000);
	}
}



/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	int i = 0;
	while(1){
		printf("C");
		milli_delay(1000);
	}
}

void TestD()
{
	int i=0;
	while (1)
	{
		printf("D");
		milli_delay(1000);
	}
}

void TestE()
{
	int i=0;
	while (1)
	{
		printf("E");
		milli_delay(1000);
	}
}


/*======================================================================*
				guess game
*=======================================================================*/
TTY *gussTTy = tty_table+3;
void guess()
{
	char gamer;  // 玩家出拳
    int computer;  // 电脑出拳
    int result;  // 比赛结果

    // 为了避免玩一次游戏就退出程序，可以将代码放在循环中
    while (1){
        printf("This is a guess game, please input your choice: ");
        printf("\nA:scissors\nB:stone\nC:cloth\nD:end game\n");
        do 
        {
        	gamer = getUserInput();
        }
        while(gamer == -1);
        if (gamer == 0)
        {
        	return;
        }
       
        computer=getRandom();  // 产生随机数并取余，得到电脑出拳
        result=gamer+computer;  
        printf("Computer....:");
        switch (computer)
        {
            case 0:printf("scissors\n");break; //4    1
            case 1:printf("stone\n");break; //7  2
            case 2:printf("cloth\n");break;   //10 3
        }
        printf("Your....:");
        switch (gamer)
        {
            case 4:printf("scissors\n");break;
            case 7:printf("stone\n");break;
            case 10:printf("cloth\n");break;
        }
        if (result==6||result==7||result==11) printf("You win!\n");
        else if (result==5||result==9||result==10) printf("Computer win!\n");
        else printf("deuce\n");

        milli_delay(1000);
        clearScreen();  // 暂停并清屏
    }
}


/*======================================================================*
 Game2048
 *=======================================================================*/
TTY *gameTty = tty_table+4;

int num[4][4];
int score, gameover, ifappear, gamew, gamef,move;
int key;
void explation()
{
    printf("*****************************************\n");
    printf("*****************************************\n");
    printf("****************  rules   ***************\n");
    printf("*****************************************\n");
    printf("*****************************************\n");
//    printf("玩家可以选择上、下、左、右或W、A、S、D去移动滑块\n");
//    printf("玩家选择的方向上若有相同的数字则合并\n");
//    printf("合并所得的所有新生成数字相加即为该步的有效得分\n");
//    printf("玩家选择的方向行或列前方有空格则出现位移\n");
//    printf("每移动一步，空位随机出现一个2或4\n");
//    printf("棋盘被数字填满，无法进行有效移动，判负，游戏结束\n");
//    printf("棋盘上出现2048，获胜，游戏结束\n");
//    printf("按上下左右去移动滑块\n");
//    printf("请按任意键返回主菜单...\n");
}
void gamefaile()
{
    int i, j;
    printf("*****************************************\n");
    printf("*****************************************\n");
    printf("***************   you fail   ************\n");
    printf("*****************************************\n");
    printf("*****************************************\n");
    printf("---------------------\n");
    for (j = 0; j<4; j++)
    {
        for (i = 0; i<4; i++)
            if (num[j][i] == 0)
                printf("|    ");
            else
                printf("|   %d", num[j][i]);
        printf("|\n");
        printf("---------------------\n");
    }
//    printf("你的成绩：%d,移动了%d步\n", score,move);
//    printf("请按任意键返回主菜单...\n");

}
void gamewin()
{
    int i, j;
    printf("*****************************************\n");
    printf("*****************************************\n");
    printf("****************   you win   ************\n");
    printf("*****************************************\n");
    printf("*****************************************\n");
    printf("---------------------\n\t\t\t");
    for (j = 0; j<4; j++)
    {
        for (i = 0; i<4; i++)
            if (num[j][i] == 0)
                printf("|    ");
            else
                printf("|   %d", num[j][i]);
        printf("|\n");
        printf("---------------------\n");
    }
//    printf("你的成绩：%d,移动了%d步\n", score,move);
//    printf("请按任意键返回主菜单...\n");
}
void prin()
{
    int i, j;
    printf("*****************************************\n");//输出界面
    printf("*****************************************\n");
    printf("***************   start   ***************\n");
    printf("*****************************************\n");
    printf("*****************************************\n");
//    printf("\t\t      请按方向键或W、A、S、D移动滑块\n");//输出操作提示语句
//    printf("\t\t          按ESC返回至主菜单\n");
    printf("---------------------\n");
    for (j = 0; j<4; j++)                 //输出4*4的表格
    {
        for (i = 0; i<4; i++)
            if (num[j][i] == 0)
                printf("|    ");
            else
                printf("|   %d", num[j][i]);
        printf("|\n");
        printf("---------------------\n");
    }
//    printf("你的成绩：%d，移动了%d步\n", score,move);//输出得分和移动步数
}



void appear()
{
    int i, j,ran,t[16],x=0,a,b;
//    srand((int)time(0));          //随机种子初始化
    for (j = 0; j < 4; j++)      //将空白的区域的坐标保存到中间数组t中
        for (i = 0; i < 4;i++)
            if (num[j][i] == 0)
            {
                t[x] = j * 10 + i;
                x++;
            }
    if (x == 1)            //在t中随机取一个坐标
        ran = x - 1;
    else
        ran = 10 % (x - 1);
    a = t[ran] / 10;      //取出这个数值的十位数
    b = t[ran] % 10;     //取出这个数值的个位数
//    srand((int)time(0));
    if ((10 % 9)>2)    //在此空白区域随机赋值2或4
        num[a][b] = 2;
    else
        num[a][b] = 4;
}

void close()
{
    return ;
}

void add(int *p)
{

    int i=0, b;
    while (i<3)
    {
        if (*(p + i) != 0)
        {
            for (b = i + 1; b < 4; b++)
            {
                if (*(p + b) != 0)
                    if (*(p + i) == *(p + b))
                    {
                    score = score + (*(p + i)) + (*(p + b));
                    *(p + i) = *(p + i) + *(p + b);
                    if (*(p + i) == 2048)
                        gamew = 1;
                    *(p + b) = 0;
                    i = b + i;
                    ++ifappear;
                    break;
                    }
                    else
                    {
                        i = b;
                        break;
                    }
            }
            if (b == 4)
                i++;
        }
        else
            i++;
    }

}

void readNumber(int *x){
	int i = 0;
	*x = 0;
	for (i = 0; i < gameTty->len && gameTty==' '; i++);
	for (; i < gameTty->len && gameTty->str[i] != ' ' && gameTty->str != '\n'; i++){
		*x = (*x)*10+(int) gameTty->str[i] - 48;	
	}
}

void Gameplay()
{
    int i, j, g, e, a, b[4];
    appear();
    appear();
    while (1)
    {
        if (ifappear!=0) 
            appear();
        sys_clear(gameTty);
        prin();
	int x;
        openStartScanf(gameTty);
        while(gameTty->startScanf);
        readNumber(&x);
        key = x;
        switch (key)
        {
        case 39:
        case 71:
        case 72:
            ifappear = 0;
            for (j = 0; j < 4; j++)
            {
                for (i = 0; i < 4; i++)
                {
                    b[i] = num[i][j];
                    num[i][j] = 0;
                }
                add(b);
                e = 0;
                for (g = 0; g < 4; g++)
                {
                    if (b[g] != 0)
                    {
                        num[e][j] = b[g];
                        if (g != e)
                            ++ifappear;
                        e++;
                    }
                }
            }
            if (ifappear!=0)
                ++move;
        break;
        case 35:
        case 67:
        case 80:
            ifappear = 0;
            for (j = 0; j < 4; j++)
            {
                for (i = 0; i < 4; i++)
                {
                    b[i] = num[i][j];
                    num[i][j] = 0;
                }
                add(b);
                e = 3;
                for (g = 3; g>=0; g--)
                {
                    if (b[g] != 0)
                    {
                        num[e][j] = b[g];
                        if (g != e)
                            ++ifappear;
                        e--;
                    }
                }
            }
            if (ifappear != 0)
                ++move;
        break;
        case  17:
        case  49:
        case  75:
            ifappear = 0;
            for (j = 0; j < 4; j++)
            {
                for (i = 0; i < 4; i++)
                {
                    b[i] = num[j][i];
                    num[j][i] = 0;
                }
                add(b);
                e = 0;
                for (g = 0; g < 4; g++)
                {
                    if (b[g] != 0)
                    {
                        num[j][e] = b[g];
                        if (g!=e)
                            ++ifappear;
                        e++;
                    }
                }
            }
            if (ifappear != 0)
                ++move;
        break;
        case  20:
        case  52:
        case  77:
            ifappear = 0;
            for (j = 0; j < 4; j++)
            {
                for (i = 0; i < 4; i++)
                {
                    b[i] = num[j][i];
                    num[j][i] = 0;
                }
                add(b);
                e = 3;
                for (g = 3; g >=0; g--)
                {
                    if (b[g] != 0)
                    {
                        num[j][e] = b[g];
                        if (g!=e)
                            ++ifappear;
                        e--;
                    }
                }
            }
            if (ifappear != 0)
                ++move;
        break;
        case 27:
            break;

        }
        for (j = 0; j < 4; j++)
        {
            for (i = 0; i < 4; i++)
            {
                if (j < 3)
                {
                    if (i < 3)
                    {
                        if (num[j][i] == num[j + 1][i] || num[j][i] == num[j][i + 1] || num[j][i] == 0)
                        {
                            gamef = 0;
                            break;
                        }
                        else
                            gamef = 1;
                    }
                    else
                    {
                        if (num[j][i] == num[j + 1][i] || num[j][i] == 0)
                        {
                            gamef = 0;
                            break;
                        }
                        else
                            gamef = 1;
                    }
                }
                else
                {
                    if (i < 3)
                    {
                        if (num[j][i] == num[j][i + 1] || num[j][i] == 0 || num[j][i + 1] == 0)
                        {
                            gamef = 0;
                            break;
                        }
                        else
                            gamef = 1;
                    }
                }

            }
            if (gamef == 0)
                break;
        }
        if (gamef == 1 || gamew == 1)
            break;

    }
    if (gamef == 1)
        gamefaile();
    else
        gamewin();
}

void menu()
{
    int n;
    printf("****************************************\n");            //输出游戏菜单的图形
    printf("*              1.start game            *\n");
    printf("*              2.game rule             *\n");
    printf("*              3.quit                  *\n");
    printf("****************************************\n");
    printf("please input 1,2or3[] ");
    int x;
    openStartScanf(gameTty);
    while(gameTty->startScanf);
    readNumber(&x);
    n = x;
    printf("%d",n);
    //scanf("%d", &n);
    switch (n)
    {
    case 1:
	sys_clear(gameTty);
        Gameplay();                                                         //游戏开始函数
        break;
    case 2:
        explation();                                                       //游戏规则函数
        break;
    case 3:
        close();                                                          //关闭游戏函数
        break;
    }
}

int goBangGame()
{   
    int j, i;
    for (j = 0; j < 4; j++)             //对4*4进行初始赋值为0
        for (i = 0; i < 4; i++)
            num[j][i] = 0;
    gamew = 0;                        //游戏获胜的判断变量初始化
    gamef = 0;                       //游戏失败的判断变量初始化
    ifappear = 0;                   //判断是否应该随机出现2或4的变量初始化
    score = 0;                     //游戏得分变量初始化
    gameover = 0;                 //游戏是否结束的变量初始化
    move = 0;                    //游戏的移动步数初始化
    menu();                     //调用主菜单函数
    while(1);
    return 0;
}
int getRandom() {
	return ticks%3;
}

int getUserInput() {
	openStartScanf(gussTTy);
	while(gussTTy->startScanf) ;
	if (strcmp(gussTTy->str, "a") == 0)
	{
		return 4;
	}
	else if (strcmp(gussTTy->str, "b") == 0) {
		return 7;
	}
	else if (strcmp(gussTTy->str, "c") == 0) {
		return 10;
	}
	else if (strcmp(gussTTy->str, "d") == 0) {
		return 0;
	}
	else {
		printf("您的输入有误\n");
		return -1;
	}
}


/*======================================================================*
				Calender of July, 2015
*=======================================================================*/
TTY *goBangGameTty=tty_table+2;
#define N 7
void readTwoNumber(int* x,int* y)
{
	int i=0;
	*x=0;
	*y=0;
	for (i=0; i<goBangGameTty->len && goBangGameTty->str[i]==' '; i++);
	for (; i<goBangGameTty->len && goBangGameTty->str[i]!=' '  && goBangGameTty->str[i]!='\n'; i++)
	{
		*x=(*x)*10+(int) goBangGameTty->str[i]-48;
	}
	for (i; i<goBangGameTty->len && goBangGameTty->str[i]==' '; i++);
	for (; i<goBangGameTty->len && goBangGameTty->str[i]!=' ' && goBangGameTty->str[i]!='\n'; i++)
	{
		*y=(*y)*10+(int) goBangGameTty->str[i]-48;
	}
}
void calendar()
{
		int year, month, x, y;
		while (1)
		{
			while (1)
			{
				printf("Please input the year and month: ");
				openStartScanf(goBangGameTty);
				while (goBangGameTty->startScanf) ;
				readTwoNumber(&x,&y);	
				year=x;
				month=y;
				rili(year,month);
			}
		}
}

void print(int day,int tian)
{
    int a[N][N],i,j,sum=1;
    for(i=0,j=0;j<7;j++)
    {
        if(j<day)
        printf("      ");
        else
        {
            a[i][j]=sum;
            printf("    %d",sum++);
            
        }
    }
    printf("\n");
    for(i=1;sum<=tian;i++)
    {
        for(j=0;sum<=tian&&j<7;j++)
        {
                a[i][j]=sum;
                printf("    %d",sum++);
        }
        printf("\n");
    }
}

int duo(int year)
{
    if(year%4==0&&year%100!=0||year%400==0)
    return 1;
    else
    return 0;
}

int rili(int year,int month)
{
   int day,tian,preday,strday;
   printf("***************%dmonth  %dyear*********\n",month,year);
   printf("  SUN    MON    TUE   WED   THUR   FRI   STAT\n");
   switch(month)
   {
   case 1:
    tian=31;
    preday=0;
   break;
   case 2:
        tian=28;
        preday=31;
    break;
    case 3:
        tian=31;
        preday=59;
    break;
    case 4:
        tian=30;
        preday=90;
    break;
    case 5:
        tian=31;
        preday=120;
    break;
    case 6:
        tian=30;
        preday=151;
    break;
    case 7:
        tian=31;
        preday=181;
    break;
    case 8:
        tian=31;
        preday=212;
    break;
    case 9:
        tian=30;
        preday=243;
    break;
    case 10:
        tian=31;
        preday=273;
    break;
    case 11:
        tian=30;
        preday=304;
    break;
    default:
        tian=31;
        preday=334;
    }
    if(duo(year)&&month>2)
    	preday++;

    if(duo(year)&&month==2)
    	tian=29;

    day=((year-1)*365+(year-1)/4-(year-1)/100+(year-1)/400+preday+1)%7;    
    print(day,tian);
}
