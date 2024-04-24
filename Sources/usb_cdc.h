#ifndef CDC_ENUMERATE_H
#define CDC_ENUMERATE_H


#define AT91C_EP_OUT 1
#define AT91C_EP_OUT_SIZE 64
#define AT91C_EP_IN  2


typedef struct _AT91S_CDC
{
        // Private members
        AT91PS_UDP pUdp;
        unsigned char currentConfiguration;
        unsigned char currentConnection;
        unsigned int  currentRcvBank;
        // Public Methods:
        unsigned char (*IsConfigured)(struct _AT91S_CDC *pCdc);
        unsigned int  (*Write) (struct _AT91S_CDC *pCdc, const char *pData, unsigned int length);
        unsigned int  (*Read)  (struct _AT91S_CDC *pCdc, char *pData, unsigned int length);
} AT91S_CDC, *AT91PS_CDC;

//* external function description
extern struct _AT91S_CDC        pCDC;
extern void AT91F_USB_Open(void);
AT91PS_CDC AT91F_CDC_Open(AT91PS_CDC pCdc, AT91PS_UDP pUdp);

#endif // CDC_ENUMERATE_H
