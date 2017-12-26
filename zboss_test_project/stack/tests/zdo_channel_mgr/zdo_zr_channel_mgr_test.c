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
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"


/*! \addtogroup ZB_TESTS */
/*! @{ */

//static void send_data(zb_buf_t *buf);
void node_power_desc_callback(zb_uint8_t param) ZB_CALLBACK;

static zb_int_t g_error_counter = 0;
static zb_int_t g_counter = 0;

#define MAX_TEST_ITER_NUMBER 30

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
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif

  zb_set_default_ffd_descriptor_values(ZB_ROUTER);

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

  TRACE_DEINIT();

  MAIN_RETURN(0);
}

void node_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_node_desc_resp_t *resp = (zb_zdo_node_desc_resp_t*)(zdp_cmd);
  zb_zdo_node_desc_req_t *req;

  g_counter++;

  if (resp->hdr.status == ZB_ZDP_STATUS_SUCCESS)
  {
    TRACE_MSG(TRACE_APS2, "node desc received Ok", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "ERROR receiving node desc %x", (FMT__D, resp->hdr.status));
    g_error_counter++;
  }

  if (g_counter < MAX_TEST_ITER_NUMBER)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_node_desc_req_t), req);
    req->nwk_addr = 0; //send to coordinator
    zb_zdo_node_desc_req(ZB_REF_FROM_BUF(buf), node_desc_callback);
  }
  else
  {
    TRACE_MSG(TRACE_APS2, "Test is finished", (FMT__0));
    zb_free_buf(buf);
  }
}

void system_server_discovery_cb(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_system_server_discovery_resp_t *resp = (zb_zdo_system_server_discovery_resp_t*)(zdp_cmd);
  zb_zdo_node_desc_req_t *req;

  g_counter++;

  if (resp->status == ZB_ZDP_STATUS_SUCCESS && resp->server_mask & ZB_NETWORK_MANAGER )
  {
    TRACE_MSG(TRACE_APS3, "system_server_discovery received Ok", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "ERROR receiving system_server_discovery status %x, mask %x",
              (FMT__D_D, resp->status, resp->server_mask));
    g_error_counter++;
  }

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_node_desc_req_t), req);
  req->nwk_addr = 0; //send to coordinator
  zb_zdo_node_desc_req(ZB_REF_FROM_BUF(buf), node_desc_callback);
}

void func1()
{
  zb_buf_t *asdu;

  asdu = zb_get_out_buf();
  if (!asdu)
  {
    TRACE_MSG(TRACE_ERROR, "out buf alloc failed!", (FMT__0));
  }
  else
  {
    zb_zdo_system_server_discovery_param_t *req_param;

    req_param = ZB_GET_BUF_PARAM(asdu, zb_zdo_system_server_discovery_param_t);
    req_param->server_mask = ZB_NETWORK_MANAGER;

    zb_zdo_system_server_discovery_req(ZB_REF_FROM_BUF(asdu), system_server_discovery_cb);
  }
}


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    func1();
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}


void data_indication(zb_uint8_t param)
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS3, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS3, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
  }

  //send_data(asdu);
}



/*! @} */
