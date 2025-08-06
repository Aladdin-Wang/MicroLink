#include "key_task.h"
#include "perf_counter.h"
#include "stdio.h"
#include "pikaScript.h"
#include "PikaObj.h"
#include "PikaVM.h"

#define GET_SYS_TIME_MS() perfc_convert_ticks_to_ms(get_system_ticks())

key_manager_t key_manager;    //定义一个按键管理器对象

extern PikaObj* pikaMain;

static void on_press_handler(key_t *key)
{

}

static void on_release_handler(key_t *key)
{
    if(pikaMain){
        pikaVM_runSingleFile(pikaMain,"offline_download.py");
    }
}

static void on_single_click_handler(key_t *key)
{
    static unsigned int single_click = 0;
    single_click++;
    printf("single\r\n");
}

static void on_double_click_handler(key_t *key)
{
    static unsigned int double_click = 0;
    double_click++;
    printf("double\r\n");
}

static void on_long_press_handler(key_t *key)
{
    static unsigned int long_press = 0;
    long_press++;
    printf("long\r\n");
}

void key_init(void)
{
    key_module_init(&key_manager);    //初始化按键管理器

    //设置各种按键状态的回调函数
    key_module_set_event_handler(&key_manager, KEY_PRESSED, on_press_handler);
    key_module_set_event_handler(&key_manager, KEY_RELEASED, on_release_handler);
    key_module_set_event_handler(&key_manager, KEY_SINGLE_CLICK, on_single_click_handler);
    key_module_set_event_handler(&key_manager, KEY_DOUBLE_CLICK, on_double_click_handler);
    key_module_set_event_handler(&key_manager, KEY_LONG_PRESS, on_long_press_handler);
}
fsm_rt_t key_module_display(uint64_t ms)
{
    #define RESET_KEY_TASK()    do{chState = 0;}while(0)
    static uint8_t chState = 0;
    static uint64_t wtime;
    enum{START = 0, IS_TIMEOUT};
    switch(chState){
        case START:{
            wtime = GET_SYS_TIME_MS() + ms;
            chState = IS_TIMEOUT;
        }
        case IS_TIMEOUT:{
            if(GET_SYS_TIME_MS() >= wtime){
                key_module_ticks_update(&key_manager);
                key_module_update(&key_manager);
                RESET_KEY_TASK();
                return fsm_rt_cpl;
            }
            break;
        }	
    }
    return fsm_rt_on_going;
}