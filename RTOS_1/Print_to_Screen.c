
/********************************************************************/
/*Resource function for MicroC OS -II exercises						*/
/*																	*/
/* The function prints a text string given by *text_ptr to a 		*/
/* fixed position to the screen. A random delay is included 		*/
/* in order to simulate an external device 							*/
/* The function shall be used as a non-reentrant resource			*/
/*******************************************************************/
OS_EVENT* RandomSem;

void Print_to_Screen(INT8U* text_ptr)
{
	INT8U err;
	INT8U delay;
	INT8U symbols[80] = { ' ' };
	delay = -1;

	OSSemPend(RandomSem, 0, &err); /*  Acquire semaphore for random number generator numbers      */

	while (delay <= 1 || delay >= 3)
		delay = rand();
	

	PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);
	OSTimeDlyHMSM(0, 0, delay, 0);
	PC_DispStr(33, 15, text_ptr, DISP_FGND_WHITE);
	OSTimeDlyHMSM(0, 0, delay, 0);
	PC_DispStr(33, 0, symbols, DISP_FGND_GREEN);

	OSSemPost(RandomSem); /*  Release semaphore for random number generator numbers      */



}/*function*/