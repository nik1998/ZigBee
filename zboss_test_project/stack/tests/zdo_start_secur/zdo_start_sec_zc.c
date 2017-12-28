/**************************************************************************2
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"


/*! \addtogroup ZB_TESTS */
/*! @{ */

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

#define On 1
#define Off 0
#define Toggle 2
#define LevelSet 3
#define LevelDown 5
#define LevelUp 4
  

zb_uint8_t stat=1;
int light=100;
zb_ieee_addr_t g_ieee_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08};
zb_uint8_t g_key[16] = { 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0};


/*
  The test is: ZC starts PAN, ZR joins to it by association and send APS data packet, when ZC
  received packet, it sends packet to ZR, when ZR received packet, it sends
  packet to ZC etc.
 */

#ifndef APS_RETRANSMIT_TEST
//static void zc_send_data(zb_buf_t *buf, zb_uint16_t addr);
//static void zc_send_data(zb_uint8_t param);
#endif
void checkf(zb_uint8_t param);
void data_indication(zb_uint8_t param) ZB_CALLBACK;
//static void send_data(zb_uint8_t param);
MAIN()
{
  ARGV_UNUSED;

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif


  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);
  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  zb_secur_setup_preconfigured_key(g_key, 0);

  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();
  zb_af_set_data_indication(checkf);
  MAIN_RETURN(0);
}



void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    // zb_af_set_data_indication(data_indication);
     zb_af_set_data_indication(checkf);
   // ZB_SCHEDULE_ALARM(zc_send_data,0,3*ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
  }
  zb_free_buf(buf);
}


/*
   Trivial test: dump all APS data received
 */


void data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
#ifndef APS_RETRANSMIT_TEST
 // zb_apsde_data_indication_t *ind = ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);
#endif

  // Remove APS header from the packet 
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS2, "apsde_data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS2, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
    if (ptr[i] != i % 32 + '0')
    {
      TRACE_MSG(TRACE_ERROR, "Bad data %hx %c wants %x %c", (FMT__H_C_D_C, ptr[i], ptr[i],
                         (int)(i % 32 + '0'), (char)i % 32 + '0'));
    }
  }
#ifdef APS_RETRANSMIT_TEST
  zb_free_buf(asdu);
#else
  // send packet back to ZR 
 // zc_send_data(asdu, ind->src_addr);
// zc_send_data();

#endif
}

//#ifndef APS_RETRANSMIT_TEST
/*void send_data(zb_uint8_t param)
{
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  zb_short_t i;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0x0001;
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;

  buf->u.hdr.handle = 0x11;

#if 0
  MAC_PIB().mac_pan_id = 0x1aaa;
  ZB_UPDATE_PAN_ID();		?
#endif

  for (i = 0 ; i < ZB_TEST_DATA_SIZE ; ++i)
  {
     ptr[i]=0;
  }
  if(stat==0)
  {
    ptr[0]=0;
  }
  else
  {
    ptr[0]=1;
  }
  ptr[1]=light;
  TRACE_MSG(TRACE_APS2, "Sending apsde_data.request", (FMT__0));
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}*/
void checkf(zb_uint8_t param)
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr;
  ZB_APS_HDR_CUT_P(asdu, ptr);
  TRACE_MSG(TRACE_APS2, "aaaaaaaaaa", (FMT__0));
  switch(*ptr)
  {
     case On:
     {
        stat=1;
        light=50;
        TRACE_MSG(TRACE_APS2, "turn on", (FMT__0));
        break;
     }
     case Off:
     {
       stat=0;
       light=0;
       TRACE_MSG(TRACE_APS2, "turn off", (FMT__0));
       break;
     }
     case Toggle:
     {
         stat=(stat+1)%2;
         if(stat==1)
         {
           light=50;
         }
         else
         {
           light=0;
         }
         TRACE_MSG(TRACE_APS2,"toggle",(FMT__0));
         break;
     }
     case LevelSet:
     {
        int value =ptr[1];
        light=value;
        TRACE_MSG(TRACE_APS2, "level of light %d", (FMT__D,value));
        break;
     }
     case LevelUp:
     {
        int value =10;
        if(light+value>100)
        {
          light=100;
        }
        else
        light=light+value;
        TRACE_MSG(TRACE_APS2, "level of light %d", (FMT__D,light));
        break;
     }
     case LevelDown:
     {
         int value =10;
         if(light<value)
         {
            light=0;
         }
         else
         light=light-value;
         TRACE_MSG(TRACE_APS2, "level of light %d", (FMT__D,light));
         break;
     }
     default:
         TRACE_MSG(TRACE_APS2,"Unknown command!!!",(FMT__0));
  }
  //send_data(param);
}



//#endif

/*! @} */
