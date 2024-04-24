/*
 * gsm.h
 *
 *  Created on: 17.02.2010
 *      Author: Admin
 */

#ifndef GSM_H_
#define GSM_H_

// TCP status
#define IP_INITIAL         0  // �����������, ��������� ���������
#define IP_START           1  // ����������� ��������� ���������� GPRS( APN � �.�. )/CSD
#define IP_CONFIG          2  // ������� ���������
#define IP_IND		       3  // ������������ ����������
#define IP_GPRSACT         4  // PDP �������� �����������/CSD PPP ����������
#define IP_STATUS          5  // ������� IP �����
#define TCP_UDP_CONNECTING 6  // ��������������� ����������� ������ TCP/����������� UDP
#define IP_CLOSE           7  // ���������� TCP ���������
#define CONNECT_OK         8  // ����������� ����������
#define PDP_DEACT_PDP      9  // �������� �������������
//#define RXBUF_SIZE 512

void config_usart_gsm(void);
//void SendAT(char * byte,unsigned char no_wait);
int get_imei(char * IMEI);
int get_csq( char *CSQ);
int tcp_status(void);
int send_data_server(unsigned char *buf, int size);
int connect_to_server(char * Server, unsigned int Port);
int ReadDataBuf(char *comparestring);
int ReadServerBuf(char *comparestring);
int enable_gprs(void);
int disable_gprs(void);
void flush_rx_buffer(void);
void flush_rx_server_buffer(void);
unsigned char gsm_networt_avalible(void);
void ReqCSD(char * byte);
int gsm_modem_on(void);
unsigned char SendATWResp (char *cmd,char *resp);
void SetAPN (char *APN);
#endif /* GSM_H_ */
