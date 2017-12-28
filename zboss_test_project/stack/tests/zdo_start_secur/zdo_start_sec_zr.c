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


/*! \addtogroup ZB_TESTS */
/*! @{ */

static void zc_send_data(zb_uint8_t param);
#ifndef APS_RETRANSMIT_TEST
void data_indication(zb_uint8_t param) ZB_CALLBACK;
#endif

/*
  ZR joins to ZC, then sends APS packet.
 */
void checkf(zb_uint8_t param);
void zc_respond(zb_uint8_t param);

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
  ///
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  ///
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

  zb_af_set_data_indication(zc_respond);
 // MACPIB.macRxOnWhenIdle=false;
  TRACE_DEINIT();
  MAIN_RETURN(0);
}
static void send_data(zb_buf_t *buf)
{
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  //zb_short_t i;

  ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0x0000; 
  //req->nwk_addr=0x0000;
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 1;
  req->dst_endpoint = 11;
  req->clusterid=0x4000;
  buf->u.hdr.handle = 0x11;
  MAC_PIB().mac_pan_id = 0x1aaa;
  ptr[0]=1;
  ptr[0]=2;
  ptr[0]=3;
  ptr[0]=4;
  ZB_UPDATE_PAN_ID();
  TRACE_MSG(TRACE_APS2, "Sending apsde_data.request", (FMT__0));
  //ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
  ZB_SCHEDULE_ALARM(zb_apsde_data_request,ZB_REF_FROM_BUF(buf),6*ZB_TIME_ONE_SECOND);
}

void zc_respond(zb_uint8_t param)
{
   TRACE_MSG(TRACE_APS2, "answer", (FMT__0));
   zb_ushort_t i;
   zb_uint8_t *ptr;
   zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  //Remove APS header from the packet
   ZB_APS_HDR_CUT_P(asdu, ptr);

   TRACE_MSG(TRACE_APS2, "answer packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));
   for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
   {
    TRACE_MSG(TRACE_APS2, "ianswer %x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
   }

}
void checkf(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APS2, "checkf", (FMT__H));
    zc_send_data(param);
    zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
    param=ZB_REF_FROM_BUF(buf);
    send_data(ZB_BUF_FROM_REF(param));
}
void zb_get_sh(zb_uint8_t param)
{
   zb_buf_t *buf =ZB_BUF_FROM_REF(param);
   zb_zdo_nwk_addr_resp_head_t *resp;
  // zb_ieee_addr_t ieee_addr;
   resp=( zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
   TRACE_MSG(TRACE_ZDO2,"Nwk_addr %hd %d",(FMT__H_D , resp->status,resp->nwk_addr));
}
void zc_nwk(zb_uint8_t param)
{
   zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
    param=ZB_REF_FROM_BUF(buf);
    zb_zdo_nwk_addr_req_param_t *req_param=ZB_GET_BUF_PARAM(buf,zb_zdo_nwk_addr_req_param_t);
    req_param->dst_addr=0x0000;
    zb_address_ieee_by_ref(req_param->ieee_addr, 0x0000);
    req_param->request_type=ZB_ZDO_SINGLE_DEVICE_RESP;
    req_param->start_index=0;
    zb_zdo_nwk_addr_req(param,zb_get_sh);
}
void ieee_addr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t * resp;
  //zb_ieee_addr_t ieee_addr;
 // zb_uint16_t nwk_addr;
  //zb_address_ieee_ref_t addr_ref;
  resp=(zb_zdo_nwk_addr_resp_head_t *)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2,"ieee_address %d",(FMT__D,resp->ieee_addr));
  zb_free_buf(buf);
}
void zc_ieee_addr(zb_uint8_t param)
{
    zb_zdo_ieee_addr_req_t *req = NULL;
    zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
    param=ZB_REF_FROM_BUF(buf);
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t),req);
    req->nwk_addr=0x0000;
    req->request_type=ZB_ZDO_SINGLE_DEV_RESPONSE;
    req->start_index=0;
    zb_zdo_ieee_addr_req(param,ieee_addr);
}
zb_uint8_t* endpoint;
void simple_desc(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_simple_desc_resp_t * resp;
  //zb_ieee_addr_t ieee_addr;
 // zb_uint16_t nwk_addr;
  //zb_address_ieee_ref_t addr_ref
  resp=(zb_zdo_simple_desc_resp_t *)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2,"Endpoint %hd Profile %d",(FMT__H_D,resp->simple_desc.endpoint,resp->simple_desc.app_profile_id));
  TRACE_MSG(TRACE_ZDO2,"DeviceID %hd DeviceVer %d",(FMT__D_H,resp->simple_desc.app_device_id,resp->simple_desc.app_device_version));
  TRACE_MSG(TRACE_APS1,"clusters:",(FMT__0));
  for(zb_uint_t i=0;i<resp->simple_desc.app_input_cluster_count+resp->simple_desc.app_output_cluster_count;i++)
  {
    TRACE_MSG(TRACE_APS1, "icluster  %hx", (FMT__H , resp->simple_desc.app_cluster_list[i]));
  }
  zb_free_buf(buf);
}
void zc_simple_req(zb_uint8_t param)
{ 
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_simple_desc_req_t *req;
  ZB_BUF_INITIAL_ALLOC(buf,sizeof(zb_zdo_simple_desc_req_t),req);
  req->endpoint =*endpoint;
  req->nwk_addr=0;
  zb_zdo_simple_desc_req(ZB_REF_FROM_BUF(buf),simple_desc);
}
void power_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_power_desc_resp_t * resp;
  //zb_ieee_addr_t ieee_addr;
 // zb_uint16_t nwk_addr;
  //zb_address_ieee_ref_t addr_ref
  resp=(zb_zdo_power_desc_resp_t *)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2,"Power_mode %hd",(FMT__H,ZB_GET_POWER_DESC_CUR_POWER_MODE(&resp->power_desc)));
}
void zc_power_req(zb_uint8_t param)
{
    zb_zdo_power_desc_req_t *req = NULL;
    zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
    param=ZB_REF_FROM_BUF(buf);
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t),req);
    req->nwk_addr=0x0000;
   // req->request_type=ZB_ZDO_SINGLE_DEV_RESPONSE;
   // req->start_index=0;
    zb_zdo_power_desc_req(param,power_req);
}

void active_ep(zb_uint8_t param)
{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_ep_resp_t * resp;
    //zb_ieee_addr_t ieee_addr;
   // zb_uint16_t nwk_addr;
    //zb_address_ieee_ref_t addr_ref
    resp=(zb_zdo_ep_resp_t *)ZB_BUF_BEGIN(buf);
    endpoint=ZB_BUF_BEGIN(buf)+sizeof(zb_zdo_ep_resp_t);
    TRACE_MSG(TRACE_ZDO2,"Endpoint %hd",(FMT__H,*endpoint));
}
void zc_active_ep(zb_uint8_t param)
{
    zb_zdo_active_ep_req_t *req = NULL;
    zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
    param=ZB_REF_FROM_BUF(buf);
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t),req);
    req->nwk_addr=0x0000;
   // req->request_type=ZB_ZDO_SINGLE_DEV_RESPONSE;
   // req->start_index=0;
    zb_zdo_active_ep_req(param,active_ep);

}
static void zc_send_data(zb_uint8_t param)
{
    zc_nwk(param);
    zc_ieee_addr(param);
    zc_active_ep(param);
   // zc_simple_req(param);
   // zc_power_req(param);
    ZB_SCHEDULE_ALARM(zc_simple_req,param,2*ZB_TIME_ONE_SECOND);
    ZB_SCHEDULE_ALARM(zc_power_req,param,4*ZB_TIME_ONE_SECOND);
   /* TRACE_MSG(TRACE_APS1, "Recall fuction", (FMT__0)); 
    ZB_SCHEDULE_ALARM(zc_send_data,0,5*ZB_TIME_ONE_SECOND);*/
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
#ifndef APS_RETRANSMIT_TEST
     zb_af_set_data_indication(zc_respond);

     ZB_SCHEDULE_CALLBACK(checkf, ZB_REF_FROM_BUF(buf));

#endif
   // send_data((zb_buf_t *)ZB_BUF_FROM_REF(param));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}



#ifndef APS_RETRANSMIT_TEST
/*void data_indication(zb_uint8_t param)
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  //Remove APS header from the packet 
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS2, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));
  /////////
  //checkf(ptr[0]);
  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS2, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
    if (ptr[i] != i % 32 + '0')
    {
      TRACE_MSG(TRACE_ERROR, "Bad data %hx %c wants %x %c", (FMT__H_C_D_C, ptr[i], ptr[i],
                              (int)(i % 32 + '0'), (char)i % 32 + '0'));
    }
  }

  send_data(asdu);
}*/
#endif


/*! @} */
