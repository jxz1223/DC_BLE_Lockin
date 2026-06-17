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
#include <string.h>

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

typedef struct
{
  uint8_t Active;
  uint8_t BeginSent;
  uint8_t AbortRequested;
  uint16_t ScanId;
  uint16_t Seq;
  uint16_t DacMin;
  uint16_t DacMax;
  uint16_t TotalPoints;
  uint16_t NextPoint;
  uint16_t CompletedPoints;
  uint16_t SettleMs;
  uint8_t AvgBlocks;
  uint16_t PreScanDac;
} Sensor_ScanContext_t;
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
#define SENSOR_LEGACY_STREAM_ENABLED    0U

#define SENSOR_PROTOCOL_MAGIC_0         0xA5U
#define SENSOR_PROTOCOL_MAGIC_1         0x5AU
#define SENSOR_PROTOCOL_VERSION         0x01U
#define SENSOR_PROTOCOL_HEADER_LEN      8U
#define SENSOR_PROTOCOL_MAX_PAYLOAD     234U
#define SENSOR_PROTOCOL_MAX_FRAME       246U

#define SENSOR_CMD_SCAN_START           0x10U
#define SENSOR_CMD_SET_DAC              0x11U
#define SENSOR_CMD_ABORT                0x12U

#define SENSOR_EVT_ACK                  0x80U
#define SENSOR_EVT_NACK                 0x81U
#define SENSOR_EVT_SCAN_BEGIN           0x90U
#define SENSOR_EVT_SCAN_POINTS          0x91U
#define SENSOR_EVT_SCAN_END             0x92U
#define SENSOR_EVT_SENSOR_READINGS      0x93U
#define SENSOR_EVT_BLE_RAW              0xA0U

#define SENSOR_STATUS_OK                0x00U
#define SENSOR_STATUS_CRC_ERROR         0x01U
#define SENSOR_STATUS_BAD_VERSION       0x02U
#define SENSOR_STATUS_BAD_LENGTH        0x03U
#define SENSOR_STATUS_UNKNOWN_TYPE      0x04U
#define SENSOR_STATUS_BUSY              0x06U

#define SENSOR_SET_FLAG_SAVE_DAC        0x01U
#define SENSOR_SCAN_POINTS_PER_FRAME    10U
#define SENSOR_SCAN_DEFAULT_SETTLE_MS   20U
#define SENSOR_SCAN_MAX_SETTLE_MS       80U
#define SENSOR_SCAN_MAX_AVG_BLOCKS      16U
#define SENSOR_READING_ITEMS_PER_FRAME  1U
#define SENSOR_READING_ITEM_SIZE        8U

#define SENSOR_DAC_STORE_MAGIC          0xDAC05A5AU
#define SENSOR_APP_FLASH_SIZE           (512U * 1024U)
#define SENSOR_DAC_STORE_ADDRESS        (FLASH_BASE + SENSOR_APP_FLASH_SIZE - FLASH_PAGE_SIZE)
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
#define LOCKIN_REF_TABLE_SIZE                 8
#define LOCKIN_Q15_SCALE                      32768
#define LOCKIN_ADC_MIDPOINT                   2048
static const int16_t LockinSinQ15[LOCKIN_REF_TABLE_SIZE] = {0, 23170, 32767, 23170, 0, -23170, -32767, -23170};
static const int16_t LockinCosQ15[LOCKIN_REF_TABLE_SIZE] = {32767, 23170, 0, -23170, -32767, -23170, 0, 23170};
uint32_t lockin_sample_index = 0;
uint16_t lockin_rms_value = 0;
static Sensor_ScanContext_t SensorScan;
static uint16_t SensorCurrentDac = 0U;
static uint8_t SensorNotifyReady = 0U;
static uint8_t SensorTxFrame[SENSOR_PROTOCOL_MAX_FRAME];
static uint16_t SensorReadingSeq = 0U;
static uint16_t SensorLatestAdcSample = 0U;
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
static uint16_t Lockin_CalcRms(const uint16_t *pSamples, uint16_t length);
static int32_t Lockin_GetCenteredSample(uint16_t adc_sample);
static void Lockin_FillRmsFrame(uint16_t rms_value);
static uint64_t IntSqrt64(uint64_t value);
static void Sensor_Command_Received(const uint8_t *pPayload, uint8_t length);
static void Sensor_Scan_Task(void);
static uint8_t Sensor_ParseFrame(const uint8_t *data, uint8_t length);
static uint8_t Sensor_HandleCommand(uint8_t type, uint8_t flags, uint16_t scan_id, uint16_t seq, const uint8_t *payload, uint8_t payload_len);
static tBleStatus Sensor_SendFrame(uint8_t type, uint8_t flags, uint16_t scan_id, uint16_t seq, const uint8_t *payload, uint8_t payload_len);
static void Sensor_SendDiag(const char *text);
static void Sensor_SendAck(uint8_t type, uint16_t scan_id, uint16_t seq, uint8_t status);
static void Sensor_SendNack(uint8_t type, uint16_t scan_id, uint16_t seq, uint8_t status);
static tBleStatus Sensor_SendReadingFrame(void);
static uint16_t Sensor_Crc16Ccitt(const uint8_t *data, uint16_t length);
static uint16_t Sensor_ReadLe16(const uint8_t *payload);
static void Sensor_WriteLe16(uint8_t *payload, uint16_t value);
static uint16_t Sensor_InterpolateDac(uint16_t index);
static uint16_t Sensor_ReadLockinAverage(uint8_t avg_blocks);
static void Sensor_DacApply(uint16_t dac_code);
static uint16_t Sensor_DacLoadSaved(uint16_t fallback);
static uint8_t Sensor_DacSave(uint16_t dac_code);
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
      SensorNotifyReady = 1U;
      BSP_LED_On(LED_GREEN);
      Sensor_SendDiag("SENSOR_NOTIFY_ENABLED");
    	UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
      /* USER CODE END CUSTOM_STM_TX_CHAR_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_TX_CHAR_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TX_CHAR_NOTIFY_DISABLED_EVT */
    	Custom_App_Context.Tx_char_Notification_Status = 0;
      SensorNotifyReady = 0U;
      BSP_LED_Off(LED_GREEN);
      /* USER CODE END CUSTOM_STM_TX_CHAR_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_RX_CHAR_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RX_CHAR_WRITE_EVT */
      BSP_LED_Toggle(LED_BLUE);
      Sensor_Command_Received(pNotification->DataTransfered.pPayload,
                              pNotification->DataTransfered.Length);
      /* USER CODE END CUSTOM_STM_RX_CHAR_WRITE_EVT */
      break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */
      UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
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
      BSP_LED_On(LED_BLUE);
      BSP_LED_Off(LED_RED);
      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */
      SensorNotifyReady = 0U;
      Custom_App_Context.Tx_char_Notification_Status = 0U;
      BSP_LED_Off(LED_BLUE);
      BSP_LED_Off(LED_GREEN);
      BSP_LED_On(LED_RED);
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
	SensorCurrentDac = Sensor_DacLoadSaved(0x0000U);
	Sensor_DacApply(SensorCurrentDac);//MAX is 0xFFFF
	DDS_START(1000);
	MM1205();
	ADC_START();
	UTIL_SEQ_RegTask( 1<<CFG_TASK_DATA_TRANSFER_UPDATE_ID, UTIL_SEQ_RFU, SendData);
	UTIL_SEQ_RegTask(1U << CFG_TASK_DAC_SCAN_ID, UTIL_SEQ_RFU, Sensor_Scan_Task);
	Custom_App_Context.Tx_char_Notification_Status = 0;
  SensorNotifyReady = 0U;
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
#if (SENSOR_LEGACY_STREAM_ENABLED != 0U)
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
#endif
  if ((SensorNotifyReady != 0U) &&
      (Custom_App_Context.Tx_char_Notification_Status == 1U) &&
      (buffer1 == 1U))
  {
    tBleStatus status;

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

    status = Sensor_SendReadingFrame();
    if (status == BLE_STATUS_SUCCESS)
    {
      buffer1 = 0U;
    }
    else if (status == BLE_STATUS_INSUFFICIENT_RESOURCES)
    {
      Custom_App_Context.Tx_char_Notification_Status = 0U;
    }
  }
  return;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
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
			lockin_rms_value = Lockin_CalcRms((const uint16_t *)aADCxConvertedData, ADC_CONVERTED_DATA_BUFFER_SIZE);
			SensorLatestAdcSample = aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE - 1U];
			Lockin_FillRmsFrame(lockin_rms_value);
			buffer1 = 1;
			UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_ID, CFG_SCH_PRIO_0);
		}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  /* Update status variable of DMA transfer */
	//Error_Handler();
  ubDmaTransferStatus = 0;
}

static uint16_t Lockin_CalcRms(const uint16_t *pSamples, uint16_t length)
{
  int64_t sum_sin_q15 = 0;
  int64_t sum_cos_q15 = 0;
  uint16_t i;

  if (length == 0U)
  {
    return 0;
  }

  for (i = 0U; i < length; i++)
  {
    uint8_t ref_index = (uint8_t)((lockin_sample_index + i) & (LOCKIN_REF_TABLE_SIZE - 1U));
    int32_t sample = Lockin_GetCenteredSample(pSamples[i]);

    sum_sin_q15 += (int64_t)sample * LockinSinQ15[ref_index];
    sum_cos_q15 += (int64_t)sample * LockinCosQ15[ref_index];
  }

  lockin_sample_index = (lockin_sample_index + length) & (LOCKIN_REF_TABLE_SIZE - 1U);

  /*
   * For x=A*sin(wt+phi), the lock-in DC terms are A/2 on I/Q.
   * RMS = A/sqrt(2) = sqrt(2 * (I^2 + Q^2)).
   */
  int64_t mean_sin_q15 = sum_sin_q15 / length;
  int64_t mean_cos_q15 = sum_cos_q15 / length;
  uint64_t power_q30 = 2ULL * ((uint64_t)(mean_sin_q15 * mean_sin_q15) +
                               (uint64_t)(mean_cos_q15 * mean_cos_q15));
  uint32_t rms = (uint32_t)((IntSqrt64(power_q30) + (LOCKIN_Q15_SCALE / 2U)) /
                            LOCKIN_Q15_SCALE);

  if (rms > 0xFFFFU)
  {
    rms = 0xFFFFU;
  }

  return (uint16_t)rms;
}

static int32_t Lockin_GetCenteredSample(uint16_t adc_sample)
{
  return (int32_t)adc_sample - (int32_t)LOCKIN_ADC_MIDPOINT;
}

static void Lockin_FillRmsFrame(uint16_t rms_value)
{
  uint16_t i;

  NotifyCharData[0] = (uint8_t)(0x8F);
  NotifyCharData[1] = (uint8_t)(0x8E);
  NotifyCharData[2] = (uint8_t)(0x8F);

  for (i = 0U; i < ADC_CONVERTED_DATA_BUFFER_SIZE; i++)
  {
    NotifyCharData[i * 2U + 3U] = (uint8_t)(rms_value >> 8);
    NotifyCharData[i * 2U + 4U] = (uint8_t)(rms_value);
  }

  NotifyCharData[243] = (uint8_t)(0x8E);
  NotifyCharData[244] = (uint8_t)(0x8F);
  NotifyCharData[245] = (uint8_t)(0x8E);
}

static uint64_t IntSqrt64(uint64_t value)
{
  uint64_t bit = 1ULL << 62;
  uint64_t result = 0;

  while (bit > value)
  {
    bit >>= 2;
  }

  while (bit != 0U)
  {
    if (value >= result + bit)
    {
      value -= result + bit;
      result = (result >> 1) + bit;
    }
    else
    {
      result >>= 1;
    }
    bit >>= 2;
  }

  return result;
}

static void Sensor_Command_Received(const uint8_t *pPayload, uint8_t length)
{
  uint8_t status;

  if ((pPayload == 0) || (length == 0U))
  {
    Sensor_SendNack(0U, 0U, 0U, SENSOR_STATUS_BAD_LENGTH);
    return;
  }

  status = Sensor_ParseFrame(pPayload, length);
  if (status != SENSOR_STATUS_OK)
  {
    Sensor_SendNack(0U, 0U, 0U, status);
  }
}

static uint8_t Sensor_ParseFrame(const uint8_t *data, uint8_t length)
{
  uint8_t type;
  uint8_t flags;
  uint16_t scan_id;
  uint16_t seq;
  uint8_t payload_len;
  uint16_t received_crc;
  uint16_t calc_crc;

  if (length < (2U + SENSOR_PROTOCOL_HEADER_LEN + 2U))
  {
    return SENSOR_STATUS_BAD_LENGTH;
  }

  if ((data[0] != SENSOR_PROTOCOL_MAGIC_0) || (data[1] != SENSOR_PROTOCOL_MAGIC_1))
  {
    return SENSOR_STATUS_BAD_LENGTH;
  }

  if (data[2] != SENSOR_PROTOCOL_VERSION)
  {
    return SENSOR_STATUS_BAD_VERSION;
  }

  type = data[3];
  flags = data[4];
  scan_id = Sensor_ReadLe16(&data[5]);
  seq = Sensor_ReadLe16(&data[7]);
  payload_len = data[9];

  if ((payload_len > SENSOR_PROTOCOL_MAX_PAYLOAD) ||
      (length != (uint8_t)(2U + SENSOR_PROTOCOL_HEADER_LEN + payload_len + 2U)))
  {
    return SENSOR_STATUS_BAD_LENGTH;
  }

  received_crc = Sensor_ReadLe16(&data[10U + payload_len]);
  calc_crc = Sensor_Crc16Ccitt(&data[2], (uint16_t)(SENSOR_PROTOCOL_HEADER_LEN + payload_len));
  if (received_crc != calc_crc)
  {
    return SENSOR_STATUS_CRC_ERROR;
  }

  return Sensor_HandleCommand(type, flags, scan_id, seq, &data[10], payload_len);
}

static uint8_t Sensor_HandleCommand(uint8_t type, uint8_t flags, uint16_t scan_id, uint16_t seq, const uint8_t *payload, uint8_t payload_len)
{
  uint16_t dac_code;

  switch (type)
  {
    case SENSOR_CMD_SCAN_START:
      if (payload_len < 9U)
      {
        BSP_LED_On(LED_RED);
        Sensor_SendNack(type, scan_id, seq, SENSOR_STATUS_BAD_LENGTH);
        return SENSOR_STATUS_OK;
      }
      if (SensorScan.Active != 0U)
      {
        BSP_LED_Toggle(LED_RED);
        Sensor_SendNack(type, scan_id, seq, SENSOR_STATUS_BUSY);
        return SENSOR_STATUS_OK;
      }

      SensorScan.DacMin = Sensor_ReadLe16(&payload[0]);
      SensorScan.DacMax = Sensor_ReadLe16(&payload[2]);
      SensorScan.TotalPoints = Sensor_ReadLe16(&payload[4]);
      SensorScan.SettleMs = Sensor_ReadLe16(&payload[6]);
      SensorScan.AvgBlocks = payload[8];

      if ((SensorScan.TotalPoints < 2U) || (SensorScan.TotalPoints > 255U))
      {
        BSP_LED_On(LED_RED);
        Sensor_SendNack(type, scan_id, seq, SENSOR_STATUS_BAD_LENGTH);
        return SENSOR_STATUS_OK;
      }

      if (SensorScan.SettleMs == 0U)
      {
        SensorScan.SettleMs = SENSOR_SCAN_DEFAULT_SETTLE_MS;
      }
      if (SensorScan.SettleMs > SENSOR_SCAN_MAX_SETTLE_MS)
      {
        SensorScan.SettleMs = SENSOR_SCAN_MAX_SETTLE_MS;
      }
      if (SensorScan.AvgBlocks == 0U)
      {
        SensorScan.AvgBlocks = 1U;
      }
      if (SensorScan.AvgBlocks > SENSOR_SCAN_MAX_AVG_BLOCKS)
      {
        SensorScan.AvgBlocks = SENSOR_SCAN_MAX_AVG_BLOCKS;
      }

      SensorScan.Active = 1U;
      SensorScan.BeginSent = 0U;
      SensorScan.AbortRequested = 0U;
      SensorScan.ScanId = scan_id;
      SensorScan.Seq = seq;
      SensorScan.NextPoint = 0U;
      SensorScan.CompletedPoints = 0U;
      SensorScan.PreScanDac = SensorCurrentDac;

      Sensor_SendAck(type, scan_id, seq, SENSOR_STATUS_OK);
      BSP_LED_On(LED_BLUE);
      Sensor_SendDiag("SENSOR_SCAN_START");
      UTIL_SEQ_SetTask(1U << CFG_TASK_DAC_SCAN_ID, CFG_SCH_PRIO_0);
      return SENSOR_STATUS_OK;

    case SENSOR_CMD_SET_DAC:
      if (payload_len < 2U)
      {
        BSP_LED_On(LED_RED);
        Sensor_SendNack(type, scan_id, seq, SENSOR_STATUS_BAD_LENGTH);
        return SENSOR_STATUS_OK;
      }

      dac_code = Sensor_ReadLe16(&payload[0]);
      Sensor_DacApply(dac_code);
      (void)flags;
      if (Sensor_DacSave(dac_code) == 0U)
      {
        BSP_LED_On(LED_RED);
        Sensor_SendNack(type, scan_id, seq, SENSOR_STATUS_BUSY);
        Sensor_SendDiag("SENSOR_SET_DAC_SAVE_FAIL");
      }
      else
      {
        BSP_LED_Toggle(LED_GREEN);
        Sensor_SendAck(type, scan_id, seq, SENSOR_STATUS_OK);
        Sensor_SendDiag("SENSOR_SET_DAC_OK");
      }
      return SENSOR_STATUS_OK;

    case SENSOR_CMD_ABORT:
      SensorScan.AbortRequested = 1U;
      BSP_LED_Toggle(LED_RED);
      Sensor_SendAck(type, scan_id, seq, SENSOR_STATUS_OK);
      Sensor_SendDiag("SENSOR_ABORT");
      UTIL_SEQ_SetTask(1U << CFG_TASK_DAC_SCAN_ID, CFG_SCH_PRIO_0);
      return SENSOR_STATUS_OK;

    default:
      BSP_LED_On(LED_RED);
      Sensor_SendNack(type, scan_id, seq, SENSOR_STATUS_UNKNOWN_TYPE);
      return SENSOR_STATUS_OK;
  }
}

static void Sensor_Scan_Task(void)
{
  uint8_t payload[1U + (SENSOR_SCAN_POINTS_PER_FRAME * 5U)];
  uint8_t count;
  uint8_t offset;
  uint16_t dac_code;
  uint16_t lockin_value;

  if ((SensorNotifyReady == 0U) || (Custom_App_Context.Tx_char_Notification_Status == 0U))
  {
    if (SensorScan.Active != 0U)
    {
      UTIL_SEQ_SetTask(1U << CFG_TASK_DAC_SCAN_ID, CFG_SCH_PRIO_0);
    }
    return;
  }

  if (SensorScan.Active == 0U)
  {
    return;
  }

  if (SensorScan.BeginSent == 0U)
  {
    uint8_t begin_payload[11];
    Sensor_WriteLe16(&begin_payload[0], SensorScan.DacMin);
    Sensor_WriteLe16(&begin_payload[2], SensorScan.DacMax);
    Sensor_WriteLe16(&begin_payload[4], SensorScan.TotalPoints);
    Sensor_WriteLe16(&begin_payload[6], SensorScan.SettleMs);
    begin_payload[8] = SensorScan.AvgBlocks;
    Sensor_WriteLe16(&begin_payload[9], SensorScan.PreScanDac);

    if (Sensor_SendFrame(SENSOR_EVT_SCAN_BEGIN, 0U, SensorScan.ScanId, SensorScan.Seq, begin_payload, sizeof(begin_payload)) == BLE_STATUS_SUCCESS)
    {
      SensorScan.BeginSent = 1U;
      Sensor_SendDiag("SENSOR_SCAN_BEGIN_SENT");
    }
    UTIL_SEQ_SetTask(1U << CFG_TASK_DAC_SCAN_ID, CFG_SCH_PRIO_0);
    return;
  }

  if (SensorScan.AbortRequested != 0U)
  {
    uint8_t end_payload[5];
    Sensor_DacApply(SensorScan.PreScanDac);
    SensorScan.Active = 0U;
    SensorScan.AbortRequested = 0U;
    end_payload[0] = 1U;
    Sensor_WriteLe16(&end_payload[1], SensorScan.CompletedPoints);
    Sensor_WriteLe16(&end_payload[3], SensorCurrentDac);
    (void)Sensor_SendFrame(SENSOR_EVT_SCAN_END, 0U, SensorScan.ScanId, SensorScan.Seq, end_payload, sizeof(end_payload));
    BSP_LED_Off(LED_BLUE);
    BSP_LED_Toggle(LED_RED);
    Sensor_SendDiag("SENSOR_SCAN_ABORTED");
    return;
  }

  if (SensorScan.NextPoint >= SensorScan.TotalPoints)
  {
    uint8_t end_payload[5];
    Sensor_DacApply(SensorScan.PreScanDac);
    SensorScan.Active = 0U;
    end_payload[0] = 0U;
    Sensor_WriteLe16(&end_payload[1], SensorScan.CompletedPoints);
    Sensor_WriteLe16(&end_payload[3], SensorCurrentDac);
    (void)Sensor_SendFrame(SENSOR_EVT_SCAN_END, 0U, SensorScan.ScanId, SensorScan.Seq, end_payload, sizeof(end_payload));
    BSP_LED_Off(LED_BLUE);
    BSP_LED_Toggle(LED_GREEN);
    Sensor_SendDiag("SENSOR_SCAN_DONE");
    return;
  }

  count = 0U;
  payload[0] = 0U;
  while ((count < SENSOR_SCAN_POINTS_PER_FRAME) && (SensorScan.NextPoint < SensorScan.TotalPoints))
  {
    dac_code = Sensor_InterpolateDac(SensorScan.NextPoint);
    Sensor_DacApply(dac_code);
    HAL_Delay(SensorScan.SettleMs);
    lockin_value = Sensor_ReadLockinAverage(SensorScan.AvgBlocks);

    offset = (uint8_t)(1U + (count * 5U));
    payload[offset] = (uint8_t)SensorScan.NextPoint;
    Sensor_WriteLe16(&payload[offset + 1U], dac_code);
    Sensor_WriteLe16(&payload[offset + 3U], lockin_value);

    SensorScan.NextPoint++;
    SensorScan.CompletedPoints++;
    count++;
  }

  payload[0] = count;
  if (Sensor_SendFrame(SENSOR_EVT_SCAN_POINTS,
                       0U,
                       SensorScan.ScanId,
                       SensorScan.Seq,
                       payload,
                       (uint8_t)(1U + count * 5U)) == BLE_STATUS_INSUFFICIENT_RESOURCES)
  {
    SensorScan.NextPoint = (uint16_t)(SensorScan.NextPoint - count);
    SensorScan.CompletedPoints = (uint16_t)(SensorScan.CompletedPoints - count);
  }

  UTIL_SEQ_SetTask(1U << CFG_TASK_DAC_SCAN_ID, CFG_SCH_PRIO_0);
}

static tBleStatus Sensor_SendFrame(uint8_t type, uint8_t flags, uint16_t scan_id, uint16_t seq, const uint8_t *payload, uint8_t payload_len)
{
  uint16_t crc;
  uint16_t index = 0U;

  if ((payload_len > SENSOR_PROTOCOL_MAX_PAYLOAD) ||
      ((payload_len > 0U) && (payload == 0)))
  {
    return BLE_STATUS_INVALID_PARAMS;
  }

  SensorTxFrame[index++] = SENSOR_PROTOCOL_MAGIC_0;
  SensorTxFrame[index++] = SENSOR_PROTOCOL_MAGIC_1;
  SensorTxFrame[index++] = SENSOR_PROTOCOL_VERSION;
  SensorTxFrame[index++] = type;
  SensorTxFrame[index++] = flags;
  Sensor_WriteLe16(&SensorTxFrame[index], scan_id);
  index = (uint16_t)(index + 2U);
  Sensor_WriteLe16(&SensorTxFrame[index], seq);
  index = (uint16_t)(index + 2U);
  SensorTxFrame[index++] = payload_len;

  if (payload_len > 0U)
  {
    memcpy(&SensorTxFrame[index], payload, payload_len);
    index = (uint16_t)(index + payload_len);
  }

  crc = Sensor_Crc16Ccitt(&SensorTxFrame[2], (uint16_t)(SENSOR_PROTOCOL_HEADER_LEN + payload_len));
  Sensor_WriteLe16(&SensorTxFrame[index], crc);
  index = (uint16_t)(index + 2U);

  return Custom_STM_App_Update_Char_Variable_Length(CUSTOM_STM_TX_CHAR, SensorTxFrame, (uint8_t)index);
}

static void Sensor_SendDiag(const char *text)
{
  uint16_t length;

  if ((text == 0) || (SensorNotifyReady == 0U) ||
      (Custom_App_Context.Tx_char_Notification_Status == 0U))
  {
    return;
  }

  length = (uint16_t)strlen(text);
  if (length > SENSOR_PROTOCOL_MAX_PAYLOAD)
  {
    length = SENSOR_PROTOCOL_MAX_PAYLOAD;
  }

  (void)Sensor_SendFrame(SENSOR_EVT_BLE_RAW,
                         0U,
                         0U,
                         0U,
                         (const uint8_t *)text,
                         (uint8_t)length);
}

static void Sensor_SendAck(uint8_t type, uint16_t scan_id, uint16_t seq, uint8_t status)
{
  uint8_t payload[2];
  payload[0] = type;
  payload[1] = status;
  (void)Sensor_SendFrame(SENSOR_EVT_ACK, 0U, scan_id, seq, payload, sizeof(payload));
}

static void Sensor_SendNack(uint8_t type, uint16_t scan_id, uint16_t seq, uint8_t status)
{
  uint8_t payload[2];
  payload[0] = type;
  payload[1] = status;
  (void)Sensor_SendFrame(SENSOR_EVT_NACK, 0U, scan_id, seq, payload, sizeof(payload));
}

static tBleStatus Sensor_SendReadingFrame(void)
{
  uint8_t payload[1U + (SENSOR_READING_ITEMS_PER_FRAME * SENSOR_READING_ITEM_SIZE)];
  uint8_t offset = 1U;
  uint16_t seq = SensorReadingSeq++;

  payload[0] = SENSOR_READING_ITEMS_PER_FRAME;
  Sensor_WriteLe16(&payload[offset], seq);
  Sensor_WriteLe16(&payload[offset + 2U], SensorCurrentDac);
  Sensor_WriteLe16(&payload[offset + 4U], lockin_rms_value);
  Sensor_WriteLe16(&payload[offset + 6U], SensorLatestAdcSample);

  return Sensor_SendFrame(SENSOR_EVT_SENSOR_READINGS,
                          0U,
                          0U,
                          seq,
                          payload,
                          sizeof(payload));
}

static uint16_t Sensor_Crc16Ccitt(const uint8_t *data, uint16_t length)
{
  uint16_t crc = 0xFFFFU;
  uint16_t i;
  uint8_t bit;

  for (i = 0U; i < length; i++)
  {
    crc ^= (uint16_t)data[i] << 8;
    for (bit = 0U; bit < 8U; bit++)
    {
      if ((crc & 0x8000U) != 0U)
      {
        crc = (uint16_t)((crc << 1) ^ 0x1021U);
      }
      else
      {
        crc = (uint16_t)(crc << 1);
      }
    }
  }

  return crc;
}

static uint16_t Sensor_ReadLe16(const uint8_t *payload)
{
  return (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
}

static void Sensor_WriteLe16(uint8_t *payload, uint16_t value)
{
  payload[0] = (uint8_t)value;
  payload[1] = (uint8_t)(value >> 8);
}

static uint16_t Sensor_InterpolateDac(uint16_t index)
{
  uint16_t denominator = (uint16_t)(SensorScan.TotalPoints - 1U);
  int32_t delta = (int32_t)SensorScan.DacMax - (int32_t)SensorScan.DacMin;
  uint32_t scaled = (uint32_t)((delta >= 0) ? delta : -delta) * index;
  scaled = (scaled + (denominator / 2U)) / denominator;

  if (delta >= 0)
  {
    return (uint16_t)(SensorScan.DacMin + scaled);
  }

  return (uint16_t)(SensorScan.DacMin - scaled);
}

static uint16_t Sensor_ReadLockinAverage(uint8_t avg_blocks)
{
  uint32_t sum = 0U;
  uint8_t i;

  for (i = 0U; i < avg_blocks; i++)
  {
    sum += lockin_rms_value;
    if (avg_blocks > 1U)
    {
      HAL_Delay(1U);
    }
  }

  return (uint16_t)(sum / avg_blocks);
}

static void Sensor_DacApply(uint16_t dac_code)
{
  DAC_CHANGE(dac_code);
  SensorCurrentDac = dac_code;
}

static uint16_t Sensor_DacLoadSaved(uint16_t fallback)
{
  uint64_t raw = *((const uint64_t *)SENSOR_DAC_STORE_ADDRESS);
  uint32_t magic = (uint32_t)raw;
  uint16_t dac_code = (uint16_t)(raw >> 32);
  uint16_t dac_invert = (uint16_t)(raw >> 48);

  if ((magic == SENSOR_DAC_STORE_MAGIC) && (dac_invert == (uint16_t)~dac_code))
  {
    return dac_code;
  }

  return fallback;
}

static uint8_t Sensor_DacSave(uint16_t dac_code)
{
  FLASH_EraseInitTypeDef erase_init;
  uint32_t page_error = 0U;
  uint64_t raw;
  HAL_StatusTypeDef status;

  raw = (uint64_t)SENSOR_DAC_STORE_MAGIC |
        ((uint64_t)dac_code << 32) |
        ((uint64_t)((uint16_t)~dac_code) << 48);

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.Page = (SENSOR_DAC_STORE_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE;
  erase_init.NbPages = 1U;
  status = HAL_FLASHEx_Erase(&erase_init, &page_error);
  if (status == HAL_OK)
  {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, SENSOR_DAC_STORE_ADDRESS, raw);
  }

  HAL_FLASH_Lock();
  return (status == HAL_OK) ? 1U : 0U;
}

/* USER CODE END FD_LOCAL_FUNCTIONS*/
