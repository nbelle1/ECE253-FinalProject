/*****************************************************************************
* main.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  November 1,2020
*****************************************************************************/

#include "qpn_port.h"                                       /* QP-nano port */
#include "bsp.h"                             /* Board Support Package (BSP) */
#include "lab2a.h"                               /* application interface */
#include "xil_cache.h"		                /* Cache Drivers */


static QEvent l_lab2aQueue[30];  

QActiveCB const Q_ROM Q_ROM_VAR QF_active[] = {
	{ (QActive *)0,            (QEvent *)0,          0                    },
	{ (QActive *)&AO_Lab2A,    l_lab2aQueue,         Q_DIM(l_lab2aQueue)  }
};

Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);

// Do not edit main, unless you have a really good reason
//int main(void) {
//
//	Xil_ICacheInvalidate();
//	Xil_ICacheEnable();
//	Xil_DCacheInvalidate();
//	Xil_DCacheEnable();
//
//	Lab2A_ctor(); // inside of lab2a.c
//	BSP_init(); // inside of bsp.c, starts out empty!
//	QF_run(); // inside of qfn.c
//	return 0;
//}

void printDebugLog() {
	int size = Q_DIM(QF_active);
	xil_printf("Number of HSMs: %i\r\n", size);
	for (int i = 0; i < size; i++) {
		xil_printf("\r\n");
		if (QF_active[i].act == 0 || QF_active[i].act->prio != i) {
			xil_printf("HSM %i: not initialized.\r\n", i);
			continue;
		}
		const QActiveCB *block = &(QF_active[i]);
		QActive *act = block->act;
		xil_printf("HSM %i: initialized\r\n", i);
		xil_printf("Queue: %i/%i\r\n", act->nUsed, block->end);
		int ind = act->tail;
		for (int j = 0; j < act->nUsed; j++) {
			QSignal sig = block->queue[ind].sig;
			xil_printf("\tEvent %i: %i\r\n", j, sig);
			ind++;
			if (ind >= block->end)
				ind -= block->end;
		}
	}
}
