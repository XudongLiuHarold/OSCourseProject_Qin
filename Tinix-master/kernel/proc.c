
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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


void initializeAllPro()	//ÖØÐÂ³õÊ¼»¯ËùÓÐµÄ½ø³Ì£¬¼ÓÈëµ½²»Í¬µÄ½ø³ÌÓÅÏÈ¼¶¶ÓÁÐÖÐ
{
	PROCESS* p;
	firstLen=0;
	secondLen=0;
	firstHead=0;
	int i;
	for (i=0; i<NR_TASKS+NR_PROCS; i++)
	{
		p=&proc_table[i]; 
		if (p->state!=kRUNNABLE) continue;
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
			p->ticks=p->priority;
			p->whichQueue=3;
		}
	}
}

/*======================================================================*
                              schedule
 *======================================================================*/


PUBLIC void schedule()
{
	PROCESS*	p;
	int 		greatest_priority=0;
	int 		i;
	
	while (!greatest_priority) 
	{
		if (firstLen-firstHead>0)
		{		
			p_proc_ready=firstQueue[firstHead];	//µÚÒ»¸ö¶ÓÁÐ°´ÕÕÏÈµ½ÏÈµÃ
			greatest_priority=p_proc_ready->ticks;
			break;
		}
		else if (secondLen-firstHead>0)						//µÚ¶þ¸ö¶ÓÁÐ°´ÕÕÓÅÏÈ¼¶
		{
			for (i=0; i<secondLen; i++)		//µÚ¶þ¸ö¶ÓÁÐÔöÉèÅÐ¶ÏÊÇ·ñÎªrunnable×´Ì¬
			{
				p=secondQueue[i];
				if (p->state!=kRUNNABLE || p->ticks==0) continue;
				if (p->ticks > greatest_priority) 
				{
					greatest_priority = p->ticks;
					p_proc_ready = p;
				}
				/*{	ÏÂÃæ´úÂë±»¿¨ËÀ¡£¡£¡£ÎÒÒ²²»¶®ÎªÊ²Ã´¡£¡£
					if (p->priority>greatest_priority && p->ticks!=0)
					{
						greatest_priority=p->priority;
						p_proc_ready=p;
					}
				}*/
			}
		}
		else//third queue
		{
			for (i = 0; i < thirdLen; ++i)
			{
				p_proc_ready=thirdQueue[firstHead];	
				greatest_priority=p_proc_ready->ticks;
				break;
			}
		}
		if (!greatest_priority)	initializeAllPro();
	}
	p_proc_ready->state=kRUNNING;
}


/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

