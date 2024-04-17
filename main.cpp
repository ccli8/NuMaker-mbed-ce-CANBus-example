#include "cmsis_os.h"
#include "mbed.h"
#include "CAN.h"


DigitalOut led1(LED1, 1); //LED R OFF
DigitalOut led2(LED2, 1); //LED G OFF
DigitalOut led3(LED3, 1); //LED B OFF

#define CAN_TX_MODE_TEST            0
#define CAN_RX_MODE_TEST            1
/* WARNING: Don't enable interrupt mode on receive side. It is not supported on Nuvoton targets. */
#define CAN_RX_IRQ_EN               0
#define LED_ALL_OFF                 led1=led2=led3=1

#define MSG_NUM_INDEX           5   // 0 ~ 31   
#define CAN_DEV_ID              0x1AC

#if defined(TARGET_NUMAKER_PFM_NUC472)
CAN canObj(PA_0, PA_1);     // Internal in the board
#elif defined(TARGET_NUMAKER_PFM_M453)
CAN canObj(PA_13, PA_12);   // Internal in the board
#elif defined(TARGET_NUMAKER_PFM_M487)
CAN canObj(D9, D8);         // (rd, td) Change to match external attachment
#elif defined(TARGET_NUMAKER_IOT_M487) 
CAN canObj(A0, A1);         // Change to match external attachment
#elif defined(TARGET_NUMAKER_IOT_M467) 
CAN canObj(A0, A1);         // CAN1(rd, td) Change to match external attachment
//CAN canObj(D9, D8);       // CAN0
#endif

CANMessage canMsg;

osThreadId mainThreadID;

static int read_MsgObj()
{
int i=0;

    i = canObj.read(canMsg, MSG_NUM_INDEX);
    
        switch (canMsg.data[0])
        {
            case 0:
                LED_ALL_OFF;
                led1=0;
                break;
            
            case 1:
                LED_ALL_OFF;
                led2=0;
                break;
            
            case 2:
                LED_ALL_OFF;
                led3=0;
                break;
            
            default:
                LED_ALL_OFF;
                break;
        }

   return i; 
}

void irq_callback(void)
{
    /* Wake up receive task */
    osSignalSet(mainThreadID, 0x06);
}

int main() {
#if CAN_TX_MODE_TEST
    printf("CAN sender sample\r\n");
#endif

#if CAN_RX_MODE_TEST
    printf("CAN receiver sample\r\n");
#endif

    int i=0;
    char data[8]={0};

    mainThreadID = osThreadGetId();

    /* Set Frequency 1khz~1000khz */
    canObj.frequency(1000000);
    
#if CAN_RX_MODE_TEST
    
#if CAN_RX_IRQ_EN
    /* Attach irq function */
    canObj.attach(irq_callback, CAN::RxIrq);
#endif
    
    canObj.filter(CAN_DEV_ID, 0, CANStandard, MSG_NUM_INDEX);
    
#endif
    
    while (true)
    {

        
#if CAN_TX_MODE_TEST
        
        canObj.write(CANMessage(CAN_DEV_ID, data));
        
        if(data[0] == 2)
            data[0]=0;
        else
            data[0]++;

        memset(&data[1], data[0], 7);
        ThisThread::sleep_for(1000ms);

#endif
        
#if CAN_RX_MODE_TEST

#if (CAN_RX_IRQ_EN)
        /* Wait for receive task to wakeup */
        osSignalWait(0x06, osWaitForever);      
#endif
        if (!read_MsgObj()) {
            continue;
        }

        printf("Read ID=%8X, Type=%s, DLC=%d,Data=",canMsg.id,canMsg.format?"EXT":"STD",canMsg.len);
        for(i=0; i<canMsg.len; i++)
            printf("%02X,",canMsg.data[i]);
        printf("\r\n");     
#endif
    }
    
}
