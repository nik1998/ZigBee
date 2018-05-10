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
#define dToggle 2
#define dStepUp 1
#define dChangeColor 3
#define ZB_TEST_DUMMY_DATA_SIZE 10
#include "stm32.h"
zb_ieee_addr_t g_zc_addr = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

/*! \addtogroup ZB_TESTS */
/*! @{ */

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

/*
  The test is: ZC starts PAN, ZR joins to it by association and send APS data packet, when ZC
  received packet, it sends packet to ZR, when ZR received packet, it sends
  packet to ZC etc.
 */
//zb_uint8_t stat=1;
int ind=1;
//int color =1;
int arr[3];
int a[10]={1,1,1,2,3,1,1,2,3,2};
static int cur=0;
static void zc_send_data(zb_buf_t *buf);
//void checkf(zb_uint8_t param);
//void data_indication(zb_uint8_t param) ZB_CALLBACK;
void TimTim();
void Initleds(int Pins, int PinSource);
void MyInit();
void CommandParse(zb_uint8_t param);
MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif


  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  //MyInit();
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = 0x1aaa;

  /* let's always be a coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  ZB_AIB().aps_channel_mask = (1l << 22);

  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();
  zb_af_set_data_indication(CommandParse);
  MAIN_RETURN(0);
}

/*static void check(zb_buf_t *buf)
{
    zb_uint8_t *ptr = NULL;
    GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15); 
    ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DUMMY_DATA_SIZE , ptr);
    if(cur<10)
    {
      ptr[0]=a[cur];
      CommandParse(ZB_REF_FROM_BUF(buf));
      cur+=1;
      ZB_SCHEDULE_ALARM(check,buf,3*ZB_TIME_ONE_SECOND);
    }
    //ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
 }*/

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    zb_af_set_data_indication(CommandParse);
    ZB_SCHEDULE_ALARM(zc_send_data,buf,5*ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
  }
  zb_free_buf(buf);
}
void MyInit()
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  //Init LedGPIO
  GPIO_InitTypeDef  GPIO_InitStructure1;
  GPIO_InitStructure1.GPIO_Pin= GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure1.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure1.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure1);
  Initleds(GPIO_Pin_8,GPIO_PinSource8);
  TimTim();
  arr[0]=0;
  arr[1]=0;
  arr[2]=0;
  GPIO_SetBits(GPIOD,GPIO_Pin_14);
}

void Initleds(int Pins,int PinSource)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   // GPIO_PinAFConfig(GPIOA,PinSource,GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource8/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
 // GPIO_InitStructure.GPIO_Pin= Pins;
    GPIO_InitStructure.GPIO_Pin= GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void TimTim()
{
    TIM_TimeBaseInitTypeDef ttt;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
    ttt.TIM_Period=100000/60-1;
    ttt.TIM_Prescaler=1680;
    ttt.TIM_ClockDivision=0;
    ttt.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1,&ttt);
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
    TIM_Cmd(TIM1,ENABLE);
    TIM_OCInitTypeDef aaa;
    aaa.TIM_OCMode=TIM_OCMode_PWM1;
    aaa.TIM_Pulse=0;
    aaa.TIM_OCPolarity=TIM_OCPolarity_Low; 
    TIM_OC1Init(TIM1,&aaa);
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);
    TIM_OC2Init(TIM1,&aaa);
    TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);
    TIM_OC3Init(TIM1,&aaa);
    TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Enable);
}
void CommandParse(zb_uint8_t param)
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr;
  ZB_APS_HDR_CUT_P(asdu, ptr);
  switch(*ptr)
  {
    case dStepUp:
    {
      arr[ind-1]+=100;
      if(arr[ind-1]>1600)
      {
        arr[ind-1]=0;
      }
      if(ind==1)
      TIM_SetCompare1(TIM1,arr[ind-1]);
      else
      if(ind==2)
      TIM_SetCompare2(TIM1,arr[ind-1]); 
      else
      TIM_SetCompare3(TIM1,arr[ind-1]);
      break;
    }
    case dToggle:
    {
      if(arr[ind-1]==0)
      {
        arr[ind-1]=800;
      }
      else
      {
        arr[ind-1]=0;
      }
      if(ind==1)
        TIM_SetCompare1(TIM1,arr[ind-1]);
      else
      if(ind==2)
        TIM_SetCompare2(TIM1,arr[ind-1]); 
      else
      TIM_SetCompare3(TIM1,arr[ind-1]); 
      break;
    }
     case dChangeColor:
    {
       GPIO_ResetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15); 
      if(ind==1)
      {
        GPIO_SetBits(GPIOD,GPIO_Pin_12); 
        ind=2;
      }
      else
            if(ind==2)
            {
              GPIO_SetBits(GPIOD,GPIO_Pin_15); 
              ind=3;
            }
            else
            if(ind==3)
            {
              GPIO_SetBits(GPIOD,GPIO_Pin_14); 
              ind=1;
            }
      break;
    }
 }
}
/*void checkf(zb_uint8_t param)
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr;
  ZB_APS_HDR_CUT_P(asdu, ptr);
 // TRACE_MSG(TRACE_APS2, "aaaaaaaaaa", (FMT__0));
  switch(*ptr)
  {

     case dToggle:
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
     case dChangeColor:
     {
        color =(color)%3+1;
        TRACE_MSG(TRACE_APS2, "Color %d", (FMT__D,color));
        break;
     }
     case dStepUp:
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
         default:
         TRACE_MSG(TRACE_APS2,"Unknown command!!!",(FMT__0));
  }
  //send_data(param);
}*/


/*
   Trivial test: dump all APS data received
 */


/*void data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);

  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS3, "apsde_data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  zc_send_data(asdu, ind->src_addr);
}*/

static void zc_send_data(zb_buf_t *buf)
{
    zb_apsde_data_req_t *req;
    zb_uint8_t *ptr = NULL;
    zb_short_t i;

    ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DUMMY_DATA_SIZE , ptr);
    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
    req->dst_addr.addr_short = 0x0001;
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;
    buf->u.hdr.handle = 0x11;
    for (i = 0 ; i < ZB_TEST_DUMMY_DATA_SIZE ; i++)
    {
      ptr[i] = i + '0';
    }
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
 }

/*! @} */
