/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __AT32_INT_H
#define __AT32_INT_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes ------------------------------------------------------------------*/
#include "sys.h"

/* exported types ------------------------------------------------------------*/
/* exported constants --------------------------------------------------------*/
/* exported macro ------------------------------------------------------------*/
/* exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif

