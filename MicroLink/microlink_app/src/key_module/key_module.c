/**
 * @file key_module.c
 * @author Jason Wen (embediot@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-3
 * 
 * @copyright EmbedIoT Studio Copyright (c) 2024
 * 
 */
#include "key_module.h"

static bool init_flag = false;    //模块初始化标志位
    
/**
 * @brief 初始化按键模块，被外部第三方调用
 * 
 * @param manager：按键管理器实例 
 */
void key_module_init(key_manager_t *manager)
{
    int i = 0;

    for(i = 0;i < KEY_NUM; i++)
    {
        manager->keys[i].key_id = (key_id_t)i;
        manager->keys[i].state = KEY_IDLE;
        manager->keys[i].last_press_time = 0;
        manager->keys[i].last_release_time = 0;
        manager->keys[i].press_duration = 0;
        manager->keys[i].is_debouncing = false;
        manager->keys[i].debounce_start_time = 0;
        manager->keys[i].awaiting_double_click = false;
    }

    manager->on_single_click = NULL;
    manager->on_double_click = NULL;
    manager->on_long_press = NULL;
    manager->on_press = NULL;
    manager->on_release = NULL;

    manager->ticks = 0;
    manager->long_press_time = LONG_PRESS_TIME;
    manager->double_click_time = DOUBLE_CLICK_TIME;

    //初始化按键模块的引脚驱动
    manager->key_drv = &key_driver;    
    manager->key_drv->init();

    init_flag = true;    //已经被初始化，可以正常工作
}

/**
 * @brief 按键模块刷新，在while(1)或定时器中被周期调用
 * 
 * @param manager 按键管理模块
 */
void key_module_update(key_manager_t *manager)
{
    int i = 0;
    key_t *key = NULL;
    bool key_pin_state = false;    //按键引脚的电平状态

    if(!init_flag)return;    //未被初始化不允许工作
    if(NULL == manager)return;
    if(NULL == manager->key_drv)return;
    
    for(i = 0; i < KEY_NUM; i++)
    {
        key = &manager->keys[i];    //取出某个按键
        key_pin_state = manager->key_drv->read_pin_state(key->key_id);    //读取某个按键引脚的电平状态

        if(key->is_debouncing)    //有按键被按下，进行消抖
        {
            if(manager->ticks - key->debounce_start_time > DEBOUNCE_TIME)key->is_debouncing = false;
            else continue;
        }

        switch(key->state)
        {
            case KEY_IDLE:
            {
                if(key_pin_state)    //有按键按下
                {
                    key->state = KEY_PRESSED;
                    key->last_press_time = manager->ticks;
                    key->debounce_start_time = manager->ticks;
                    key->is_debouncing = true;
                    if(manager->on_press)manager->on_press(key);
                }		
            }break;

            case KEY_PRESSED:
            {
                if(!key_pin_state)    //按键被释放了
                {
                    key->state = KEY_RELEASED;
                    key->last_release_time = manager->ticks;
                    key->press_duration = manager->ticks - key->last_press_time;
                    key->debounce_start_time = manager->ticks;
                    key->is_debouncing = true;
                    if(manager->on_release)manager->on_release(key);

                    if(key->awaiting_double_click)    //处理双击
                    {
                        key->awaiting_double_click = false;
                        key->state = KEY_DOUBLE_CLICK;    //按键双击
                        if(manager->on_double_click)manager->on_double_click(key);
                    }
                    else key->awaiting_double_click = true;
                }
                else if(manager->ticks - key->last_press_time > manager->long_press_time)    //按键被长按了
                {
                    key->state = KEY_LONG_PRESS;
                    if(manager->on_long_press)manager->on_long_press(key);
                }
            }break;

            case KEY_RELEASED:
            {
                if(key_pin_state)    //按键又被按下了
                {
                    key->state = KEY_PRESSED;    //普通被按下
                    key->last_press_time = manager->ticks;
                    key->debounce_start_time = manager->ticks;
                    key->is_debouncing = true;
                    if(manager->on_press)manager->on_press(key);
                }
                else if(manager->ticks - key->last_release_time > manager->double_click_time)    //双击处理
                {
                    if(key->awaiting_double_click)
                    {
                        key->awaiting_double_click = false;
                        key->state = KEY_SINGLE_CLICK; 
                        if(manager->on_single_click)manager->on_single_click(key);         
                    }else key->state = KEY_IDLE;
                }else {}
            }break;

            case KEY_SINGLE_CLICK:
            case KEY_DOUBLE_CLICK:
            case KEY_LONG_PRESS:
                if(!key_pin_state)
                {
                    key->state = KEY_IDLE;
                    key->debounce_start_time = manager->ticks;
                    key->is_debouncing = true;
                }
                
            break;

            default:break;
        }
    }
}

/**
 * @brief 按键管理器的时基递增函数，在1毫秒或10毫秒的中断里面被周期调用
 * 
 * @param manager 
 */
void key_module_ticks_update(key_manager_t *manager)
{
    if(!init_flag)return;    //未被初始化不允许工作
    manager->ticks++;
}

/**
 * @brief 设置按键事件回调函数
 * 
 * @param manager 按键管理器
 * @param state 按键活动状态
 * @param handler 回调函数
 */
void key_module_set_event_handler(key_manager_t *manager, key_state_t state, key_event_handler_t handler)
{
    if(NULL == manager)return;

    if(state == KEY_PRESSED)manager->on_press = handler;
    else if(state == KEY_RELEASED)manager->on_release = handler;
    else if(state == KEY_SINGLE_CLICK)manager->on_single_click = handler;
    else if(state == KEY_DOUBLE_CLICK)manager->on_double_click = handler;
    else if(state == KEY_LONG_PRESS)manager->on_long_press = handler;
    else {}
}

