/**
 * @file key_module.h
 * @author Jason Wen (embediot@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-3
 * 
 * @copyright EmbedIoT Studio Copyright (c) 2024
 * 
 */
#ifndef _KEY_MODULE_H_
#define _KEY_MODULE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "key_driver.h"

//按键模块参数配置宏
#define KEY_NUM             KEY_ID_MAX     //按键个数
#define DEBOUNCE_TIME       50    //按键消抖时间，单位：毫秒
#define LONG_PRESS_TIME     1000  //长按时间，单位：毫秒
#define DOUBLE_CLICK_TIME   150   //双击间隔时间，在此时间内按第二下，才算双击，单位：毫秒

//按键状态枚举值
typedef enum{
    KEY_IDLE = 0,
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_SINGLE_CLICK,
    KEY_DOUBLE_CLICK,
    KEY_LONG_PRESS,
}key_state_t;

//单个按键信息结构体
typedef struct key
{
    key_id_t key_id;      //按键id，通常是0~KEY_NUM-1
    key_state_t state;         //按键状态枚举
    unsigned int last_press_time;    //记录按下的时间点
    unsigned int last_release_time;    //记录抬起的时间点
    unsigned int press_duration;    //按下了多长时间
    bool is_debouncing;    //是否处于消抖周期
    unsigned int debounce_start_time;    //开始进行消抖的时间点
    bool awaiting_double_click;    //按键双击检测
}key_t;

//按键事件回调函数
typedef void  (*key_event_handler_t)(key_t *key);

//按键管理器，用于管理所有按键的状态与回调
typedef struct key_manager{
    key_t keys[KEY_NUM];      //定义多个按键
    key_event_handler_t on_single_click;    //单击的函数指针
    key_event_handler_t on_double_click;    //双击的函数指针
    key_event_handler_t on_long_press;      //长按的函数指针
    key_event_handler_t on_press;           //按下的函数指针
    key_event_handler_t on_release;         //释放的函数指针

    key_driver_t *key_drv;    //按键模块的MCU引脚驱动

    unsigned int ticks;    //计数时基，在定时器中不断累加，建议以1毫秒或10毫秒为单位
    unsigned int long_press_time;    //长按时间，表示按多久才算是长按
    unsigned int double_click_time;    //双击间隔，表示在此间隔内按下第二次才算双击
}key_manager_t;


//按键状态与按键事件回调匹配表
typedef struct key_event_tbl{
    unsigned char key_id;
    key_state_t key_state;
    key_event_handler_t event_handler;
}key_event_tbl_t;


extern void key_module_init(key_manager_t *manager);
extern void key_module_update(key_manager_t *manager);
extern void key_module_ticks_update(key_manager_t *manager);
extern void key_module_set_event_handler(key_manager_t *manager, key_state_t state, key_event_handler_t handler);


#endif

