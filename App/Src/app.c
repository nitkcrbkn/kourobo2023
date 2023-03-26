#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include "message.h"
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "MW_flash.h"
#include "constManager.h"
#include "trapezoid_ctrl.h"
/*suspensionSystem*/

/*アーム関連の定数*/
#define AUTO_UP 100
#define AUTO_DOWN -50
#define AUTO_ARM_WIDTH 250
#define MOTOR_SPEED_ARM 1000
#define MOTOR_SPEED_UPDOWN 3000
#define ARM_MAG 0.5 /*早いほうのモーターの倍率から1引いた値*/

static
int suspensionSystem(void);

static
int armSystem(void);

static
int upDownSystem(void);



/* 腕振り部の変数 */
int situation = 0;
int judgepush = 0;

/* モードチェンジ用変数 */
int mode = 0;
int checkpush = 0;

/*メモ
 *g_ab_h...ABのハンドラ
 *g_md_h...MDのハンドラ
 *
 *g_rc_data...RCのデータ
 */

#define WRITE_ADDR (const void*)(0x8000000+0x400*(128-1))/*128[KiB]*/
flashError_t checkFlashWrite(void){
    const char data[]="HelloWorld!!TestDatas!!!\n"
                      "however you like this microcomputer, you don`t be kind to this computer.";
    return MW_flashWrite(data,WRITE_ADDR,sizeof(data));
}

int appInit(void){
    message("msg","hell");

    /* switch(checkFlashWrite()){ */
    /* case MW_FLASH_OK: */
    /*   message("msg","FLASH WRITE TEST SUCCESS\n%s",(const char*)WRITE_ADDR); */
    /*   break; */
    /* case MW_FLASH_LOCK_FAILURE: */
    /*   message("err","FLASH WRITE TEST LOCK FAILURE\n"); */
    /*   break; */
    /* case MW_FLASH_UNLOCK_FAILURE: */
    /*   message("err","FLASH WRITE TEST UNLOCK FAILURE\n"); */
    /*   break; */
    /* case MW_FLASH_ERASE_VERIFY_FAILURE: */
    /*   message("err","FLASH ERASE VERIFY FAILURE\n"); */
    /*   break; */
    /* case MW_FLASH_ERASE_FAILURE: */
    /*   message("err","FLASH ERASE FAILURE\n"); */
    /*   break; */
    /* case MW_FLASH_WRITE_VERIFY_FAILURE: */
    /*   message("err","FLASH WRITE TEST VERIFY FAILURE\n"); */
    /*   break; */
    /* case MW_FLASH_WRITE_FAILURE: */
    /*   message("err","FLASH WRITE TEST FAILURE\n"); */
    /*   break;         */
    /* default: */
    /*   message("err","FLASH WRITE TEST UNKNOWN FAILURE\n"); */
    /*   break; */
    /* } */
    /* flush(); */

    ad_init();

    message("msg","plz confirm\n%d\n",g_adjust.rightadjust.value);

    /*GPIO の設定などでMW,GPIOではHALを叩く*/
    return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
    int ret=0;

    /*if(__RC_ISPRESSED_R1(g_rc_data)&&__RC_ISPRESSED_R2(g_rc_data)&&
       __RC_ISPRESSED_L1(g_rc_data)&&__RC_ISPRESSED_L2(g_rc_data)){
      while(__RC_ISPRESSED_R1(g_rc_data)||__RC_ISPRESSED_R2(g_rc_data)||
        __RC_ISPRESSED_L1(g_rc_data)||__RC_ISPRESSED_L2(g_rc_data));
      ad_main();
      }*/

    /*それぞれの機構ごとに処理をする*/
    /*途中必ず定数回で終了すること。*/
    ret = suspensionSystem();
    if(ret){
        return ret;
    }

    ret = upDownSystem();
    if(ret){
        return ret;
    }

    ret = armSystem();
    if(ret){
        return ret;
    }

    return EXIT_SUCCESS;
}


/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
    const int num_of_motor = 2;/*モータの個数*/
    int rc_analogdata;/*アナログデータ*/
    unsigned int idx;/*インデックス*/
    int i;

    const tc_const_t tc ={
            .inc_con = 100,
            .dec_con = 225,
    };

    /* コントローラのボタンは押されてるか */
    if(!__RC_ISPRESSED_TRIANGLE(g_rc_data)){
        checkpush = 1;
    }

/*
  if(__RC_ISPRESSED_TRIANGLE(g_rc_data) && checkpush == 1 && !__RC_ISPRESSED_CROSS(g_rc_data)){

    switch(mode){
    case 1:
      mode = 0;
      break;
    case 0:
      mode = 1;
      break;
    }

    checkpush = 0;
  }
*/

    /*for each motor*/
    for(i=0;i<num_of_motor;i++){
        /*それぞれの差分*/
        switch(i){
            case 0:
                idx = MECHA1_MD0;
                rc_analogdata = DD_RCGetRY(g_rc_data);
                break;
            case 1:
                idx = MECHA1_MD1;
                rc_analogdata = -DD_RCGetLY(g_rc_data);
                break;

            default:
                return EXIT_FAILURE;
        }
        trapezoidCtrl(rc_analogdata * MD_GAIN * ((i-1) * -0.2 + 1),&g_md_h[idx],&tc);
    }


    return EXIT_SUCCESS;
}


static
int armSystem(void){
    unsigned int idx;/*インデックス*/
    int i;
    int duty = 0;
    int flagMD=-1; /*1番でどっちも、2番でMD2番、3番でMD3番*/
    static int flagAutoArm = 0;  /*正なら開く動作、負なら閉じる動作を示す*/

    const tc_const_t tc ={
            .inc_con = 100,
            .dec_con = 225,
    };

    if(flagAutoArm > 0){ /*開く動作中ならtrue*/
        flagMD = 1;
        flagAutoArm -= 1;
        duty = MOTOR_SPEED_ARM;
    }
    else if(flagAutoArm < 0){ /*閉じる動作中ならture*/
        flagMD = 1;
        flagAutoArm += 1;
        duty = MOTOR_SPEED_ARM * -1;
    }
    else{
        if(__RC_ISPRESSED_CIRCLE(g_rc_data)){
            flagMD = 1;
            flagAutoArm = 0;
            duty= MOTOR_SPEED_ARM;
        }
        else if(__RC_ISPRESSED_CROSS(g_rc_data)){
            flagMD = 1;
            flagAutoArm = 0;
            duty = MOTOR_SPEED_ARM * -1;
        }
        else if(__RC_ISPRESSED_TRIANGLE(g_rc_data)){
            flagMD = 1;
            flagAutoArm = AUTO_ARM_WIDTH;
            duty = MOTOR_SPEED_ARM;
        }
        else if(__RC_ISPRESSED_SQARE(g_rc_data)){
            flagMD = 1;
            flagAutoArm = AUTO_ARM_WIDTH * -1;
            duty = MOTOR_SPEED_ARM * -1;
        }
        else if(__RC_ISPRESSED_R1(g_rc_data)){
            flagMD = 2;
            duty = MOTOR_SPEED_ARM;
        }
        else if(__RC_ISPRESSED_R2(g_rc_data)){
            flagMD = 2;
            duty = MOTOR_SPEED_ARM * -1;
        }
        else if(__RC_ISPRESSED_L1(g_rc_data)){
            flagMD = 3;
            duty = MOTOR_SPEED_ARM * (1 + ARM_MAG);
        }
        else if(__RC_ISPRESSED_L2(g_rc_data)){
            flagMD = 3;
            duty = duty = MOTOR_SPEED_ARM * (1 + ARM_MAG) * -1;
        }
        else{
            flagAutoArm = 0;
            duty = 0;
        }
    }
    if(flagMD == 1){
        for(idx=0;idx<=1;idx++){
            trapezoidCtrl(duty*(0.5*idx + 1),&g_md_h[idx+2],&tc);
        }
    }
    else if(flagMD == 2){
        for(idx=0;idx<=1;idx++){
            trapezoidCtrl(duty * (idx-1) * -1,&g_md_h[idx+2],&tc);
        }
    }
    else if(flagMD == 3){
        for(idx=0;idx<=1;idx++){
            trapezoidCtrl(duty * idx ,&g_md_h[idx+2],&tc);
        }
    }
    else{
        for(idx=0;idx<=1;idx++){
            trapezoidCtrl(0 ,&g_md_h[idx+2],&tc);
        }
    }

    return EXIT_SUCCESS;
}

static
int upDownSystem(void) {
    unsigned int idx = 4;/*インデックス*/
    int i;
    int duty;
    static int flagAutoUpDown = 0;

    const tc_const_t tc = {
            .inc_con = 100,
            .dec_con = 225,
    };

    if (flagAutoUpDown > 0) { /*上昇中なら*/
        flagAutoUpDown -= 1;
        duty = MOTOR_SPEED_UPDOWN;
    } else if (flagAutoUpDown < 0) { /*下昇中なら*/
        flagAutoUpDown += 1;
        duty = MOTOR_SPEED_UPDOWN * -1;
    } else {
        if (__RC_ISPRESSED_RIGHT(g_rc_data)) {
            flagAutoUpDown = AUTO_UP;
            duty = MOTOR_SPEED_UPDOWN;
        } else if (__RC_ISPRESSED_LEFT(g_rc_data)) {
            flagAutoUpDown = AUTO_DOWN;
            duty = MOTOR_SPEED_UPDOWN * -1;
        } else if (__RC_ISPRESSED_UP(g_rc_data)) {
            flagAutoUpDown = 0;
            duty = MOTOR_SPEED_UPDOWN;
        } else if (__RC_ISPRESSED_DOWN(g_rc_data)) {
            flagAutoUpDown = 0;
            duty = MOTOR_SPEED_UPDOWN * -1;
        } else {
            duty = 0;
        }
    }


    trapezoidCtrl(duty, &g_md_h[idx], &tc);
    return EXIT_SUCCESS;
}