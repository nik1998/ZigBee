#!/bin/sh
#
#/***************************************************************************
#*                                                                          *
#* INSERT COPYRIGHT HERE!                                                   *
#*                                                                          *
#****************************************************************************
#
# Purpose:
#

PIPE_NAME=/tmp/st

wait_for_start() {
    nm=$1
    s=''
    while [ A"$s" = A ]
    do
        sleep 1
        if [ -f core ]
        then
            echo 'Somebody has crashed (ns?)'
            killch
            exit 1
        fi
        s=`grep Device zdo_${nm}*.log`
    done
    if echo $s | grep OK
    then
        return
    else
        echo $s
        killch
        exit 1
    fi
}

killch() {
    kill $router2PID $router1PID $coordPID $PipePID
    echo Kill -9 $PipePID
    kill -9 $PipePID
}

killch_ex() {
    killch
    echo Interrupted by user!
    exit 1
}

trap killch_ex TERM INT



rm -f *.log *.pcap *.dump core

echo "run ns"
../../../devtools/network_simulator/network_simulator --nNode=3 --pipeName=${PIPE_NAME}  1>sim_log.txt 2>&1 &
PipePID=$!
echo sim started, pid $PipePID

sleep 5

echo "run coordinator"
./tr_pro_bv_37_DUTZC ${PIPE_NAME}0.write ${PIPE_NAME}0.read &
coordPID=$!
wait_for_start zc

echo ZC STARTED OK
sleep 1

echo "run zr1"
./tr_pro_bv_37_DUTZR1 ${PIPE_NAME}1.write ${PIPE_NAME}1.read &
router1PID=$!
wait_for_start zr1

echo ZR1 STARTED OK
sleep 1

echo "run zr2"
./tr_pro_bv_37_DUTZR2 ${PIPE_NAME}2.write ${PIPE_NAME}2.read &
router2PID=$!
wait_for_start zr2

echo ZR2 STARTED OK

# ZB_NWK_BROADCAST_DELIVERY_TIM is 30 sec long
sleep 60

echo shutdown...
killch

if grep "packets_sent_cb" zdo_zr*.log
then
    if grep "zb_zdo_mgmt_nwk_update_req" zdo_zc*.log
    then
        echo "DONE. TEST PASSED!!!"
    else
        echo "test packets sent, but channel is not changed, ERROR"
    fi
else
  echo "ERROR. test packets are not sent"
fi

