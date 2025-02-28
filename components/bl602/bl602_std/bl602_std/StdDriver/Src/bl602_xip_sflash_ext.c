/**
  ******************************************************************************
  * @file    bl602_xip_sflash_ext.c
  * @version V1.0
  * @date
  * @brief   This file is the standard driver c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2020 Bouffalo Lab</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Bouffalo Lab nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "string.h"
#include "bl602_xip_sflash_ext.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  XIP_SFLASH
 *  @{
 */

/** @defgroup  XIP_SFLASH_EXT_Private_Macros
 *  @{
 */

/*@} end of group XIP_SFLASH_EXT_Private_Macros */

/** @defgroup  XIP_SFLASH_EXT_Private_Types
 *  @{
 */

/*@} end of group XIP_SFLASH_EXT_Private_Types */

/** @defgroup  XIP_SFLASH_EXT_Private_Variables
 *  @{
 */

static SPI_Flash_Cfg_Type flashCfg;
static uint8_t aesEnable;

/*@} end of group XIP_SFLASH_EXT_Private_Variables */

/** @defgroup  XIP_SFLASH_EXT_Global_Variables
 *  @{
 */

/*@} end of group XIP_SFLASH_EXT_Global_Variables */

/** @defgroup  XIP_SFLASH_EXT_Private_Fun_Declaration
 *  @{
 */

/*@} end of group XIP_SFLASH_EXT_Private_Fun_Declaration */

/** @defgroup  XIP_SFLASH_EXT_Private_Functions
 *  @{
 */

/****************************************************************************//**
 * @brief  Read data from flash with lock
 *
 * @param  pFlashCfg:Flash config pointer
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_TCM_SECTION XIP_SFlash_Init(SPI_Flash_Cfg_Type *pFlashCfg)
{
    uint32_t ret;

    if(pFlashCfg==NULL){
        /* Get flash config identify */
        XIP_SFlash_Opt_Enter(&aesEnable);
        ret=SF_Cfg_Flash_Identify_Ext(1,1,0,0,&flashCfg);
        XIP_SFlash_Opt_Exit(aesEnable);
        if((ret&BFLB_FLASH_ID_VALID_FLAG)==0){
            return ERROR;
        }
    }else{
        memcpy(&flashCfg,pFlashCfg,sizeof(flashCfg));
    }
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Read data from flash with lock
 *
 * @param  addr: flash read start address
 * @param  dst: data pointer to store data read from flash
 * @param  len: data length to read
 *
 * @return 0
 *
*******************************************************************************/
__WEAK
int ATTR_TCM_SECTION XIP_SFlash_Read(uint32_t addr, uint8_t *dst, int len)
{
    __disable_irq();
    XIP_SFlash_Opt_Enter(&aesEnable);
    XIP_SFlash_Read_Need_Lock(&flashCfg, addr, dst, len);
    XIP_SFlash_Opt_Exit(aesEnable);
    __enable_irq();
    return 0;
}

/****************************************************************************//**
 * @brief  Program flash one region with lock
 *
 * @param  addr: Start address to be programed
 * @param  src: Data pointer to be programed
 * @param  len: Data length to be programed
 *
 * @return 0
 *
*******************************************************************************/
__WEAK
int ATTR_TCM_SECTION XIP_SFlash_Write(uint32_t addr, uint8_t *src, int len)
{
    __disable_irq();
    XIP_SFlash_Opt_Enter(&aesEnable);
    XIP_SFlash_Write_Need_Lock(&flashCfg, addr, src, len);
    XIP_SFlash_Opt_Exit(aesEnable);
    __enable_irq();
    return 0;
}

/****************************************************************************//**
 * @brief  Erase flash one region with lock
 *
 * @param  addr: Start address to be erased
 * @param  len: Data length to be erased
 *
 * @return 0
 *
*******************************************************************************/
__WEAK
int ATTR_TCM_SECTION XIP_SFlash_Erase(uint32_t addr, int len)
{
    __disable_irq();
    XIP_SFlash_Opt_Enter(&aesEnable);
    XIP_SFlash_Erase_Need_Lock(&flashCfg, addr, addr + len - 1);
    XIP_SFlash_Opt_Exit(aesEnable);
    __enable_irq();
    return 0;
}

/****************************************************************************//**
 * @brief  Sflash enable RCV mode to recovery for erase while power drop need lock
 *
 * @param  pFlashCfg: Flash config pointer
 * @param  rCmd: Read RCV register cmd
 * @param  wCmd: Write RCV register cmd
 * @param  bitPos: RCV register bit pos
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_TCM_SECTION XIP_SFlash_RCV_Enable_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg,
    uint8_t rCmd, uint8_t wCmd, uint8_t bitPos)
{
    BL_Err_Type stat;
    uint32_t offset;

    stat=XIP_SFlash_State_Save(pFlashCfg,&offset);
    if(stat!=SUCCESS){
        SFlash_Set_IDbus_Cfg(pFlashCfg,SF_CTRL_QIO_MODE,1,0,32);
    }else{
        stat=SFlash_RCV_Enable(pFlashCfg, rCmd, wCmd, bitPos);
        XIP_SFlash_State_Restore(pFlashCfg,offset);
    }

    return stat;
}


/****************************************************************************//**
 * @brief  Sflash enable RCV mode to recovery for erase while power drop with lock
 *
 * @param  pFlashCfg: Flash config pointer
 * @param  rCmd: Read RCV register cmd
 * @param  wCmd: Write RCV register cmd
 * @param  bitPos: RCV register bit pos
 *
 * @return 0
 *
*******************************************************************************/
__WEAK
int ATTR_TCM_SECTION XIP_SFlash_RCV_Enable_With_Lock(SPI_Flash_Cfg_Type *pFlashCfg,
    uint8_t rCmd, uint8_t wCmd, uint8_t bitPos)
{
    __disable_irq();
    XIP_SFlash_RCV_Enable_Need_Lock(pFlashCfg, rCmd, wCmd, bitPos);
    __enable_irq();
    return 0;
}

/*@} end of group XIP_SFLASH_EXT_Public_Functions */

/*@} end of group XIP_SFLASH_EXT */

/*@} end of group BL602_Peripheral_Driver */
