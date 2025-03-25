/*

*********************************************************************************************************

*                                                uC/OS-II

*                                          The Real-Time Kernel

*

* WIN32 PORT & LINUX PORT

*                          (c) Copyright 2004, Werner.Zimmermann@fht-esslingen.de

*                 (Similar to Example 1 of the 80x86 Real Mode port by Jean J. Labrosse)

*                                           All Rights Reserved

** *****************************************************************************************************

* Further modified by mikael.jakas@puv.fi, jukka.matila@vamk.fi

* *****************************************************************************************************

*                                               EXAMPLE #1

*********************************************************************************************************

*/



#include "includes.h" 



/*

*********************************************************************************************************

*                                               CONSTANTS

*********************************************************************************************************

*/



#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */ 
#define  N_TASKS                        2       /* Number of identical tasks                          */ 
#define MAX_X                           80 
#define MAX_Y                           25 
#define MAX_BOARD 20 
#define EMPTY_CHAR '*' 
#define WIN_COUNT 4 // Number of consecutive marks needed to win
#define DISPO 2

#define PLAYER_1_SIGN  'X' 
#define PLAYER_2_SIGN  'O' 

#define PLAYER_1 0x01 
#define PLAYER_2 0x02 
#define DRAW 0x04 
#define STATUS 0x08

/*

*********************************************************************************************************

*                                               VARIABLES

*********************************************************************************************************

*/
OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
/*

*********************************************************************************************************

*                                           FUNCTION PROTOTYPES

*********************************************************************************************************

*/

void TaskStart(void* data);        /* Function prototype of Startup task           */
void DrawBoard(void* data);
void TaskPLAYER_1(void* pdata);
void TaskPLAYER_2(void* pdata);
void CheckFlagStatus(void* data);

void Check_Keyboard(); /* Function prototype of Check_Keyboard Task     */
void Check_Winner();
void PlayerMove();
void InitialBoard();
void UpdateBoard();
void KillTast();

/*

*********************************************************************************************************

*                                                MAIN

*********************************************************************************************************

*/
//EVENT
OS_FLAG_GRP* GameLogic;
OS_FLAGS flags;
OS_EVENT* SemStatus;

/*DATABASE*/
INT32U board[MAX_BOARD][MAX_BOARD] = { 0 };

int  main(void)
{
    INT8U err;
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */
    OSInit();                                              /* Initialize uC/OS-II                      */
	GameLogic = OSFlagCreate(0, &err); //Create flag group

    OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
    return 0;
}
/*

*********************************************************************************************************

*                                              STARTUP TASK

*********************************************************************************************************

*/
void  TaskStart(void* pdata)
{
    OSTaskCreate(DrawBoard, (void*)0, &TaskStk[0][TASK_STK_SIZE - 1], 2);
    OSTaskCreate(TaskPLAYER_1, (void*)0, &TaskStk[2][TASK_STK_SIZE - 1], 3);
    OSTaskCreate(TaskPLAYER_2, (void*)0, &TaskStk[3][TASK_STK_SIZE - 1], 4);
    OSTaskCreate(CheckFlagStatus, (void*)0, &TaskStk[4][TASK_STK_SIZE - 1], 1);
    InitialBoard();
    Check_Keyboard();   /* Check for keyboard operations            */
}



/*

*********************************************************************************************************

*                                                  TASKS

*********************************************************************************************************

*/
void DrawBoard(void* data) {
    INT8U err;    
    OSFlagPost(GameLogic, STATUS, OS_FLAG_SET, &err);
    OSFlagPost(GameLogic, PLAYER_1, OS_FLAG_SET, &err);
    
    for (;;) {
        OSFlagPend(GameLogic, DRAW, OS_FLAG_WAIT_SET_ALL, 0, &err);
        UpdateBoard();
        
        
        flags = OSFlagAccept(GameLogic, PLAYER_1 + PLAYER_2, OS_FLAG_WAIT_CLR_ANY, &err);
        if ((flags & PLAYER_1) == PLAYER_1) {
            Check_Winner();
            OSFlagPost(GameLogic, PLAYER_1, OS_FLAG_CLR, &err);
            OSFlagPost(GameLogic, PLAYER_2 + STATUS, OS_FLAG_SET, &err);
        }
            
        else {
            Check_Winner();
            OSFlagPost(GameLogic, PLAYER_2, OS_FLAG_CLR, &err);
            OSFlagPost(GameLogic, PLAYER_1 + STATUS, OS_FLAG_SET, &err);
        }
            

        OSFlagPost(GameLogic, DRAW+STATUS, OS_FLAG_CLR, &err);
        OSTimeDlyHMSM(0, 0, 1, 0);
        
    }
    
}

void TaskPLAYER_1(void* pdata) {
    INT8U err;
    
    for (;;) {	
        OSFlagPend(GameLogic, PLAYER_1, OS_FLAG_WAIT_SET_ANY, 0, &err); //Wait all flags to be set
        PlayerMove();
        OSFlagPost(GameLogic, DRAW+STATUS, OS_FLAG_SET, &err);

        
        OSTimeDlyHMSM(0, 0, 1, 0);
        
    }
    
}



void TaskPLAYER_2(void* pdata) {
    INT8U err;
    
    for (;;) {
        OSFlagPend(GameLogic, PLAYER_2, OS_FLAG_WAIT_SET_ALL, 0, &err);
        PlayerMove();   
        
        OSFlagPost(GameLogic, DRAW, OS_FLAG_SET, &err);
        
        
        OSTimeDlyHMSM(0, 0, 1, 0);
        
    }
    
}

void CheckFlagStatus(void* data) {
    INT8U err;
    for (;;) {
        OSFlagPend(GameLogic, STATUS, OS_FLAG_WAIT_SET_ALL, 10, &err);
        flags = OSFlagAccept(GameLogic, PLAYER_1 + PLAYER_2 + DRAW+ STATUS, OS_FLAG_WAIT_SET_ALL, &err);
        PC_DispStr(30, 2, "P1", DISP_FGND_WHITE + DISP_BGND_BLACK);
        PC_DispChar(30, 3, (flags & PLAYER_1) + '0', DISP_FGND_WHITE + DISP_BGND_BLACK);
        
        PC_DispStr(34, 2, "P2", DISP_FGND_WHITE + DISP_BGND_BLACK);
        PC_DispChar(34, 3, ((flags & PLAYER_2) >> 1) + '0', DISP_FGND_WHITE + DISP_BGND_BLACK);
        
        PC_DispStr(38, 2, "DR", DISP_FGND_WHITE + DISP_BGND_BLACK);
        PC_DispChar(38, 3, ((flags & DRAW) >> 2) + '0', DISP_FGND_WHITE + DISP_BGND_BLACK);

        PC_DispStr(42, 2, "ST", DISP_FGND_WHITE + DISP_BGND_BLACK);
        PC_DispChar(42, 3, ((flags & STATUS) >> 3) + '0', DISP_FGND_WHITE + DISP_BGND_BLACK);
       
        OSFlagPost(GameLogic, STATUS, OS_FLAG_CLR, &err);
        OSTimeDlyHMSM(0, 0, 0, 500);  
    } 
}
/*

*********************************************************************************************************

*                                      NON-TASK FUNCTIONS

*********************************************************************************************************

*/


// Function to check if a player has won
void Check_Winner() {
    INT8U tkill = 0,err,player_char,player;

    flags = OSFlagAccept(GameLogic, PLAYER_1 + PLAYER_2, OS_FLAG_WAIT_SET_ALL, &err);
    if ((flags & PLAYER_1) == PLAYER_1) {
        player_char = PLAYER_1_SIGN;
    }
    else {
        player_char = PLAYER_2_SIGN;
    }
        
    for (int i = 0; i < MAX_BOARD; i++) {
        for (int j = 0; j < MAX_BOARD; j++) {
            player = board[i][j];
            if (player == EMPTY_CHAR) // Skip empty cells
                continue;

            // Check horizontal
            if (j + WIN_COUNT <= MAX_BOARD) {
                int count = 0;
                for (int k = 0; k < WIN_COUNT; k++) {
                    if (board[i][j + k] == player && board[i][j + k] == player_char)
                        count++;
                    else
                        break;
                }
                if (count == WIN_COUNT) {
                    KillTast();
                }
                    
            }

            // Check vertical
            if (i + WIN_COUNT <= MAX_BOARD) {
                int count = 0;
                for (int k = 0; k < WIN_COUNT; k++) {
                    if (board[i + k][j] == player && board[i + k][j] == player_char)
                        count++;
                    else
                        break;
                }
                if (count == WIN_COUNT)
                    KillTast();
            }

            // Check diagonal (top-left to bottom-right)
            if (i + WIN_COUNT <= MAX_BOARD && j + WIN_COUNT <= MAX_BOARD) {
                int count = 0;
                for (int k = 0; k < WIN_COUNT; k++) {
                    if (board[i + k][j + k] == player && board[i + k][j + k] == player_char)
                        count++;
                    else
                        break;
                }
                if (count == WIN_COUNT)
                    KillTast();
            }

            // Check diagonal (top-right to bottom-left)
            if (i + WIN_COUNT <= MAX_BOARD && j - WIN_COUNT + 1 >= 0) {
                int count = 0;
                for (int k = 0; k < WIN_COUNT; k++) {
                    if (board[i + k][j - k] == player && board[i + k][j - k] == player_char)
                        count++;
                    else
                        break;
                }
                if (count == WIN_COUNT)
                    KillTast();
            }
        }
    }
}
void KillTast() {
    INT8U err;
    flags = OSFlagAccept(GameLogic, PLAYER_1 + PLAYER_2, OS_FLAG_WAIT_SET_ALL, &err);
    if ((flags & PLAYER_1) == PLAYER_1)
        PC_DispStr(30, 5, "PLAYER 1 WIN", DISP_FGND_WHITE + DISP_BGND_RED);
    else
        PC_DispStr(30, 5, "PLAYER 2 WIN", DISP_FGND_WHITE + DISP_BGND_GREEN);

    err = OSTaskDel(1);
    err = OSTaskDel(3);
    err = OSTaskDel(4);
    err = OSTaskDel(5);
    
}
void Check_Keyboard()
{
    INT16U key;
    for (;;)
    {
        //Display student ID 
        PC_DispStr(MAX_X / 2, 0, "Thong - e2301482", DISP_FGND_WHITE + DISP_BGND_GREEN);
        //Check stop key 
        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                exit(0);                             /* End program                              */
            }
        }
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}
void UpdateBoard() {
    int i, j;
    INT8U err;
    for (i = 0; i < MAX_BOARD; i++)
        for (j = 0; j < MAX_BOARD; j++)
        {
            if (board[i][j] != 0) {
                if(board[i][j]==PLAYER_1_SIGN)
                    PC_DispChar(i + DISPO, j + DISPO, board[i][j], DISP_FGND_BLACK + DISP_BGND_RED);
                else if(board[i][j] == PLAYER_2_SIGN)
                    PC_DispChar(i + DISPO, j + DISPO, board[i][j], DISP_FGND_BLACK + DISP_BGND_GREEN);
            }
        }
}

void InitialBoard() {
    int i, j;
    INT8U g, h, m = '1', n = '1', err;
    for (g = 2; g <= MAX_BOARD + 1; g++)
        PC_DispChar(g, 0, m++, DISP_FGND_WHITE + DISP_BGND_BLACK);
    for (h = 2; h <= MAX_BOARD + 1; h++)
        PC_DispChar(0, h, n++, DISP_FGND_WHITE + DISP_BGND_BLACK);
    for (i = 0; i < MAX_BOARD; i++)
        for (j = 0; j < MAX_BOARD; j++)
        {
            if (board[i][j] == 0) {
                board[i][j] = EMPTY_CHAR;
                PC_DispChar(i + DISPO, j + DISPO, board[i][j], DISP_FGND_WHITE + DISP_BGND_BLACK);
            }
        }
}

void PlayerMove() {
    int x, y;
    INT8U err;
    flags = OSFlagAccept(GameLogic, PLAYER_1 + PLAYER_2, OS_FLAG_WAIT_SET_ANY, &err);
    do {
        x = rand() % MAX_BOARD;
        y = rand() % MAX_BOARD;
    } while (board[x][y] != EMPTY_CHAR);
    
    if ((flags & PLAYER_1) == PLAYER_1) 
        board[x][y] = PLAYER_1_SIGN;
    else
        board[x][y] = PLAYER_2_SIGN;

        
}