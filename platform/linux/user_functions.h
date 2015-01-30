#include <signal.h>
#include <pthread.h>
#include <stdint.h>

#define WAIT_FOREVER 	0xff
#define NO_WAIT 	0x00
#define RETURN_CODE_OK 	(0)

typedef void (*P_EVENT_HANDLER)(void* pValue);
typedef unsigned int Fd_t;


/* function pthread_cond_wait in condObjWait needs a mutex */
typedef struct{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
}condObj;



/* function prototyps */
void stopWDT();
void initClk();
void CLI_Configure();

void CC3100_disable();
void CC3100_enable();
void IRQ_ISR(int n, siginfo_t *info, void *unused);
Fd_t spi_open(char *ifName, unsigned long flags);
int spi_close(Fd_t fd);
int spi_write(Fd_t fd, unsigned char *pBuff, int len);
int spi_read(Fd_t fd, unsigned char *pBuff, int len);
int registerInterruptHandler(P_EVENT_HANDLER InterruptHdl , void* pValue);

int mutexObjCreate(pthread_mutex_t *mutex, char dummy);
int mutexObjDelete(pthread_mutex_t *mutex);
int mutexObjLock(pthread_mutex_t *mutex, char timeout);
int mutexObjUnlock(pthread_mutex_t *mutex);
int condObjCreate(condObj *obj, char dummy);
int condObjDelete(condObj *obj);
int condObjSignal(condObj *obj);
int condObjWait(condObj *obj, char timeout);
