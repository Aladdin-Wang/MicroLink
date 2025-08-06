#include "BaseObj.h"
#include "dataStrs.h"
#include "pika_hal.h"
#include "hpm_gpio_drv.h"
#include "board.h"


typedef struct platform_data_GPIO {
    uint16_t gpioPin;
    uint32_t pinMode;
} platform_data_GPIO;

/**
 * @brief Get Pin index from name
 *
 * Name rule is : <GPIO NAME><Index>
 *  for example: PA00, PZ03
 *
 **/
static int32_t hpm_pin_get(const char *name)
{
    if (!(  (strlen(name) == 4) &&
            (name[0] == 'P') &&
            ((('A' <= name[1]) && (name[1] <= 'F')) || (('V' <= name[1]) && (name[1] <= 'Z'))) &&
            (('0' <= name[2]) && (name[2] <= '9')) &&
            (('0' <= name[3]) && (name[3] <= '9'))
        ))
    {
        return -1;
    }

    uint32_t gpio_idx = (name[1] <= 'F') ? (name[1] - 'A') : (11 + name[1] - 'V');
    uint32_t pin_idx = (uint32_t)(name[2] - '0') * 10 + (name[3] - '0');
    return (gpio_idx * 32 + pin_idx);
}

int pika_hal_platform_GPIO_open(pika_dev* dev, char* name) {
    platform_data_GPIO* gpio =
        (platform_data_GPIO*)pikaMalloc(sizeof(platform_data_GPIO));
    if (NULL == gpio) {
        return -1;
    }
    memset(gpio, 0, sizeof(platform_data_GPIO));

    HPM_IOC->PAD[4].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(0);
    HPM_IOC->PAD[4].PAD_CTL =  IOC_PAD_PAD_CTL_PRS_SET(2) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1);
    gpio_set_pin_output_with_initial(HPM_GPIO0, GPIO_DO_GPIOA, 4, 0);
    return 0;
}


int pika_hal_platform_GPIO_close(pika_dev* dev) {
    platform_data_GPIO* gpio = (platform_data_GPIO*)dev->platform_data;
    if (NULL == gpio) {
        return -1;
    }
    pikaFree(gpio, sizeof(platform_data_GPIO));
    dev->platform_data = NULL;
    return 0;
}

int pika_hal_platform_GPIO_read(pika_dev* dev, void* buf, size_t count) {
    platform_data_GPIO* data = dev->platform_data;
    //uint32_t level = LL_GPIO_IsInputPinSet(data->gpioPort, data->gpioPin);
    //if (level != 1 && level != 0) {
    //    return -1;
    //}
   // memcpy(buf, &level, count);
    return 0;
}

int pika_hal_platform_GPIO_write(pika_dev* dev, void* buf, size_t count) {
    platform_data_GPIO* data = dev->platform_data;
    gpio_write_pin(HPM_GPIO0, GPIO_DO_GPIOA, 4, *((uint32_t*)buf));
    return 0;
}

int pika_hal_platform_GPIO_ioctl_enable(pika_dev* dev) {
    platform_data_GPIO* data = dev->platform_data;

    //if (0 != _enable_gpio_clk(data->gpioPort)) {
    //    return -1;
    //}

    ///*Configure GPIO pin Output Level */
    //LL_GPIO_ResetOutputPin(data->gpioPort, data->gpioPin);

    //LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    ///*Configure GPIO*/
    //GPIO_InitStruct.Pin = data->gpioPin;
    //GPIO_InitStruct.Mode = data->pinMode;
    //GPIO_InitStruct.Pull = data->gpioPull;
    //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //LL_GPIO_Init(data->gpioPort, &GPIO_InitStruct);
    return 0;
}

int pika_hal_platform_GPIO_ioctl_disable(pika_dev* dev) {
    platform_data_GPIO* data = dev->platform_data;
    //LL_GPIO_DeInit(data->gpioPort);
    return 0;
}

int pika_hal_platform_GPIO_ioctl_config(pika_dev* dev,
                                        pika_hal_GPIO_config* cfg) {
    platform_data_GPIO* data = dev->platform_data;
    //switch (cfg->dir) {
    //    case PIKA_HAL_GPIO_DIR_IN:
    //        data->pinMode = LL_GPIO_MODE_INPUT;
    //        break;
    //    case PIKA_HAL_GPIO_DIR_OUT:
    //        data->pinMode = LL_GPIO_MODE_OUTPUT;
    //        break;
    //    default:
    //        data->pinMode = LL_GPIO_MODE_OUTPUT;
    //}

    //LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    ///*Configure GPIO*/
    //GPIO_InitStruct.Pin = data->gpioPin;
    //GPIO_InitStruct.Mode = data->pinMode;
    //GPIO_InitStruct.Pull = data->gpioPull;
    //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //LL_GPIO_Init(data->gpioPort, &GPIO_InitStruct);
    return 0;
}