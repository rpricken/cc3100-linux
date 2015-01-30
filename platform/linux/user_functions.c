#include <stdio.h>
#include <linux/ioctl.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include "cc3100-spi.h"
#include "user_functions.h"

/**************************************************/
/* interface functions for cc3100-spi char driver */
/**************************************************/

int file_desc;
P_EVENT_HANDLER pIraEventHandler = 0;

/* dummy functions */
void stopWDT(){}
void initClk() {}
void CLI_Configure(){}


void CC3100_disable(){
        char dummy;
	//set nHIB low
	if (ioctl(file_desc, CC3100_nHIB_LOW, &dummy) < 0) {
                printf("CC3100_disable() failed\n");
		exit(-1);
        }
}

void CC3100_enable(){
	char dummy;
	//set nHIB high
	if (ioctl(file_desc, CC3100_nHIB_HIGH, &dummy) < 0) {
		printf("CC3100_enable() failed\n");
		exit(-1);
	}
}

Fd_t spi_open(char *ifName, unsigned long flags){
	struct sigaction sig;
	int oflags;

	/* open char device  */
	file_desc = open(DEVICE_FILE_NAME, O_RDWR);
	if (file_desc < 0) {
		printf("Can't open device file: %s\n",DEVICE_FILE_NAME);
		exit(-1);
        }

	/* set signal  */
	sig.sa_sigaction = IRQ_ISR;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGIO, &sig, NULL);

	/* register process in char driver, to pass interrupts */
	fcntl(file_desc, F_SETOWN, getpid());
	oflags = fcntl(file_desc, F_GETFL);
	fcntl(file_desc, F_SETFL, oflags | FASYNC);
	//use of F_SETSIG needs define of _GNU_SOURCE
	fcntl(file_desc, F_SETSIG, SIGIO);

	return 0;
}

int spi_close(Fd_t fd){
	close(file_desc);
	return 0;
}

int spi_write(Fd_t fd, unsigned char *pBuff, int len){
	int err;
	err = write(file_desc, pBuff, len);
	if(err != 0){
		printf("Error write: %i \n", err);
		return -1;
	}
	else{
		return len;
	}
}

int spi_read(Fd_t fd, unsigned char *pBuff, int len){
	int err;
	err = read(file_desc, pBuff, len);
	if(err != 0){
		printf("Error read: %i \n", err);
		return -1;
	}
	else{
		return len;
	}
}

int registerInterruptHandler(P_EVENT_HANDLER InterruptHdl , void* pValue){
	pIraEventHandler = InterruptHdl;
	return 0;
}

void IRQ_ISR(int n, siginfo_t *info, void *unused){
	/* call event handler from SL driver */
	if (pIraEventHandler){
		pIraEventHandler(0);
	}
}


/****************************************/
/* functions for block and sync objects */
/****************************************/

int mutexObjCreate(pthread_mutex_t *mutex, char dummy){
	return pthread_mutex_init(mutex, NULL);
}

int mutexObjDelete(pthread_mutex_t *mutex){
	return pthread_mutex_destroy(mutex);
}

int mutexObjLock(pthread_mutex_t *mutex, char timeout){
	/*timeout NO_WAIT is used after BlockObj is created
	 *intention from SL driver is to set the Block Obj
	 *into a default mode (unblocked),
	 *but there is no need here*/
	if (timeout == NO_WAIT)
		return 0;
	return pthread_mutex_lock (mutex);
}

int mutexObjUnlock(pthread_mutex_t *mutex){
	return pthread_mutex_unlock (mutex);
}

int condObjCreate(condObj *obj, char dummy){
	int retVal = 0;
	retVal += pthread_cond_init (&obj->cond, NULL);
	retVal += pthread_mutex_init(&obj->mutex, NULL);
	return retVal;
}

int condObjDelete(condObj *obj){
	int retVal = 0;
	retVal += pthread_cond_destroy(&obj->cond);
	retVal += pthread_mutex_destroy(&obj->mutex);
	return	retVal;
}

int condObjSignal(condObj *obj){
	int retVal = 0;
	retVal += pthread_mutex_lock (&obj->mutex);
	retVal += pthread_cond_signal(&obj->cond);
	retVal += pthread_mutex_unlock(&obj->mutex);
	return retVal;
}

/* blocks a condObj until condObjSignal is called */
int condObjWait(condObj *obj, char timeout){
	/* same as in mutexObjLock */
	int retVal = 0;
	if (timeout == NO_WAIT)
		return retVal;
	retVal += pthread_mutex_lock (&obj->mutex);
	retVal += pthread_cond_wait(&obj->cond, &obj->mutex);
	retVal += pthread_mutex_unlock(&obj->mutex);
	return retVal;
}
