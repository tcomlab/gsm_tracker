#include "main.h"
#include "usb_cdc.h"


typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define AT91C_EP_IN_SIZE 64
const char pxLanguageStringDescriptor[] =
{
        4,
        0x03,
        0x09, 0x04
};

const char pxManufacturerStringDescriptor[] =
{
        14,
        0x03,

        'B', 0x00,
        'E', 0x00,
        'L', 0x00,
        'E', 0x00,
        'M', 0x00,
        'N', 0x00,
};

const char pxProductStringDescriptor[] =
{
        34,
        0x03,

        'S', 0x00,
        'e', 0x00,
        'r', 0x00,
        'i', 0x00,
        'a', 0x00,
        'l', 0x00,
        ' ', 0x00,
        'c', 0x00,
        'o', 0x00,
        'n', 0x00,
        'e', 0x00,
        'c', 0x00,
        't', 0x00,
        'i', 0x00,
        'o', 0x00,
        'n', 0x00,
};

const char pxConfigurationStringDescriptor[] =
{
        38,
        0x03,

        'C', 0x00,
        'o', 0x00,
        'n', 0x00,
        'f', 0x00,
        'i', 0x00,
        'g', 0x00,
        'u', 0x00,
        'r', 0x00,
        'a', 0x00,
        't', 0x00,
        'i', 0x00,
        'o', 0x00,
        'n', 0x00,
        ' ', 0x00,
        'N', 0x00,
        'a', 0x00,
        'm', 0x00,
        'e', 0x00
};

const char pxInterfaceStringDescriptor[] =
{
        30,
        0x03,

        'I', 0x00,
        'n', 0x00,
        't', 0x00,
        'e', 0x00,
        'r', 0x00,
        'f', 0x00,
        'a', 0x00,
        'c', 0x00,
        'e', 0x00,
        ' ', 0x00,
        'N', 0x00,
        'a', 0x00,
        'm', 0x00,
        'e', 0x00
};

const char pxDevDescriptor[] = {
    	/* Device descriptor */
    	0x12,   // bLength
    	0x01,   // bDescriptorType
    	0x10,   // bcdUSBL
    	0x01,   //
    	0x02,   // bDeviceClass:    CDC class code
    	0x00,   // bDeviceSubclass: CDC class sub code
    	0x00,   // bDeviceProtocol: CDC Device protocol
    	0x08,   // bMaxPacketSize0
    	0xEB,   // idVendorL
    	0x03,   //
    	0x01,   // idProductL
    	0x00,   //
    	0x10,   // bcdDeviceL
    	0x01,   //
    	0x00,   // iManufacturer    // 0x01
    	0x00,   // iProduct
    	0x00,   // SerialNumber
    	0x01    // bNumConfigs
};

const char pxCfgDescriptor[] = {
		/* ============== CONFIGURATION 1 =========== */
		/* Configuration 1 descriptor */
		0x09,   // CbLength
		0x02,   // CbDescriptorType
		0x43,   // CwTotalLength 2 EP + Control
		0x00,
		0x02,   // CbNumInterfaces
		0x01,   // CbConfigurationValue
		0x00,   // CiConfiguration
		0xC0,   // CbmAttributes 0xA0
		0x00,   // CMaxPower

		/* Communication Class Interface Descriptor Requirement */
		0x09, // bLength
		0x04, // bDescriptorType
		0x00, // bInterfaceNumber
		0x00, // bAlternateSetting
		0x01, // bNumEndpoints
		0x02, // bInterfaceClass
		0x02, // bInterfaceSubclass
		0x00, // bInterfaceProtocol
		0x00, // iInterface

		/* Header Functional Descriptor */
		0x05, // bFunction Length
		0x24, // bDescriptor type: CS_INTERFACE
		0x00, // bDescriptor subtype: Header Func Desc
		0x10, // bcdCDC:1.1
		0x01,

		/* ACM Functional Descriptor */
		0x04, // bFunctionLength
		0x24, // bDescriptor Type: CS_INTERFACE
		0x02, // bDescriptor Subtype: ACM Func Desc
		0x00, // bmCapabilities

		/* Union Functional Descriptor */
		0x05, // bFunctionLength
		0x24, // bDescriptorType: CS_INTERFACE
		0x06, // bDescriptor Subtype: Union Func Desc
		0x00, // bMasterInterface: Communication Class Interface
		0x01, // bSlaveInterface0: Data Class Interface

		/* Call Management Functional Descriptor */
		0x05, // bFunctionLength
		0x24, // bDescriptor Type: CS_INTERFACE
		0x01, // bDescriptor Subtype: Call Management Func Desc
		0x00, // bmCapabilities: D1 + D0
		0x01, // bDataInterface: Data Class Interface 1

		/* Endpoint 1 descriptor */
		0x07,   // bLength
		0x05,   // bDescriptorType
		0x83,   // bEndpointAddress, Endpoint 03 - IN
		0x03,   // bmAttributes      INT
		0x08,   // wMaxPacketSize
		0x00,
		0xFF,   // bInterval

		/* Data Class Interface Descriptor Requirement */
		0x09, // bLength
		0x04, // bDescriptorType
		0x01, // bInterfaceNumber
		0x00, // bAlternateSetting
		0x02, // bNumEndpoints
		0x0A, // bInterfaceClass
		0x00, // bInterfaceSubclass
		0x00, // bInterfaceProtocol
		0x00, // iInterface

		/* First alternate setting */
		/* Endpoint 1 descriptor */
		0x07,   // bLength
		0x05,   // bDescriptorType
		0x01,   // bEndpointAddress, Endpoint 01 - OUT
		0x02,   // bmAttributes      BULK
		AT91C_EP_OUT_SIZE,   // wMaxPacketSize
		0x00,
		0x00,   // bInterval

		/* Endpoint 2 descriptor */
		0x07,   // bLength
		0x05,   // bDescriptorType
		0x82,   // bEndpointAddress, Endpoint 02 - IN
		0x02,   // bmAttributes      BULK
		AT91C_EP_IN_SIZE,   // wMaxPacketSize
		0x00,
		0x00    // bInterval
};

/* Descriptor type definitions. */
#define usbDESCRIPTOR_TYPE_DEVICE               ( 0x01 )
#define usbDESCRIPTOR_TYPE_CONFIGURATION        ( 0x02 )
#define usbDESCRIPTOR_TYPE_STRING               ( 0x03 )


/* Index to the various string. */
#define usbLANGUAGE_STRING                      ( 0 )
#define usbMANUFACTURER_STRING                  ( 1 )
#define usbPRODUCT_STRING                       ( 2 )
#define usbCONFIGURATION_STRING                 ( 3 )
#define usbINTERFACE_STRING                     ( 4 )




/* USB standard request code */
#define STD_GET_STATUS_ZERO           0x0080
#define STD_GET_STATUS_INTERFACE      0x0081
#define STD_GET_STATUS_ENDPOINT       0x0082

#define STD_CLEAR_FEATURE_ZERO        0x0100
#define STD_CLEAR_FEATURE_INTERFACE   0x0101
#define STD_CLEAR_FEATURE_ENDPOINT    0x0102

#define STD_SET_FEATURE_ZERO          0x0300
#define STD_SET_FEATURE_INTERFACE     0x0301
#define STD_SET_FEATURE_ENDPOINT      0x0302

#define STD_SET_ADDRESS               0x0500
#define STD_GET_DESCRIPTOR            0x0680
#define STD_SET_DESCRIPTOR            0x0700
#define STD_GET_CONFIGURATION         0x0880
#define STD_SET_CONFIGURATION         0x0900
#define STD_GET_INTERFACE             0x0A81
#define STD_SET_INTERFACE             0x0B01
#define STD_SYNCH_FRAME               0x0C82

/* CDC Class Specific Request Code */
#define GET_LINE_CODING               0x21A1
#define SET_LINE_CODING               0x2021
#define SET_CONTROL_LINE_STATE        0x2221


typedef struct {
        unsigned int dwDTERRate;
        char bCharFormat;
        char bParityType;
        char bDataBits;
} AT91S_CDC_LINE_CODING, *AT91PS_CDC_LINE_CODING;

AT91S_CDC_LINE_CODING line = {
        115200, // baudrate
        0,      // 1 Stop Bit
        0,      // None Parity
        8};     // 8 Data bits

uint currentReceiveBank = AT91C_UDP_RX_DATA_BK0;


struct _AT91S_CDC       pCDC;


static uchar AT91F_UDP_IsConfigured(AT91PS_CDC pCdc);
static uint AT91F_UDP_Read(AT91PS_CDC pCdc, char *pData, uint length);
static uint AT91F_UDP_Write(AT91PS_CDC pCdc, const char *pData, uint length);
static void AT91F_CDC_Enumerate(AT91PS_CDC pCdc);


//*----------------------------------------------------------------------------
//* \fn    AT91F_USB_Open
//* \brief This function Open the USB device
//*----------------------------------------------------------------------------
/*
void AT91F_USB_Open(void)
{

  // Set the PLL USB Divider
    AT91C_BASE_CKGR->CKGR_PLLR |= AT91C_CKGR_USBDIV_1 ;
    // Specific Chip USB Initialisation
    // Enables the 48MHz USB clock UDPCK and System Peripheral USB Clock
    AT91F_UDP_CfgPMC();
    // CDC Open by structure initialization
    AT91F_CDC_Open(&pCDC, AT91C_BASE_UDP);


}
*/
void AT91F_USB_Open(void)
{
    // Set the PLL USB Divider
    AT91C_BASE_CKGR->CKGR_PLLR |= AT91C_CKGR_USBDIV_1 ;

    // Specific Chip USB Initialisation
    // Enables the 48MHz USB clock UDPCK and System Peripheral USB Clock
    AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);

    // Enable UDP PullUp (USB_DP_PUP) : enable & Clear of the corresponding PIO
    // Set in PIO mode and Configure in Output
    //AT91F_PIO_CfgOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);
    // Clear for set the Pul up resistor
    //AT91F_PIO_ClearOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);

    // CDC Open by structure initialization
    AT91F_CDC_Open(&pCDC, AT91C_BASE_UDP);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_CDC_Open
//* \brief
//*----------------------------------------------------------------------------
AT91PS_CDC AT91F_CDC_Open(AT91PS_CDC pCdc, AT91PS_UDP pUdp)
{
        pCdc->pUdp = pUdp;
        pCdc->currentConfiguration = 0;
        pCdc->currentConnection    = 0;
        pCdc->currentRcvBank       = AT91C_UDP_RX_DATA_BK0;
        pCdc->IsConfigured = AT91F_UDP_IsConfigured;
        pCdc->Write        = AT91F_UDP_Write;
        pCdc->Read         = AT91F_UDP_Read;
        return pCdc;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_IsConfigured
//* \brief Test if the device is configured and handle enumeration
//*----------------------------------------------------------------------------
static uchar AT91F_UDP_IsConfigured(AT91PS_CDC pCdc)
{
        AT91PS_UDP pUDP = pCdc->pUdp;
        AT91_REG isr = pUDP->UDP_ISR;

        if (isr & AT91C_UDP_ENDBUSRES) {
                pUDP->UDP_ICR = AT91C_UDP_ENDBUSRES;
                // reset all endpoints
                pUDP->UDP_RSTEP  = (unsigned int)-1;
                pUDP->UDP_RSTEP  = 0;
                // Enable the function
                pUDP->UDP_FADDR = AT91C_UDP_FEN;
                // Configure endpoint 0
                pUDP->UDP_CSR[0] = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL);
        }
        else if (isr & AT91C_UDP_EPINT0) {
                pUDP->UDP_ICR = AT91C_UDP_EPINT0;
                AT91F_CDC_Enumerate(pCdc);
        }
        return pCdc->currentConfiguration;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_Read
//* \brief Read available data from Endpoint OUT
//*----------------------------------------------------------------------------
static uint AT91F_UDP_Read(AT91PS_CDC pCdc, char *pData, uint length)
{
        static unsigned int tmp_leigh=0;
        AT91PS_UDP pUdp = pCdc->pUdp;
        uint packetSize, nbBytesRcv = 0, currentReceiveBank = pCdc->currentRcvBank;
       // if (length == 255) return 0;
        while (length) {
                if ( !AT91F_UDP_IsConfigured(pCdc) )
                        break;
                if (tmp_leigh == length) length = 0;
                tmp_leigh = length;
                if ( pUdp->UDP_CSR[AT91C_EP_OUT] & currentReceiveBank ) {
                        packetSize = MIN(pUdp->UDP_CSR[AT91C_EP_OUT] >> 16, length);
                        length -= packetSize;
                        if (packetSize < AT91C_EP_OUT_SIZE)
                                length = 0;
                        while(packetSize--)
                                pData[nbBytesRcv++] = pUdp->UDP_FDR[AT91C_EP_OUT];
                        pUdp->UDP_CSR[AT91C_EP_OUT] &= ~(currentReceiveBank);
                        if (currentReceiveBank == AT91C_UDP_RX_DATA_BK0)
                                currentReceiveBank = AT91C_UDP_RX_DATA_BK1;
                        else
                                currentReceiveBank = AT91C_UDP_RX_DATA_BK0;

                }
                
        }
        pCdc->currentRcvBank = currentReceiveBank;
        return nbBytesRcv;

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_CDC_Write
//* \brief Send through endpoint 2
//*----------------------------------------------------------------------------
static uint AT91F_UDP_Write(AT91PS_CDC pCdc, const char *pData, uint length)
{
        AT91PS_UDP pUdp = pCdc->pUdp;
        uint cpt = 0;

        // Send the first packet
        cpt = MIN(length, AT91C_EP_IN_SIZE);
        length -= cpt;
        while (cpt--) pUdp->UDP_FDR[AT91C_EP_IN] = *pData++;
        pUdp->UDP_CSR[AT91C_EP_IN] |= AT91C_UDP_TXPKTRDY;

        while (length) {
                // Fill the second bank
                cpt = MIN(length, AT91C_EP_IN_SIZE);
                length -= cpt;
                while (cpt--) pUdp->UDP_FDR[AT91C_EP_IN] = *pData++;
                // Wait for the the first bank to be sent
                while ( !(pUdp->UDP_CSR[AT91C_EP_IN] & AT91C_UDP_TXCOMP) )
                        if ( !AT91F_UDP_IsConfigured(pCdc) ) return length;
                pUdp->UDP_CSR[AT91C_EP_IN] &= ~(AT91C_UDP_TXCOMP);
                while (pUdp->UDP_CSR[AT91C_EP_IN] & AT91C_UDP_TXCOMP);
                pUdp->UDP_CSR[AT91C_EP_IN] |= AT91C_UDP_TXPKTRDY;
        }
        // Wait for the end of transfer
        while ( !(pUdp->UDP_CSR[AT91C_EP_IN] & AT91C_UDP_TXCOMP) )
                if ( !AT91F_UDP_IsConfigured(pCdc) ) return length;
        pUdp->UDP_CSR[AT91C_EP_IN] &= ~(AT91C_UDP_TXCOMP);
        while (pUdp->UDP_CSR[AT91C_EP_IN] & AT91C_UDP_TXCOMP);

        return length;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_USB_SendData
//* \brief Send Data through the control endpoint
//*----------------------------------------------------------------------------
unsigned int csrTab[100];
unsigned char csrIdx = 0;

static void AT91F_USB_SendData(AT91PS_UDP pUdp, const char *pData, uint length)
{
        uint cpt = 0;
        AT91_REG csr;

        do {
                cpt = MIN(length, 8);
                length -= cpt;

                while (cpt--)
                        pUdp->UDP_FDR[0] = *pData++;

                if (pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP) {
                        pUdp->UDP_CSR[0] &= ~(AT91C_UDP_TXCOMP);
                        while (pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP);
                }

                pUdp->UDP_CSR[0] |= AT91C_UDP_TXPKTRDY;
                do {
                        csr = pUdp->UDP_CSR[0];

                        // Data IN stage has been stopped by a status OUT
                        if (csr & AT91C_UDP_RX_DATA_BK0) {
                                pUdp->UDP_CSR[0] &= ~(AT91C_UDP_RX_DATA_BK0);
                                return;
                        }
                } while ( !(csr & AT91C_UDP_TXCOMP) );

        } while (length);

        if (pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP) {
                pUdp->UDP_CSR[0] &= ~(AT91C_UDP_TXCOMP);
                while (pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP);
        }
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_USB_SendZlp
//* \brief Send zero length packet through the control endpoint
//*----------------------------------------------------------------------------
void AT91F_USB_SendZlp(AT91PS_UDP pUdp)
{
        pUdp->UDP_CSR[0] |= AT91C_UDP_TXPKTRDY;
        while ( !(pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP) );
        pUdp->UDP_CSR[0] &= ~(AT91C_UDP_TXCOMP);
        while (pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_USB_SendStall
//* \brief Stall the control endpoint
//*----------------------------------------------------------------------------
void AT91F_USB_SendStall(AT91PS_UDP pUdp)
{
        pUdp->UDP_CSR[0] |= AT91C_UDP_FORCESTALL;
        while ( !(pUdp->UDP_CSR[0] & AT91C_UDP_ISOERROR) );
        pUdp->UDP_CSR[0] &= ~(AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR);
        while (pUdp->UDP_CSR[0] & (AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_CDC_Enumerate
//* \brief This function is a callback invoked when a SETUP packet is received
//*----------------------------------------------------------------------------
static void AT91F_CDC_Enumerate(AT91PS_CDC pCdc)
{
        AT91PS_UDP pUDP = pCdc->pUdp;
        uchar bmRequestType, bRequest;
        ushort wValue, wIndex, wLength, wStatus;


        if ( !(pUDP->UDP_CSR[0] & AT91C_UDP_RXSETUP) )
                return;

        bmRequestType = pUDP->UDP_FDR[0];
        bRequest      = pUDP->UDP_FDR[0];
        wValue        = (pUDP->UDP_FDR[0] & 0xFF);
        wValue       |= (pUDP->UDP_FDR[0] << 8);
        wIndex        = (pUDP->UDP_FDR[0] & 0xFF);
        wIndex       |= (pUDP->UDP_FDR[0] << 8);
        wLength       = (pUDP->UDP_FDR[0] & 0xFF);
        wLength      |= (pUDP->UDP_FDR[0] << 8);

        if (bmRequestType & 0x80) {
                pUDP->UDP_CSR[0] |= AT91C_UDP_DIR;
                while ( !(pUDP->UDP_CSR[0] & AT91C_UDP_DIR) );
        }
        pUDP->UDP_CSR[0] &= ~AT91C_UDP_RXSETUP;
        while ( (pUDP->UDP_CSR[0]  & AT91C_UDP_RXSETUP)  );

        // Handle supported standard device request Cf Table 9-3 in USB specification Rev 1.1
        switch ((bRequest << 8) | bmRequestType) {
        case STD_GET_DESCRIPTOR:

                switch(wValue>>8)
                {
                  case usbDESCRIPTOR_TYPE_DEVICE:        // Return Device Descriptor
                            AT91F_USB_SendData(pUDP, pxDevDescriptor, MIN(sizeof(pxDevDescriptor), wLength));
                    break;
                  case usbDESCRIPTOR_TYPE_CONFIGURATION:        // Return Device Descriptor
                            AT91F_USB_SendData(pUDP, pxCfgDescriptor, MIN(sizeof(pxCfgDescriptor), wLength));
                    break;

                 case usbDESCRIPTOR_TYPE_STRING:        // Return Strings
                        switch(wValue&0xFF)
                        {
                           case usbLANGUAGE_STRING:        // Return Strings
                            AT91F_USB_SendData(pUDP, pxLanguageStringDescriptor, MIN(sizeof(pxLanguageStringDescriptor), wLength));
                          break;
                          case usbMANUFACTURER_STRING:        // Return Strings
                            AT91F_USB_SendData(pUDP, pxManufacturerStringDescriptor, MIN(sizeof(pxManufacturerStringDescriptor), wLength));
                          break;
                          case usbPRODUCT_STRING:        // Return Strings
                            AT91F_USB_SendData(pUDP, pxProductStringDescriptor, MIN(sizeof(pxProductStringDescriptor), wLength));
                          break;
                          case usbCONFIGURATION_STRING:        // Return Strings
                            AT91F_USB_SendData(pUDP, pxConfigurationStringDescriptor, MIN(sizeof(pxConfigurationStringDescriptor), wLength));
                          break;
                          case usbINTERFACE_STRING:        // Return Strings
                            AT91F_USB_SendData(pUDP, pxInterfaceStringDescriptor, MIN(sizeof(pxInterfaceStringDescriptor), wLength));
                          break;

                        default:AT91F_USB_SendStall(pUDP);
                        }

                  default:AT91F_USB_SendStall(pUDP);
                }
                break;
        case STD_SET_ADDRESS:
                AT91F_USB_SendZlp(pUDP);
                pUDP->UDP_FADDR = (AT91C_UDP_FEN | wValue);
                pUDP->UDP_GLBSTATE  = (wValue) ? AT91C_UDP_FADDEN : 0;
                break;
        case STD_SET_CONFIGURATION:
                pCdc->currentConfiguration = wValue;
                AT91F_USB_SendZlp(pUDP);
                pUDP->UDP_GLBSTATE  = (wValue) ? AT91C_UDP_CONFG : AT91C_UDP_FADDEN;
                pUDP->UDP_CSR[1] = (wValue) ? (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT) : 0;
                pUDP->UDP_CSR[2] = (wValue) ? (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN)  : 0;
                pUDP->UDP_CSR[3] = (wValue) ? (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_ISO_IN)   : 0;
                break;
        case STD_GET_CONFIGURATION:
                AT91F_USB_SendData(pUDP, (char *) &(pCdc->currentConfiguration), sizeof(pCdc->currentConfiguration));
                break;
        case STD_GET_STATUS_ZERO:
                wStatus = 0;
                AT91F_USB_SendData(pUDP, (char *) &wStatus, sizeof(wStatus));
                break;
        case STD_GET_STATUS_INTERFACE:
                wStatus = 0;
                AT91F_USB_SendData(pUDP, (char *) &wStatus, sizeof(wStatus));
                break;
        case STD_GET_STATUS_ENDPOINT:
                wStatus = 0;
                wIndex &= 0x0F;
                if ((pUDP->UDP_GLBSTATE & AT91C_UDP_CONFG) && (wIndex <= 3)) {
                        wStatus = (pUDP->UDP_CSR[wIndex] & AT91C_UDP_EPEDS) ? 0 : 1;
                        AT91F_USB_SendData(pUDP, (char *) &wStatus, sizeof(wStatus));
                }
                else if ((pUDP->UDP_GLBSTATE & AT91C_UDP_FADDEN) && (wIndex == 0)) {
                        wStatus = (pUDP->UDP_CSR[wIndex] & AT91C_UDP_EPEDS) ? 0 : 1;
                        AT91F_USB_SendData(pUDP, (char *) &wStatus, sizeof(wStatus));
                }
                else
                        AT91F_USB_SendStall(pUDP);
                break;
        case STD_SET_FEATURE_ZERO:
                AT91F_USB_SendStall(pUDP);
            break;
        case STD_SET_FEATURE_INTERFACE:
                AT91F_USB_SendZlp(pUDP);
                break;
        case STD_SET_FEATURE_ENDPOINT:
                wIndex &= 0x0F;
                if ((wValue == 0) && wIndex && (wIndex <= 3)) {
                        pUDP->UDP_CSR[wIndex] = 0;
                        AT91F_USB_SendZlp(pUDP);
                }
                else
                        AT91F_USB_SendStall(pUDP);
                break;
        case STD_CLEAR_FEATURE_ZERO:
                AT91F_USB_SendStall(pUDP);
            break;
        case STD_CLEAR_FEATURE_INTERFACE:
                AT91F_USB_SendZlp(pUDP);
                break;
        case STD_CLEAR_FEATURE_ENDPOINT:
                wIndex &= 0x0F;
                if ((wValue == 0) && wIndex && (wIndex <= 3)) {
                        if (wIndex == 1)
                                pUDP->UDP_CSR[1] = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT);
                        else if (wIndex == 2)
                                pUDP->UDP_CSR[2] = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN);
                        else if (wIndex == 3)
                                pUDP->UDP_CSR[3] = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_ISO_IN);
                        AT91F_USB_SendZlp(pUDP);
                }
                else
                        AT91F_USB_SendStall(pUDP);
                break;

        // handle CDC class requests
        case SET_LINE_CODING:
                while ( !(pUDP->UDP_CSR[0] & AT91C_UDP_RX_DATA_BK0) );
                pUDP->UDP_CSR[0] &= ~(AT91C_UDP_RX_DATA_BK0);
                AT91F_USB_SendZlp(pUDP);
                break;
        case GET_LINE_CODING:
                AT91F_USB_SendData(pUDP, (char *) &line, MIN(sizeof(line), wLength));
                break;
        case SET_CONTROL_LINE_STATE:
                pCdc->currentConnection = wValue;
                AT91F_USB_SendZlp(pUDP);
                break;
        default:
                AT91F_USB_SendStall(pUDP);
            break;
        }
}
