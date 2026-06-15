/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    custom_app.c
  * @author  MCD Application Team
  * @brief   Custom Example Application (Server)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "custom_app.h"
#include "custom_stm.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  /* DT_SERVICE */
  uint8_t               Tx_char_Notification_Status;
  /* USER CODE BEGIN CUSTOM_APP_Context_t */
  uint8_t               SW1_Status;
  /* USER CODE END CUSTOM_APP_Context_t */

  uint16_t              ConnectionHandle;
} Custom_App_Context_t;

/* USER CODE BEGIN PTD */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SCL_0()		HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin,GPIO_PIN_RESET)
#define SCL_1()		HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin,GPIO_PIN_SET)
#define SDA_0()		HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin,GPIO_PIN_RESET)
#define SDA_1()		HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin,GPIO_PIN_SET)

#define FSYNC_0()		HAL_GPIO_WritePin(FSYNC_GPIO_Port, FSYNC_Pin,GPIO_PIN_RESET)
#define FSYNC_1()		HAL_GPIO_WritePin(FSYNC_GPIO_Port, FSYNC_Pin,GPIO_PIN_SET)

#define SCK_0()		HAL_GPIO_WritePin(SCLK_GPIO_Port, SCLK_Pin,GPIO_PIN_RESET)
#define SCK_1()		HAL_GPIO_WritePin(SCLK_GPIO_Port, SCLK_Pin,GPIO_PIN_SET)

#define DAT_0()		HAL_GPIO_WritePin(SDATA_GPIO_Port, SDATA_Pin,GPIO_PIN_RESET)
#define DAT_1()		HAL_GPIO_WritePin(SDATA_GPIO_Port, SDATA_Pin,GPIO_PIN_SET)

#define TOGGLE_ON                       1
#define TOGGLE_OFF                      0
/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ADC_CONVERTED_DATA_BUFFER_SIZE   ((uint8_t)  120)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[512];
uint8_t NotifyCharData[512];
uint16_t Connection_Handle;
/* USER CODE BEGIN PV */
__IO   uint16_t   aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]; /* ADC group regular conversion data (array of data) */
uint16_t   aADCxConvertedData_Voltage_mVolt[ADC_CONVERTED_DATA_BUFFER_SIZE];  /* Value of voltage calculated from ADC conversion data (unit: mV) (array of data) */
/*汇报DMA转换状�??*/
/* Variable to report status of DMA transfer of ADC group regular conversions */
/*  0: DMA transfer is not completed                                          */
/*  1: DMA transfer is completed                                              */
/*  2: DMA transfer has not yet been started yet (initial state)              */
__IO   uint8_t ubDmaTransferStatus = 2; /* Variable set into DMA interruption callback */
uint16_t kk;
uint16_t Att_Mtu_Exchanged;
uint8_t buffer1 = 0;
uint8_t jj=0;
uint8_t jk=0;
uint8_t jjk=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* DT_SERVICE */
static void Custom_Tx_char_Update_Char(void);
static void Custom_Tx_char_Send_Notification(void);

/* USER CODE BEGIN PFP */
void DDS_START(double Freq);
void DAC_START(void);
void DAC_CHANGE(unsigned int AM);
void ADC_START(void);
void MM1205(void);
static void SendData(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_1 */
    
  /* USER CODE END CUSTOM_STM_App_Notification_1 */
  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* USER CODE END CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* DT_SERVICE */
    case CUSTOM_STM_TX_CHAR_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TX_CHAR_NOTIFY_ENABLED_EVT */
    	Custom_App_Context.Tx_char_Notification_Status = 1;
    	UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
      /* USER CODE END CUSTOM_STM_TX_CHAR_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_TX_CHAR_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TX_CHAR_NOTIFY_DISABLED_EVT */
    	Custom_App_Context.Tx_char_Notification_Status = 0;
      /* USER CODE END CUSTOM_STM_TX_CHAR_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */

      /* USER CODE END CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_STM_App_Notification_default */
    	BSP_LED_On(LED_RED);
      /* USER CODE END CUSTOM_STM_App_Notification_default */
      break;
  }
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_2 */

  /* USER CODE END CUSTOM_STM_App_Notification_2 */
  return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_APP_Notification_1 */

  /* USER CODE END CUSTOM_APP_Notification_1 */

  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

    /* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
    case CUSTOM_CONN_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */
          
      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */
      
      /* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_APP_Notification_default */

      /* USER CODE END CUSTOM_APP_Notification_default */
      break;
  }

  /* USER CODE BEGIN CUSTOM_APP_Notification_2 */

  /* USER CODE END CUSTOM_APP_Notification_2 */

  return;
}

void Custom_APP_Init(void)
{
  /* USER CODE BEGIN CUSTOM_APP_Init */
	DAC_START();
	DAC_CHANGE(0x0000);//MAX is 0xFFFF
	DDS_START(500);
	MM1205();
	ADC_START();
	UTIL_SEQ_RegTask( 1<<CFG_TASK_DATA_TRANSFER_UPDATE_ID, UTIL_SEQ_RFU, SendData);
	Custom_App_Context.Tx_char_Notification_Status = 1;
  /* USER CODE END CUSTOM_APP_Init */
  return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* DT_SERVICE */
__USED void Custom_Tx_char_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Tx_char_UC_1*/

  /* USER CODE END Tx_char_UC_1*/

  if (updateflag != 0)
  {
	Custom_STM_App_Update_Char_Ext(Connection_Handle, CUSTOM_STM_TX_CHAR, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Tx_char_UC_Last*/

  /* USER CODE END Tx_char_UC_Last*/
  return;
}

void Custom_Tx_char_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Tx_char_NS_1*/
  updateflag=Custom_App_Context.Tx_char_Notification_Status;
  /* USER CODE END Tx_char_NS_1*/

  if (updateflag != 0)
  {
	Custom_STM_App_Update_Char_Ext(Connection_Handle, CUSTOM_STM_TX_CHAR, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Tx_char_NS_Last*/

  /* USER CODE END Tx_char_NS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/
void SW1_Button_Action(void)
{
  UTIL_SEQ_SetTask( 1<<CFG_TASK_SW1_BUTTON_PUSHED_ID, CFG_SCH_PRIO_0);

  return;
}

void Resume_Notification(void)
{
	Custom_App_Context.Tx_char_Notification_Status = 1;
	UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
	return;
}

void DAC_WRITE(unsigned int TxData)
{
	unsigned char i;
	SCL_0();HAL_Delay(1);
	for(i = 0; i < 8; i++)
	{
		if(TxData & 0x80)
			SDA_1();
		else
			SDA_0();
		HAL_Delay(1);
		SCL_1();
		HAL_Delay(1);
		SCL_0();
		TxData <<= 1;
	}
	SDA_0();HAL_Delay(1);SCL_1();HAL_Delay(1);SCL_0();
}

void DAC_START(void)
{
	SDA_1();SCL_1();
	HAL_Delay(1);
	/*Write Address*/
	SDA_0();HAL_Delay(1);
	DAC_WRITE(0x98);
	DAC_WRITE(0x40);
	DAC_WRITE(0x00);
	DAC_WRITE(0x00);
	HAL_Delay(1);SCL_1();HAL_Delay(1);SDA_1();
}

void DAC_CHANGE(unsigned int AM)
{
	SDA_0();HAL_Delay(1);
	DAC_WRITE(0x98);
	DAC_WRITE(0x30);
	DAC_WRITE(AM>>8);
	DAC_WRITE(AM);
	HAL_Delay(1);SCL_1();HAL_Delay(1);SDA_1();
}

void AD9833_Write(unsigned int TxData)
{
	unsigned char i;

	SCK_1();
	HAL_Delay(1);
	FSYNC_1();
	HAL_Delay(1);
	FSYNC_0();
	HAL_Delay(1);
	for(i = 0; i < 16; i++)
	{
		if (TxData & 0x8000)
			DAT_1();
		else
			DAT_0();

		HAL_Delay(1);
		SCK_0();
		HAL_Delay(1);
		SCK_1();

		TxData <<= 1;
	}
	FSYNC_1();

}

void DDS_START(double Freq)
{
	int frequence_LSB,frequence_MSB,Phs_data;
	double   frequence_mid,frequence_DATA;
	long int frequence_hex;
	FSYNC_0();SCK_0();DAT_0();
	frequence_mid=268435456/8;//适合25M晶振
	frequence_DATA=Freq;
	frequence_DATA=frequence_DATA/1000000;
	frequence_DATA=frequence_DATA*frequence_mid;
	frequence_hex=frequence_DATA;  //这个frequence_hex的�?�是32位的�????个很大的数字，需要拆分成两个14位进行处理；
	frequence_LSB=frequence_hex; //frequence_hex�????16位�?�给frequence_LSB
	frequence_LSB=frequence_LSB&0x3fff;//去除�????高两位，16位数换去掉高位后变成�????14�????
	frequence_MSB=frequence_hex>>14; //frequence_hex�????16位�?�给frequence_HSB
	frequence_MSB=frequence_MSB&0x3fff;//去除�????高两位，16位数换去掉高位后变成�????14�????

	Phs_data=0|0xC000;	//相位�????
	AD9833_Write(0x0100); //复位AD9833,即RESET位为1
	AD9833_Write(0x2100); //选择数据�????次写入，B28位和RESET位为1

	frequence_LSB=frequence_LSB|0x4000;
	frequence_MSB=frequence_MSB|0x4000;
	 //使用频率寄存�????0输出波形
	AD9833_Write(frequence_LSB); //L14，�?�择频率寄存�????0的低14位数据输�????
	AD9833_Write(frequence_MSB); //H14 频率寄存器的�????14位数据输�????
	AD9833_Write(Phs_data);	//设置相位
	AD9833_Write(0x2000);//输出正弦�????
}

void MM1205(void)
{
	//90V on
	HAL_Delay(100);
	//ON and Off
	//while(1)
	{
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, ME_Pin, GPIO_PIN_SET);
		//HAL_Delay(1000);
		//HAL_GPIO_WritePin(GPIOB, ME_Pin, GPIO_PIN_RESET);
		//HAL_Delay(1000);
	}


	//HAL_Delay(10000);
}


void ADC_START(void)
{
	uint8_t i;
	  for (i=0 ; i<(ADC_CONVERTED_DATA_BUFFER_SIZE*2) ; i++)
	  {
		  NotifyCharData[i] = 2;
	  }
	  NotifyCharData[0] = (uint8_t)(0x8F);
	  NotifyCharData[1] = (uint8_t)(0x8E);
	  NotifyCharData[2] = (uint8_t)(0x8F);
	  NotifyCharData[243] = (uint8_t)(0x8E);
	  NotifyCharData[244] = (uint8_t)(0x8F);
	  NotifyCharData[245] = (uint8_t)(0x8E);
	  for (i=0 ; i<(SizeTx_Char) ; i++)
	  	  {
		  UpdateCharData[i] = i;
	  	  }

		uint32_t tmp_index_adc_converted_data = 0;
		for (tmp_index_adc_converted_data = 0; tmp_index_adc_converted_data < ADC_CONVERTED_DATA_BUFFER_SIZE; tmp_index_adc_converted_data++)
	  {
	    aADCxConvertedData[tmp_index_adc_converted_data] = VAR_CONVERTED_DATA_INIT_VALUE;
	  }
	  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
	  {
	    /* Calibration Error */
	    Error_Handler();
	  }
		if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
	  {
	    /* Counter enable error */
	    Error_Handler();
	  }
	  /*## Start ADC conversions ###############################################*/
	  /* Start ADC group regular conversion with DMA */
	  if (HAL_ADC_Start_DMA(&hadc1,
	                        (uint32_t *)aADCxConvertedData,
	                        ADC_CONVERTED_DATA_BUFFER_SIZE
	                       ) != HAL_OK)
	  {
	    /* ADC conversion start error */
	    Error_Handler();
		}
	return;
}




void SendData( void )
{
	tBleStatus status = BLE_STATUS_INVALID_PARAMS;
  if (Custom_App_Context.Tx_char_Notification_Status == 1 )
  {
		if	(buffer1==1)
		{

			kk=kk+1;
			if (kk==20)
			{
				BSP_LED_On(LED_BLUE);

			}
			if (kk==40)
			{
				BSP_LED_Off(LED_BLUE);

				kk=0;
			}
			status = Custom_STM_App_Update_Char(CUSTOM_STM_TX_CHAR, (uint8_t *)NotifyCharData);
			//if (status != BLE_STATUS_SUCCESS)
			//{
				//BSP_LED_On(LED_RED);
			//}
			buffer1 = 0;
			if (status == BLE_STATUS_INSUFFICIENT_RESOURCES)
			{
				Custom_App_Context.Tx_char_Notification_Status = 0;
			}
			else
			{
				UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
			}

		}
		else
		{
			UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
		}
  }
  return;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	uint32_t i = 0;

	jj=jj+1;
	if (jj>20)
	{
		BSP_LED_On(LED_GREEN);
	}
	if (jj>40)
	{
    BSP_LED_Off(LED_GREEN);
    jj=0;
	}
	/*单定时器结束*/

  /* Computation of ADC conversions raw data to physical values               */
  /* using LL ADC driver helper macro.                                        */
  /* Management of the 2nd half of the buffer */

	//for (tmp_index = (ADC_CONVERTED_DATA_BUFFER_SIZE/2); tmp_index < ADC_CONVERTED_DATA_BUFFER_SIZE; tmp_index++)
  //{
    //aADCxConvertedData_Voltage_mVolt[tmp_index] = __ADC_CALC_DATA_VOLTAGE(VDDA_APPLI, aADCxConvertedData[tmp_index]);
  //}
	  /* Update status variable of DMA transfer */
  ubDmaTransferStatus = 1;
		if	(buffer1==0)
		{
			//for (i=0; i < ADC_CONVERTED_DATA_BUFFER_SIZE; i++)
			for (i = (ADC_CONVERTED_DATA_BUFFER_SIZE/2); i < ADC_CONVERTED_DATA_BUFFER_SIZE; i++)
			{
				NotifyCharData[i*2+3] = (uint8_t)(aADCxConvertedData[i]>>8);
				NotifyCharData[i*2+1+3] = (uint8_t)(aADCxConvertedData[i]);
			}
			/*UPDATE*/
			buffer1 = 1;
		}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	//uint32_t tmp_index = 0;
	uint32_t i = 0;
//
//  /* Computation of ADC conversions raw data to physical values               */
//  /* using LL ADC driver helper macro.                                        */
//  /* Management of the 1st half of the buffer */
  //for (tmp_index = 0; tmp_index < (ADC_CONVERTED_DATA_BUFFER_SIZE/2); tmp_index++)
  //{
    //aADCxConvertedData_Voltage_mVolt[tmp_index] = __ADC_CALC_DATA_VOLTAGE(VDDA_APPLI, aADCxConvertedData[tmp_index]);
  //}
  if	(buffer1==0)
  		{
  			//for (i=0; i < ADC_CONVERTED_DATA_BUFFER_SIZE; i++)
  			for (i =0; i < (ADC_CONVERTED_DATA_BUFFER_SIZE/2); i++)
  			{
  				NotifyCharData[i*2+3] = (uint8_t)(aADCxConvertedData[i]>>8);
  				NotifyCharData[i*2+1+3] = (uint8_t)(aADCxConvertedData[i]);
  				//Notification_Data_Buffer1[i] = aADCxConvertedData[i];
  			}
  		}

  /* Update status variable of DMA transfer */
	//Error_Handler();
  ubDmaTransferStatus = 0;
}


/* USER CODE END FD_LOCAL_FUNCTIONS*/
