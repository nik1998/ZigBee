/***************************************************************************
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
PURPOSE:
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

#ifndef ZB_ROUTER_ROLE
#error Router role is not compiled!
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif
#define dToggle 2
#define dStepUp 3
#define dChangeColor 1

/*! \addtogroup ZB_TESTS */
/*! @{ */

//static void send_data(zb_buf_t *buf);
#ifndef APS_RETRANSMIT_TEST
void data_indication(zb_uint8_t param) ZB_CALLBACK;
#endif
static void zc_send_data(zb_uint8_t param);
/*
  ZR joins to ZC, then sends APS packet.
 */



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
  ZB_INIT("zdo_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr", "2", "2");
#endif
  //ZB_PIB_RX_ON_WHEN_IDLE()=ZB_FALSE;//sleep

  /* FIXME: temporary, until neighbor table is not in nvram */
  /* add extended address of potential parent to emulate that we've already
   * been connected to it */
  {
    zb_ieee_addr_t ieee_address;
    zb_address_ieee_ref_t ref;

    ZB_64BIT_ADDR_ZERO(ieee_address);
    ieee_address[7] = 8;

    zb_address_update(ieee_address, 0, ZB_FALSE, &ref);
  }

  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

 // zb_af_set_data_indication(checkf);
 // MACPIB.macRxOnWhenIdle=false;
  TRACE_DEINIT();
  MAIN_RETURN(0);
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
#ifndef APS_RETRANSMIT_TEST
   // zb_af_set_data_indication(data_indication);
    ZB_SCHEDULE_CALLBACK(zc_send_data, ZB_REF_FROM_BUF(buf));
 //  zb_af_set_data_indication(checkf);
#endif
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}


zb_uint8_t inn=0;
void sent_data (zb_uint8_t param)
{
    zb_buf_t *buf =(zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_apsde_data_req_t *req;
   // zb_uint8_t *ptr = NULL;
   // zb_short_t i;
    zb_uint16_t addr=0x0000;
   // ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
    req->dst_addr.addr_short = addr; 
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;
    buf->u.hdr.handle = 0x11;
   /* for (i = 1 ; i < ZB_TEST_DATA_SIZE ; ++i)
    {
      ptr[i] =0 ;
    }
    ptr[0]=stat;
    ptr[1]=val;*/
   // TRACE_MSG(TRACE_APS1, "Sending apsde_data.request", (FMT__0)); 
   // ZB_SCHEDULE_ALARM(zb_apsde_data_request, ZB_REF_FROM_BUF(buf),5*ZB_TIME_ONE_SECOND);
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));

}
void  Toggle (zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
   zb_buf_t *buf = ZB_BUF_FROM_REF(param);
   ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
   ptr[0]=dToggle;
   ptr[1]=0;
   sent_data(param);
}
void  StepUp (zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
   zb_buf_t *buf = ZB_BUF_FROM_REF(param);
   ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
   ptr[1]=0;
   ptr[0]= dStepUp ;
   sent_data(param);
}
void  ChangeColor (zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
   zb_buf_t *buf = ZB_BUF_FROM_REF(param);
   ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
   ptr[1]=0;
   ptr[0]= dChangeColor ;
   sent_data(param);
}

//static void zc_send_data(zb_buf_t *buf, zb_uint16_t addr)
static void zc_send_data(zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
    zb_short_t i;
    zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
    param=ZB_REF_FROM_BUF(buf);
    ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
    for (i = 1 ; i < ZB_TEST_DATA_SIZE ; ++i)
    {
      ptr[i] =0 ;
    }

   // zb_free_buf(buf);
   /* zb_apsde_data_req_t *req;
    zb_uint8_t *ptr = NULL;
    zb_short_t i;
    ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
    req->dst_addr.addr_short = addr;*/ /* send to ZR */
   /* req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;  
    buf->u.hdr.handle = 0x11;  
    for (i = 1 ; i < ZB_TEST_DATA_SIZE ; ++i)
    {
      ptr[i] =0 ;
    }
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request%hd", (FMT__H,inn));  */
       switch(inn%3+1)
       {
         case dToggle:
         {
 
           Toggle(param);
           break;
         }
         case dStepUp:
         {
           StepUp(param);
           break;
         }
         case dChangeColor:
         {
           ChangeColor(param);
           break;
         }
         default:break;
        }
        inn++;
       TRACE_MSG(TRACE_APS1, "Recall fuction", (FMT__0)); 

       ZB_SCHEDULE_ALARM(zc_send_data,0,5*ZB_TIME_ONE_SECOND);
      // ZB_SCHEDULE_CALLBACK(zc_send_data, ZB_REF_FROM_BUF(buf));

      /*TRACE_MSG(TRACE_APS1, "Sending apsde_data.request", (FMT__0)); 
      // zb_apsde_data_request(1);
       ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));*/
}

