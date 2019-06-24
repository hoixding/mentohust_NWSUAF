/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mystate.c
* 摘	要：改变认证状态
* 作	者：HustMoon@BYHH
* 邮	箱：www.ehust@gmail.com
*/


/*
 * modified by shilianggoo@gmail.com 2014
 * add NEED_LOGOUT, fix permission problems
 * in some secure setuid shebang implemention
 */

#include "mystate.h"
#include "i18n.h"
#include "myfunc.h"
#include "checkV4.h"
#include "dlfunc.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define MAX_SEND_COUNT		3	/* 最大超时次数 */

volatile int state = ID_DISCONNECT;	/* 认证状态 */
const u_char *capBuf = NULL;	/* 抓到的包 */
static u_char sendPacket[0x3E8];	/* 用来发送的包 */
static int sendCount = 0;	/* 同一阶段发包计数 */

extern char nic[];
extern const u_char STANDARD_ADDR[];
extern char userName[];
extern unsigned startMode;
extern unsigned dhcpMode;
extern u_char localMAC[], destMAC[];
extern unsigned timeout;
extern unsigned echoInterval;
extern unsigned restartWait;
extern char dhcpScript[];
extern pcap_t *hPcap;
extern u_char *fillBuf;
extern unsigned fillSize;
extern u_int32_t pingHost;
extern u_int32_t ip, mask, gateway, dns, pingHost;
#ifndef NO_ARP
extern u_int32_t rip, gateway;
extern u_char gateMAC[];
static void sendArpPacket();	/* ARP监视 */
#endif

static const u_char pad[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static const unsigned char pkt1[532] = {
/*0x01, 0xd0, 0xf8, 0x00, 0x00, 0x03, 0x30, 0x3a, 
0x64, 0xc7, 0x4f, 0xc0, 0x88, 0x8e, 0x01, 0x01, 
0x00, 0x00,*/ /*0xff, 0xff, 0x37, 0x77, 0x7f, 0x24, 
0xd8, 0xe0, 0x5d, 0x00, 0x00, 0x00, 0xff, 0x24, 
0xd8, 0xe0, 0x7f, 0xca, 0x77, 0xef, 0xaf, 0x02, 
0xb7, 0x00, 0x00, 0x13, 0x11, 0x38, 0x30, 0x32, 
0x31, 0x78, 0x2e, 0x65, 0x78, 0x65, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 
0x00, 0x01, 0x00, 0x00, 0x13, 0x11, 0x01, 0xe3, 
0x1a, 0x28, 0x00, 0x00, 0x13, 0x11, 0x17, 0x22, 
0x34, 0x36, 0x33, 0x32, 0x31, 0x41, 0x33, 0x33, 
0x42, 0x35, 0x43, 0x34, 0x44, 0x37, 0x34, 0x31, 
0x45, 0x35, 0x30, 0x43, 0x43, 0x43, 0x43, 0x43, 
0x43, 0x43, 0x43, 0x33, 0x38, 0x38, 0x34, 0x39, 
0x1a, 0x0c, 0x00, 0x00, 0x13, 0x11, 0x18, 0x06, 
0x00, 0x00, 0x00, 0x00, 0x1a, 0x0e, 0x00, 0x00, 
0x13, 0x11, 0x2d, 0x08, 0xf8, 0xa9, 0x63, 0x32, 
0xe5, 0x6e, 0x1a, 0x18, 0x00, 0x00, 0x13, 0x11, 
0x2f, 0x12, 0x48, 0xce, 0xbe, 0xb1, 0xb0, 0xe7, 
0x35, 0x02, 0x71, 0x2e, 0x81, 0xe4, 0xab, 0xf4, 
0x5c, 0x06, 0x1a, 0x09, 0x00, 0x00, 0x13, 0x11, 
0x35, 0x03, 0x05, 0x1a, 0x18, 0x00, 0x00, 0x13, 
0x11, 0x36, 0x12, 0xfe, 0x80, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x5a, 0x69, 0x6c, 0xff, 0xfe, 
0x35, 0x69, 0x37, 0x1a, 0x18, 0x00, 0x00, 0x13, 
0x11, 0x38, 0x12, 0xfe, 0x80, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x44, 0x4f, 0x56, 0x30, 0x95, 
0xb5, 0xaf, 0x90, 0x1a, 0x18, 0x00, 0x00, 0x13, 
0x11, 0x4e, 0x12, 0x20, 0x01, 0x02, 0x50, 0x68, 
0x01, 0x11, 0x08, 0x44, 0x4f, 0x56, 0x30, 0x95, 
0xb5, 0xaf, 0x90, 0x1a, 0x88, 0x00, 0x00, 0x13, 
0x11, 0x4d, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x1a, 0x28, 0x00, 0x00, 0x13, 
0x11, 0x39, 0x22, 0xbd, 0xcc, 0xd3, 0xfd, 0xcd, 
0xf8, 0xbd, 0xd3, 0xc8, 0xeb, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x1a, 0x48, 0x00, 0x00, 0x13, 
0x11, 0x54, 0x42, 0x32, 0x30, 0x31, 0x35, 0x30, 
0x32, 0x30, 0x39, 0x34, 0x30, 0x31, 0x35, 0x31, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x1a, 0x09, 0x00, 0x00, 0x13, 
0x11, 0x62, 0x03, 0x00, 0x1a, 0x09, 0x00, 0x00, 
0x13, 0x11, 0x6b, 0x03, 0x00, 0x1a, 0x09, 0x00, 
0x00, 0x13, 0x11, 0x70, 0x03, 0x40, 0x1a, 0x09, 
0x00, 0x00, 0x13, 0x11, 0x6f, 0x03, 0x00, 0x1a, 
0x09, 0x00, 0x00, 0x13, 0x11, 0x79, 0x03, 0x02, 
0x1a, 0x13, 0x00, 0x00, 0x13, 0x11, 0x76, 0x0d, 
0x31, 0x37, 0x32, 0x2e, 0x31, 0x37, 0x2e, 0x38, 
0x2e, 0x31, 0x33*/
/*0x01,0xd0,0xf8,0x00,0x00,0x03,0x00,0xe0,0x4c,0x36,0x00,0x39,0x88,0x8e,0x01,0x01,
0x00,0x00,*/0xff,0xff,0x37,0x77,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xb4,0x27,0xf5,0xbf,0xee,0x51,0x00,0x00,0x13,0x11,0x38,0x30,0x32,
0x31,0x78,0x2e,0x65,0x78,0x65,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1f,0x01,
0x02,0x00,0x00,0x00,0x13,0x11,0x01,0xce,0x1a,0x0c,0x00,0x00,0x13,0x11,0x18,0x06,
0x00,0x00,0x00,0x01,0x1a,0x0e,0x00,0x00,0x13,0x11,0x2d,0x08,0x00,0xe0,0x4c,0x36,
0x00,0x39,0x1a,0x08,0x00,0x00,0x13,0x11,0x2f,0x02,0x1a,0x22,0x00,0x00,0x13,0x11,
0x76,0x1c,0x31,0x31,0x33,0x2e,0x32,0x30,0x30,0x2e,0x36,0x37,0x2e,0x32,0x30,0x30,
0x3b,0x36,0x31,0x2e,0x31,0x35,0x30,0x2e,0x34,0x37,0x2e,0x31,0x1a,0x09,0x00,0x00,
0x13,0x11,0x35,0x03,0x02,0x1a,0x18,0x00,0x00,0x13,0x11,0x36,0x12,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x18,0x00,
0x00,0x13,0x11,0x38,0x12,0xfe,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x2b,0x40,
0x62,0xa2,0x3c,0x31,0x1d,0x1a,0x18,0x00,0x00,0x13,0x11,0x4e,0x12,0x20,0x01,0x02,
0x50,0x10,0x02,0x24,0x65,0x32,0xbc,0x35,0xc5,0x8e,0x2d,0xc5,0xd6,0x1a,0x88,0x00,
0x00,0x13,0x11,0x4d,0x82,0x63,0x65,0x64,0x66,0x65,0x62,0x31,0x64,0x37,0x66,0x37,
0x31,0x34,0x35,0x34,0x36,0x65,0x35,0x37,0x63,0x61,0x32,0x33,0x62,0x35,0x36,0x65,
0x31,0x38,0x38,0x38,0x62,0x31,0x63,0x35,0x37,0x36,0x37,0x38,0x30,0x62,0x36,0x38,
0x64,0x61,0x32,0x37,0x36,0x34,0x62,0x32,0x64,0x61,0x39,0x62,0x61,0x64,0x66,0x36,
0x38,0x66,0x65,0x37,0x39,0x31,0x33,0x30,0x37,0x34,0x30,0x39,0x37,0x35,0x39,0x36,
0x35,0x32,0x33,0x32,0x63,0x35,0x61,0x35,0x65,0x33,0x36,0x62,0x30,0x30,0x63,0x30,
0x39,0x34,0x30,0x31,0x64,0x30,0x66,0x64,0x38,0x31,0x31,0x37,0x39,0x61,0x33,0x31,
0x38,0x62,0x36,0x36,0x35,0x38,0x37,0x66,0x66,0x39,0x65,0x34,0x39,0x61,0x61,0x39,
0x38,0x66,0x35,0x64,0x30,0x1a,0x28,0x00,0x00,0x13,0x11,0x39,0x22,0x69,0x6e,0x74,
0x65,0x72,0x6e,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x48,0x00,
0x00,0x13,0x11,0x54,0x42,0x53,0x32,0x30,0x47,0x4e,0x59,0x41,0x47,0x31,0x30,0x30,
0x30,0x37,0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1a,0x08,0x00,0x00,0x13,0x11,0x55,0x02,0x1a,0x09,0x00,
0x00,0x13,0x11,0x62,0x03,0x00,0x1a,0x09,0x00,0x00,0x13,0x11,0x70,0x03,0x40,0x1a,
0x1e,0x00,0x00,0x13,0x11,0x6f,0x18,0x52,0x47,0x2d,0x53,0x55,0x20,0x46,0x6f,0x72,
0x20,0x4c,0x69,0x6e,0x75,0x78,0x20,0x56,0x31,0x2e,0x33,0x30,0x00,0x1a,0x09,0x00,
0x00,0x13,0x11,0x79,0x03,0x02
};

static const unsigned char pkt2[532] = {
/*0x00, 0x1a, 0xa9, 0xc4, 0xdb, 0xb1, 0x30, 0x3a, 
0x64, 0xc7, 0x4f, 0xc0, 0x88, 0x8e, 0x01, 0x00, 
0x00, 0x11, 0x02, 0x01, 0x00, 0x11, 0x01, 0x32, 
0x30, 0x31, 0x34, 0x32, 0x31, 0x31, 0x32, 0x31, 
0x30, 0x35, 0x39,*//* 0xff, 0xff, 0x37, 0x77, 0x7f, 
0x24, 0xd8, 0xe0, 0x5d, 0x00, 0x00, 0x00, 0xff, 
0x24, 0xd8, 0xe0, 0x7f, 0xca, 0x77, 0xef, 0xaf, 
0x02, 0xb7, 0x00, 0x00, 0x13, 0x11, 0x38, 0x30, 
0x32, 0x31, 0x78, 0x2e, 0x65, 0x78, 0x65, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 
0x03, 0x00, 0x01, 0x00, 0x00, 0x13, 0x11, 0x01, 
0xe3, 0x1a, 0x28, 0x00, 0x00, 0x13, 0x11, 0x17, 
0x22, 0x34, 0x36, 0x33, 0x32, 0x31, 0x41, 0x33, 
0x33, 0x42, 0x35, 0x43, 0x34, 0x44, 0x37, 0x34, 
0x31, 0x45, 0x35, 0x30, 0x43, 0x43, 0x43, 0x43, 
0x43, 0x43, 0x43, 0x43, 0x33, 0x38, 0x38, 0x34, 
0x39, 0x1a, 0x0c, 0x00, 0x00, 0x13, 0x11, 0x18, 
0x06, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x0e, 0x00, 
0x00, 0x13, 0x11, 0x2d, 0x08, 0xf8, 0xa9, 0x63, 
0x32, 0xe5, 0x6e, 0x1a, 0x18, 0x00, 0x00, 0x13, 
0x11, 0x2f, 0x12, 0x48, 0xce, 0xbe, 0xb1, 0xb0, 
0xe7, 0x35, 0x02, 0x71, 0x2e, 0x81, 0xe4, 0xab, 
0xf4, 0x5c, 0x06, 0x1a, 0x09, 0x00, 0x00, 0x13, 
0x11, 0x35, 0x03, 0x05, 0x1a, 0x18, 0x00, 0x00, 
0x13, 0x11, 0x36, 0x12, 0xfe, 0x80, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x5a, 0x69, 0x6c, 0xff, 
0xfe, 0x35, 0x69, 0x37, 0x1a, 0x18, 0x00, 0x00, 
0x13, 0x11, 0x38, 0x12, 0xfe, 0x80, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x44, 0x4f, 0x56, 0x30, 
0x95, 0xb5, 0xaf, 0x90, 0x1a, 0x18, 0x00, 0x00, 
0x13, 0x11, 0x4e, 0x12, 0x20, 0x01, 0x02, 0x50, 
0x68, 0x01, 0x11, 0x08, 0x44, 0x4f, 0x56, 0x30, 
0x95, 0xb5, 0xaf, 0x90, 0x1a, 0x88, 0x00, 0x00, 
0x13, 0x11, 0x4d, 0x82, 0x32, 0x36, 0x39, 0x37, 
0x65, 0x38, 0x30, 0x62, 0x37, 0x62, 0x64, 0x36, 
0x66, 0x31, 0x39, 0x34, 0x33, 0x62, 0x61, 0x39, 
0x38, 0x62, 0x32, 0x30, 0x37, 0x62, 0x63, 0x64, 
0x65, 0x37, 0x65, 0x36, 0x64, 0x62, 0x36, 0x33, 
0x33, 0x62, 0x36, 0x66, 0x64, 0x32, 0x38, 0x63, 
0x31, 0x61, 0x30, 0x32, 0x38, 0x32, 0x31, 0x39, 
0x61, 0x62, 0x31, 0x33, 0x39, 0x65, 0x64, 0x39, 
0x61, 0x33, 0x62, 0x39, 0x38, 0x34, 0x66, 0x37, 
0x34, 0x64, 0x36, 0x39, 0x38, 0x62, 0x39, 0x61, 
0x35, 0x66, 0x66, 0x33, 0x64, 0x39, 0x33, 0x36, 
0x32, 0x34, 0x33, 0x37, 0x38, 0x33, 0x33, 0x61, 
0x39, 0x38, 0x35, 0x38, 0x37, 0x64, 0x63, 0x33, 
0x39, 0x63, 0x61, 0x62, 0x63, 0x33, 0x39, 0x62, 
0x63, 0x61, 0x37, 0x34, 0x37, 0x32, 0x32, 0x62, 
0x66, 0x35, 0x66, 0x36, 0x65, 0x34, 0x66, 0x37, 
0x37, 0x33, 0x35, 0x31, 0x1a, 0x28, 0x00, 0x00, 
0x13, 0x11, 0x39, 0x22, 0xbd, 0xcc, 0xd3, 0xfd, 
0xcd, 0xf8, 0xbd, 0xd3, 0xc8, 0xeb, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x1a, 0x48, 0x00, 0x00, 
0x13, 0x11, 0x54, 0x42, 0x32, 0x30, 0x31, 0x35, 
0x30, 0x32, 0x30, 0x39, 0x34, 0x30, 0x31, 0x35, 
0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x1a, 0x09, 0x00, 0x00, 
0x13, 0x11, 0x62, 0x03, 0x00, 0x1a, 0x09, 0x00, 
0x00, 0x13, 0x11, 0x6b, 0x03, 0x00, 0x1a, 0x09, 
0x00, 0x00, 0x13, 0x11, 0x70, 0x03, 0x40, 0x1a, 
0x09, 0x00, 0x00, 0x13, 0x11, 0x6f, 0x03, 0x00, 
0x1a, 0x09, 0x00, 0x00, 0x13, 0x11, 0x79, 0x03, 
0x02, 0x1a, 0x13, 0x00, 0x00, 0x13, 0x11, 0x76, 
0x0d, 0x31, 0x37, 0x32, 0x2e, 0x31, 0x37, 0x2e, 
0x38, 0x2e, 0x31, 0x33*/
/*0x00,0x1a,0xa9,0x0f,0x27,0xfa,0x00,0xe0,0x4c,0x36,0x00,0x39,0x88,0x8e,0x01,0x00,
0x00,0x0f,0x02,0x01,0x00,0x0f,0x01,0x32,0x30,0x31,0x35,0x30,0x31,0x32,0x38,0x30,
0x30,*/0xff,0xff,0x37,0x77,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xb4,0x27,0xf5,0xbf,0xee,0x51,0x00,0x00,0x13,0x11,0x38,0x30,0x32,0x31,
0x78,0x2e,0x65,0x78,0x65,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1f,0x01,0x02,
0xd0,0x00,0x00,0x13,0x11,0x01,0xce,0x1a,0x0c,0x00,0x00,0x13,0x11,0x18,0x06,0x00,
0x00,0x00,0x01,0x1a,0x0e,0x00,0x00,0x13,0x11,0x2d,0x08,0x00,0xe0,0x4c,0x36,0x00,
0x39,0x1a,0x08,0x00,0x00,0x13,0x11,0x2f,0x02,0x1a,0x22,0x00,0x00,0x13,0x11,0x76,
0x1c,0x31,0x31,0x33,0x2e,0x32,0x30,0x30,0x2e,0x36,0x37,0x2e,0x32,0x30,0x30,0x3b,
0x36,0x31,0x2e,0x31,0x35,0x30,0x2e,0x34,0x37,0x2e,0x31,0x1a,0x09,0x00,0x00,0x13,
0x11,0x35,0x03,0x02,0x1a,0x18,0x00,0x00,0x13,0x11,0x36,0x12,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x18,0x00,0x00,
0x13,0x11,0x38,0x12,0xfe,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x2b,0x40,0x62,
0xa2,0x3c,0x31,0x1d,0x1a,0x18,0x00,0x00,0x13,0x11,0x4e,0x12,0x20,0x01,0x02,0x50,
0x10,0x02,0x24,0x65,0x32,0xbc,0x35,0xc5,0x8e,0x2d,0xc5,0xd6,0x1a,0x88,0x00,0x00,
0x13,0x11,0x4d,0x82,0x63,0x65,0x64,0x66,0x65,0x62,0x31,0x64,0x37,0x66,0x37,0x31,
0x34,0x35,0x34,0x36,0x65,0x35,0x37,0x63,0x61,0x32,0x33,0x62,0x35,0x36,0x65,0x31,
0x38,0x38,0x38,0x62,0x31,0x63,0x35,0x37,0x36,0x37,0x38,0x30,0x62,0x36,0x38,0x64,
0x61,0x32,0x37,0x36,0x34,0x62,0x32,0x64,0x61,0x39,0x62,0x61,0x64,0x66,0x36,0x38,
0x66,0x65,0x37,0x39,0x31,0x33,0x30,0x37,0x34,0x30,0x39,0x37,0x35,0x39,0x36,0x35,
0x32,0x33,0x32,0x63,0x35,0x61,0x35,0x65,0x33,0x36,0x62,0x30,0x30,0x63,0x30,0x39,
0x34,0x30,0x31,0x64,0x30,0x66,0x64,0x38,0x31,0x31,0x37,0x39,0x61,0x33,0x31,0x38,
0x62,0x36,0x36,0x35,0x38,0x37,0x66,0x66,0x39,0x65,0x34,0x39,0x61,0x61,0x39,0x38,
0x66,0x35,0x64,0x30,0x1a,0x28,0x00,0x00,0x13,0x11,0x39,0x22,0x69,0x6e,0x74,0x65,
0x72,0x6e,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x48,0x00,0x00,
0x13,0x11,0x54,0x42,0x53,0x32,0x30,0x47,0x4e,0x59,0x41,0x47,0x31,0x30,0x30,0x30,
0x37,0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1a,0x08,0x00,0x00,0x13,0x11,0x55,0x02,0x1a,0x09,0x00,0x00,
0x13,0x11,0x62,0x03,0x00,0x1a,0x09,0x00,0x00,0x13,0x11,0x70,0x03,0x40,0x1a,0x1e,
0x00,0x00,0x13,0x11,0x6f,0x18,0x52,0x47,0x2d,0x53,0x55,0x20,0x46,0x6f,0x72,0x20,
0x4c,0x69,0x6e,0x75,0x78,0x20,0x56,0x31,0x2e,0x33,0x30,0x00,0x1a,0x09,0x00,0x00,
0x13,0x11,0x79,0x03,0x02
};

static const unsigned char pkt3[548] = {
/*0x00, 0x1a, 0xa9, 0xc4, 0xdb, 0xb1, 0x30, 0x3a, 
0x64, 0xc7, 0x4f, 0xc0, 0x88, 0x8e, 0x01, 0x00, 
0x00, 0x22, 0x02, 0x02, 0x00, 0x22, 0x04, 0x10, 
0x35, 0xc7, 0xed, 0xff, 0x3c, 0x9d, 0xfd, 0x4c, 
0x7f, 0x23, 0x2e, 0xec, 0xd7, 0x2f, 0x21, 0x73, 
0x32, 0x30, 0x31, 0x34, 0x32, 0x31, 0x31, 0x32, 
0x31, 0x30, 0x35, 0x39,*/ /*0xff, 0xff, 0x37, 0x77, 
0x7f, 0x24, 0xd8, 0xe0, 0x5d, 0x00, 0x00, 0x00, 
0xff, 0x24, 0xd8, 0xe0, 0x7f, 0xca, 0x77, 0xef, 
0xaf, 0x02, 0xb7, 0x00, 0x00, 0x13, 0x11, 0x38, 
0x30, 0x32, 0x31, 0x78, 0x2e, 0x65, 0x78, 0x65, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x13, 0x11, 
0x01, 0xe3, 0x1a, 0x28, 0x00, 0x00, 0x13, 0x11, 
0x17, 0x22, 0x63, 0x32, 0x61, 0x36, 0x36, 0x36, 
0x35, 0x39, 0x32, 0x36, 0x64, 0x34, 0x35, 0x66, 
0x34, 0x61, 0x36, 0x64, 0x33, 0x34, 0x39, 0x63, 
0x37, 0x33, 0x38, 0x65, 0x61, 0x33, 0x34, 0x30, 
0x66, 0x35, 0x1a, 0x0c, 0x00, 0x00, 0x13, 0x11, 
0x18, 0x06, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x0e, 
0x00, 0x00, 0x13, 0x11, 0x2d, 0x08, 0xf8, 0xa9, 
0x63, 0x32, 0xe5, 0x6e, 0x1a, 0x18, 0x00, 0x00, 
0x13, 0x11, 0x2f, 0x12, 0xcc, 0x61, 0xea, 0x9f, 
0x3e, 0x1d, 0xfc, 0x81, 0x1a, 0xa0, 0x14, 0x01, 
0xbc, 0x6f, 0xbf, 0xb0, 0x1a, 0x09, 0x00, 0x00, 
0x13, 0x11, 0x35, 0x03, 0x05, 0x1a, 0x18, 0x00, 
0x00, 0x13, 0x11, 0x36, 0x12, 0xfe, 0x80, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0x69, 0x6c, 
0xff, 0xfe, 0x35, 0x69, 0x37, 0x1a, 0x18, 0x00, 
0x00, 0x13, 0x11, 0x38, 0x12, 0xfe, 0x80, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x4f, 0x56, 
0x30, 0x95, 0xb5, 0xaf, 0x90, 0x1a, 0x18, 0x00, 
0x00, 0x13, 0x11, 0x4e, 0x12, 0x20, 0x01, 0x02, 
0x50, 0x68, 0x01, 0x11, 0x08, 0x44, 0x4f, 0x56, 
0x30, 0x95, 0xb5, 0xaf, 0x90, 0x1a, 0x88, 0x00, 
0x00, 0x13, 0x11, 0x4d, 0x82, 0x31, 0x38, 0x63, 
0x32, 0x32, 0x65, 0x39, 0x39, 0x63, 0x34, 0x30, 
0x66, 0x34, 0x38, 0x39, 0x39, 0x39, 0x30, 0x62, 
0x61, 0x66, 0x35, 0x34, 0x61, 0x31, 0x63, 0x38, 
0x35, 0x61, 0x36, 0x66, 0x61, 0x63, 0x31, 0x64, 
0x31, 0x38, 0x32, 0x39, 0x38, 0x30, 0x34, 0x66, 
0x36, 0x66, 0x32, 0x34, 0x39, 0x63, 0x61, 0x65, 
0x32, 0x65, 0x63, 0x66, 0x65, 0x36, 0x36, 0x35, 
0x39, 0x39, 0x35, 0x31, 0x64, 0x30, 0x65, 0x34, 
0x64, 0x61, 0x31, 0x30, 0x32, 0x38, 0x39, 0x36, 
0x66, 0x63, 0x31, 0x36, 0x66, 0x35, 0x66, 0x30, 
0x39, 0x66, 0x35, 0x39, 0x32, 0x37, 0x61, 0x65, 
0x31, 0x33, 0x65, 0x39, 0x32, 0x30, 0x32, 0x39, 
0x36, 0x39, 0x61, 0x38, 0x31, 0x37, 0x36, 0x36, 
0x35, 0x39, 0x66, 0x30, 0x33, 0x61, 0x32, 0x66, 
0x64, 0x35, 0x30, 0x35, 0x32, 0x63, 0x65, 0x35, 
0x61, 0x32, 0x62, 0x36, 0x65, 0x1a, 0x28, 0x00, 
0x00, 0x13, 0x11, 0x39, 0x22, 0xbd, 0xcc, 0xd3, 
0xfd, 0xcd, 0xf8, 0xbd, 0xd3, 0xc8, 0xeb, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x48, 0x00, 
0x00, 0x13, 0x11, 0x54, 0x42, 0x32, 0x30, 0x31, 
0x35, 0x30, 0x32, 0x30, 0x39, 0x34, 0x30, 0x31, 
0x35, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x09, 0x00, 
0x00, 0x13, 0x11, 0x62, 0x03, 0x00, 0x1a, 0x09, 
0x00, 0x00, 0x13, 0x11, 0x6b, 0x03, 0x00, 0x1a, 
0x09, 0x00, 0x00, 0x13, 0x11, 0x70, 0x03, 0x40, 
0x1a, 0x09, 0x00, 0x00, 0x13, 0x11, 0x6f, 0x03, 
0x00, 0x1a, 0x09, 0x00, 0x00, 0x13, 0x11, 0x79, 
0x03, 0x02, 0x1a, 0x13, 0x00, 0x00, 0x13, 0x11, 
0x76, 0x0d, 0x31, 0x37, 0x32, 0x2e, 0x31, 0x37, 
0x2e, 0x38, 0x2e, 0x31, 0x33*/
/*0x00,0x1a,0xa9,0x0f,0x27,0xfa,0x00,0xe0,0x4c,0x36,0x00,0x39,0x88,0x8e,0x01,0x00,
0x00,0x20,0x02,0x02,0x00,0x20,0x04,0x10,0x90,0xc8,0xc3,0x7c,0x09,0x82,0xcd,0xe1,
0xca,0xc6,0xec,0x2a,0xe9,0x85,0x75,0x36,0x32,0x30,0x31,0x35,0x30,0x31,0x32,0x38,
0x30,0x30,*/0xff,0xff,0x37,0x77,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xb4,0x27,0xf5,0xbf,0xee,0x51,0x00,0x00,0x13,0x11,0x38,0x30,0x32,
0x31,0x78,0x2e,0x65,0x78,0x65,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1f,0x01,
0x02,0x20,0x00,0x00,0x13,0x11,0x01,0xde,0x1a,0x0c,0x00,0x00,0x13,0x11,0x18,0x06,
0x00,0x00,0x00,0x01,0x1a,0x0e,0x00,0x00,0x13,0x11,0x2d,0x08,0x00,0xe0,0x4c,0x36,
0x00,0x39,0x1a,0x18,0x00,0x00,0x13,0x11,0x2f,0x12,0x4f,0x6c,0xec,0x22,0x27,0x28,
0x8d,0x76,0x8b,0x6a,0xe9,0xf4,0x49,0x0f,0x65,0x76,0x1a,0x22,0x00,0x00,0x13,0x11,
0x76,0x1c,0x31,0x31,0x33,0x2e,0x32,0x30,0x30,0x2e,0x36,0x37,0x2e,0x32,0x30,0x30,
0x3b,0x36,0x31,0x2e,0x31,0x35,0x30,0x2e,0x34,0x37,0x2e,0x31,0x1a,0x09,0x00,0x00,
0x13,0x11,0x35,0x03,0x02,0x1a,0x18,0x00,0x00,0x13,0x11,0x36,0x12,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x18,0x00,
0x00,0x13,0x11,0x38,0x12,0xfe,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x2b,0x40,
0x62,0xa2,0x3c,0x31,0x1d,0x1a,0x18,0x00,0x00,0x13,0x11,0x4e,0x12,0x20,0x01,0x02,
0x50,0x10,0x02,0x24,0x65,0x32,0xbc,0x35,0xc5,0x8e,0x2d,0xc5,0xd6,0x1a,0x88,0x00,
0x00,0x13,0x11,0x4d,0x82,0x37,0x33,0x30,0x64,0x37,0x31,0x64,0x33,0x63,0x37,0x65,
0x34,0x65,0x37,0x34,0x33,0x65,0x38,0x39,0x66,0x32,0x64,0x30,0x31,0x32,0x35,0x36,
0x32,0x64,0x32,0x37,0x63,0x36,0x65,0x37,0x35,0x31,0x64,0x62,0x39,0x64,0x38,0x64,
0x63,0x39,0x61,0x30,0x32,0x36,0x66,0x34,0x31,0x66,0x65,0x36,0x30,0x32,0x62,0x63,
0x33,0x34,0x33,0x62,0x35,0x30,0x61,0x62,0x39,0x35,0x63,0x36,0x65,0x34,0x65,0x39,
0x33,0x39,0x63,0x35,0x65,0x64,0x33,0x65,0x30,0x65,0x31,0x36,0x37,0x61,0x32,0x33,
0x64,0x37,0x31,0x37,0x65,0x34,0x66,0x33,0x64,0x39,0x30,0x64,0x39,0x63,0x65,0x31,
0x34,0x37,0x31,0x64,0x37,0x36,0x34,0x30,0x31,0x32,0x61,0x36,0x31,0x35,0x34,0x61,
0x30,0x65,0x37,0x65,0x39,0x1a,0x28,0x00,0x00,0x13,0x11,0x39,0x22,0x69,0x6e,0x74,
0x65,0x72,0x6e,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x48,0x00,
0x00,0x13,0x11,0x54,0x42,0x53,0x32,0x30,0x47,0x4e,0x59,0x41,0x47,0x31,0x30,0x30,
0x30,0x37,0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1a,0x08,0x00,0x00,0x13,0x11,0x55,0x02,0x1a,0x09,0x00,
0x00,0x13,0x11,0x62,0x03,0x00,0x1a,0x09,0x00,0x00,0x13,0x11,0x70,0x03,0x40,0x1a,
0x1e,0x00,0x00,0x13,0x11,0x6f,0x18,0x52,0x47,0x2d,0x53,0x55,0x20,0x46,0x6f,0x72,
0x20,0x4c,0x69,0x6e,0x75,0x78,0x20,0x56,0x31,0x2e,0x33,0x30,0x00,0x1a,0x09,0x00,
0x00,0x13,0x11,0x79,0x03,0x02
};

static unsigned char* pkt_start;
static unsigned char* pkt_identity;
static unsigned char* pkt_md5;

static void setTimer(unsigned interval);	/* 设置定时器 */
static int renewIP();	/* 更新IP */
static void fillEtherAddr(u_int32_t protocol);  /* 填充MAC地址和协议 */
static int sendStartPacket();	 /* 发送Start包 */
static int sendIdentityPacket();	/* 发送Identity包 */
static int sendChallengePacket();   /* 发送Md5 Challenge包 */
static int sendEchoPacket();	/* 发送心跳包 */
static int sendLogoffPacket();  /* 发送退出包 */
static int waitEchoPacket();	/* 等候响应包 */

static void setTimer(unsigned interval) /* 设置定时器 */
{
	struct itimerval timer;
	timer.it_value.tv_sec = interval;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = interval;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
}

void customizeServiceName(char* service)
{
		pkt_start = pkt1;
		pkt_identity = pkt2;
		pkt_md5 = pkt3;
}

int switchState(int type)
{

	if (state == type) /* 跟上次是同一状态？ */
		sendCount++;
	else
	{
		state = type;
		sendCount = 0;
	}
	if (sendCount>=MAX_SEND_COUNT && type!=ID_ECHO)  /* 超时太多次？ */
	{
		switch (type)
		{
		case ID_START:
			printf(_(">> 找不到服务器，重启认证!\n"));
			break;
		case ID_IDENTITY:
			printf(_(">> 发送用户名超时，重启认证!\n"));
			break;
		case ID_CHALLENGE:
			printf(_(">> 发送密码超时，重启认证!\n"));
			break;
		case ID_WAITECHO:
			printf(_(">> 等候响应包超时，自行响应!\n"));
			return switchState(ID_ECHO);
		}
		return restart();
	}
	switch (type)
	{
	case ID_DHCP:
		return renewIP();
	case ID_START:
		return sendStartPacket();
	case ID_IDENTITY:
		return sendIdentityPacket();
	case ID_CHALLENGE:
		return sendChallengePacket();
	case ID_WAITECHO:	/* 塞尔的就不ping了，不好计时 */
		return waitEchoPacket();
	case ID_ECHO:
		if (pingHost && sendCount*echoInterval > 60) {	/* 1分钟左右 */
			if (isOnline() == -1) {
				printf(_(">> 认证掉线，开始重连!\n"));
				return switchState(ID_START);
			}
			sendCount = 1;
		}
#ifndef NO_ARP
		if (gateMAC[0] != 0xFE)
			sendArpPacket();
#endif
		return sendEchoPacket();
	case ID_DISCONNECT:
		return sendLogoffPacket();
	}
	return 0;
}

int restart()
{
	if (startMode >= 3)	/* 标记服务器地址为未获取 */
		startMode -= 3;
	state = ID_START;
	sendCount = -1;
	setTimer(restartWait);	/* restartWait秒后或者服务器请求后重启认证 */
	return 0;
}

static int renewIP()
{
	setTimer(0);	/* 取消定时器 */
	printf(">> 正在获取IP...\n");
    setreuid(0,0);                              // new code
    printf("%s\n", dhcpScript);					// new code
	system(dhcpScript);
    printf(">> 操作结束。\n");
	dhcpMode += 3; /* 标记为已获取，123变为456，5不需再认证*/
	if (fillHeader() == -1)
		exit(EXIT_FAILURE);
	if (dhcpMode == 5)
		return switchState(ID_ECHO);
	return switchState(ID_START);
}

static void fillEtherAddr(u_int32_t protocol)
{
	memset(sendPacket, 0, 0x3E8);
	memcpy(sendPacket, destMAC, 6);
	memcpy(sendPacket+0x06, localMAC, 6);
	*(u_int32_t *)(sendPacket+0x0C) = htonl(protocol);
}

static int sendStartPacket()
{
	printf(">> 发送Start包...\n");
	if (startMode%3 == 2)	/* 赛尔 */
	{
		if (sendCount == 0)
		{
			printf(_(">> 寻找服务器...(赛尔)\n"));
			memcpy(sendPacket, STANDARD_ADDR, 6);
			memcpy(sendPacket+0x06, localMAC, 6);
			*(u_int32_t *)(sendPacket+0x0C) = htonl(0x888E0101);
			*(u_int16_t *)(sendPacket+0x10) = 0;
			memset(sendPacket+0x12, 0xa5, 42);
			setTimer(timeout);
		}
		return pcap_sendpacket(hPcap, sendPacket, 60);
	}
	if (sendCount == 0)
	{
		printf(_(">> 寻找服务器...\n"));
		//fillStartPacket();             // new code
		fillEtherAddr(0x888E0101);
		memcpy(sendPacket + 0x12, pkt_start, sizeof(pkt1));      // new code
	//	memcpy(sendPacket + 0xe2, computeV4(pad, 16), 0x80);     // new code
	//	memcpy(sendPacket + 0x100, computeV4(pad, 16), 0x80); 

		memcpy(sendPacket+0x17, encodeIP(ip), 0x04);	//fill IP
		memcpy(sendPacket+0x1b, encodeIP(mask), 0x04);	//fill mask
		memcpy(sendPacket+0x1F, encodeIP(gateway), 0x04); //fill gateway

		setTimer(timeout);
	}
	return pcap_sendpacket(hPcap, sendPacket, 549);
}

static int sendIdentityPacket()
{
	printf(">> 发送Identity包...\n");
	int nameLen = strlen(userName);
	if (startMode%3 == 2)	/* 赛尔 */
	{
		if (sendCount == 0)
		{
			printf(_(">> 发送用户名...(赛尔)\n"));
			*(u_int16_t *)(sendPacket+0x0E) = htons(0x0100);
			*(u_int16_t *)(sendPacket+0x10) = *(u_int16_t *)(sendPacket+0x14) = htons(nameLen+30);
			sendPacket[0x12] = 0x02;
			sendPacket[0x16] = 0x01;
			sendPacket[0x17] = 0x01;
			fillCernetAddr(sendPacket);
			memcpy(sendPacket+0x28, "03.02.05", 8);
			memcpy(sendPacket+0x30, userName, nameLen);
			setTimer(timeout);
		}
		sendPacket[0x13] = capBuf[0x13];
		return pcap_sendpacket(hPcap, sendPacket, nameLen+48);
	}
	if (sendCount == 0)
	{
		printf(_(">> 发送用户名...\n"));
		fillEtherAddr(0x888E0100);
		nameLen = strlen(userName);
		*(u_int16_t *)(sendPacket+0x14) = *(u_int16_t *)(sendPacket+0x10) = htons(nameLen+5);
		sendPacket[0x12] = 0x02;
		sendPacket[0x13] = capBuf[0x13];
		sendPacket[0x16] = 0x01;
		memcpy(sendPacket+0x17, userName, nameLen);                         // new code
		memcpy(sendPacket+0x17+nameLen, pkt_identity, sizeof(pkt2));       // new code
		//memcpy(sendPacket + 0xe7 + nameLen, computeV4(pad, 16), 0x80);     // new code

		/* 不知道为啥，在Identity包里插入 Ip地址等信息，服务器收到的ip地址为0.0.0.0
		memcpy(sendPacket+0x28, encodeIP(ip), 0x04);	//fill IP
		memcpy(sendPacket+0x2c, encodeIP(mask), 0x04);	//fill mask
		memcpy(sendPacket+0x30, encodeIP(gateway), 0x04); //fill gateway
		*/

		memcpy(sendPacket + 0xFA + nameLen, computeV4(pad, 16), 0x80);
		setTimer(timeout);
	}
	return pcap_sendpacket(hPcap, sendPacket, 564);
}

static int sendChallengePacket()
{
	int i;
	printf(">> 发送Challenge包...\n");
	int nameLen = strlen(userName);
	if (startMode%3 == 2)	/* 赛尔 */
	{
		if (sendCount == 0)
		{
			printf(_(">> 发送密码...(赛尔)\n"));
			*(u_int16_t *)(sendPacket+0x0E) = htons(0x0100);
			*(u_int16_t *)(sendPacket+0x10) = *(u_int16_t *)(sendPacket+0x14) = htons(nameLen+22);
			sendPacket[0x12] = 0x02;
			sendPacket[0x13] = capBuf[0x13];
			sendPacket[0x16] = 0x04;
			sendPacket[0x17] = 16;

			memcpy(sendPacket+0x18, checkPass(capBuf[0x13], capBuf+0x18, capBuf[0x17]), 16);
			memcpy(sendPacket+0x28, userName, nameLen);

			setTimer(timeout);
		}
		return pcap_sendpacket(hPcap, sendPacket, nameLen+40);
	}
	if (sendCount == 0)
	{
		printf(_(">> 发送密码...\n"));
		//fillMd5Packet(capBuf+0x18);       // new code
		fillEtherAddr(0x888E0100);
		*(u_int16_t *)(sendPacket+0x14) = *(u_int16_t *)(sendPacket+0x10) = htons(nameLen+22);
		sendPacket[0x12] = 0x02;
		sendPacket[0x13] = capBuf[0x13];
		sendPacket[0x16] = 0x04;
		sendPacket[0x17] = 16;


		memcpy(sendPacket+0x18, checkPass(capBuf[0x13], capBuf+0x18, capBuf[0x17]), 16);
		memcpy(sendPacket+0x28, userName, nameLen);

		memcpy(sendPacket+0x28+nameLen, pkt_md5, sizeof(pkt3));         // new code
		
		memcpy(sendPacket+0x39, encodeIP(ip), 0x04);	//fill IP
		memcpy(sendPacket+0x3d, encodeIP(mask), 0x04);	//fill mask
		memcpy(sendPacket+0x41, encodeIP(gateway), 0x04); //fill gateway

	//	memcpy(sendPacket + 0x90 + nameLen, computePwd(capBuf+0x18), 0x10);        // new code
	    memcpy(sendPacket + 0x90 + nameLen, computePwd(capBuf+0x18), 0x10);
		//memcpy(sendPacket + 0xa0 +nameLen, fillBuf + 0x68, fillSize-0x68);      // new code
		//memcpy(sendPacket + 0x108 + nameLen, computeV4(capBuf+0x18, capBuf[0x17]), 0x80);       // new code
		printf("** MD5 seeds:\t");
		for(i=0; i<capBuf[0x17]; ++i){
			printf("%02x", capBuf[0x18 + i]);
		}
		printf("\n");
		memcpy(sendPacket + 0x11b + nameLen, computeV4(capBuf+0x18, capBuf[0x17]), 0x80);
		//sendPacket[0x77] = 0xc7;          
		setTimer(timeout);
	}
	return pcap_sendpacket(hPcap, sendPacket, 597);
}

static int sendEchoPacket()
{
	if (startMode%3 == 2)	/* 赛尔 */
	{
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0106);
		*(u_int16_t *)(sendPacket+0x10) = 0;
		memset(sendPacket+0x12, 0xa5, 42);
		switchState(ID_WAITECHO);	/* 继续等待 */
		return pcap_sendpacket(hPcap, sendPacket, 60);
	}
	if (sendCount == 0)
	{
		u_char echo[] =
		{
			0x00,0x1E,0xFF,0xFF,0x37,0x77,0x7F,0x9F,0xFF,0xFF,0xD9,0x13,0xFF,0xFF,0x37,0x77,
			0x7F,0x9F,0xFF,0xFF,0xF7,0x2B,0xFF,0xFF,0x37,0x77,0x7F,0x3F,0xFF
		};
		printf(_(">> 发送心跳包以保持在线...\n"));
		fillEtherAddr(0x888E01BF);
		memcpy(sendPacket+0x10, echo, sizeof(echo));
		setTimer(echoInterval);
	}
	fillEchoPacket(sendPacket);
	return pcap_sendpacket(hPcap, sendPacket, 0x2D);
}

static int sendLogoffPacket()
{
	setTimer(0);	/*  */
	if (startMode%3 == 2)	/* 赛尔 */
	{
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0102);
		*(u_int16_t *)(sendPacket+0x10) = 0;
		memset(sendPacket+0x12, 0xa5, 42);
		return pcap_sendpacket(hPcap, sendPacket, 60);
	}
#ifndef NO_LOGOUT         // new code
	fillStartPacket();	/* 锐捷的退出包与Start包类似，不过其实不这样也是没问题的 */
	fillEtherAddr(0x888E0102);
	memcpy(sendPacket+0x12, fillBuf, fillSize);
	return pcap_sendpacket(hPcap, sendPacket, 0x3E8);
#else           // new code
	return 0;     // new code
#endif            // new code
}

static int waitEchoPacket()
{
	if (sendCount == 0)
		setTimer(echoInterval);
	return 0;
}

#ifndef NO_ARP
static void sendArpPacket()
{
	u_char arpPacket[0x3C] = {
		0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x06,0x00,0x01,
		0x08,0x00,0x06,0x04,0x00};

	if (gateMAC[0] != 0xFF) {
		memcpy(arpPacket, gateMAC, 6);
		memcpy(arpPacket+0x06, localMAC, 6);
		arpPacket[0x15]=0x02;
		memcpy(arpPacket+0x16, localMAC, 6);
		memcpy(arpPacket+0x1c, &rip, 4);
		memcpy(arpPacket+0x20, gateMAC, 6);
		memcpy(arpPacket+0x26, &gateway, 4);
		pcap_sendpacket(hPcap, arpPacket, 0x3C);
	}
	memset(arpPacket, 0xFF, 6);
	memcpy(arpPacket+0x06, localMAC, 6);
	arpPacket[0x15]=0x01;
	memcpy(arpPacket+0x16, localMAC, 6);
	memcpy(arpPacket+0x1c, &rip, 4);
	memset(arpPacket+0x20, 0, 6);
	memcpy(arpPacket+0x26, &gateway, 4);
	pcap_sendpacket(hPcap, arpPacket, 0x2A);
}
#endif
