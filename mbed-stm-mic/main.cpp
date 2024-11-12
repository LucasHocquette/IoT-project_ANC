#include "AnalogIn.h"
#include "mbed.h"

#define BUFFER_SIZE 64

uint32_t ADC_DMABuffer[BUFFER_SIZE];

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TIM_HandleTypeDef htim3;

static bool MX_GPIO_Init();
static bool MX_ADC1_Init();
static bool MX_DMA_Init();
static bool MX_TIM3_Init();

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {      
	UNUSED(hadc);  // we only use  ADC1
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {       
	UNUSED(hadc);  // we only use  ADC1
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc) {       
	UNUSED(hadc);  // we only use  ADC1
}

// main() runs in its own thread in the OS
int main()
{    
    printf("Starting ...\r\n");

    if (!MX_GPIO_Init()) {
        return 1;
    }

    if (!MX_DMA_Init()) {
        return 1;
    }
    
    if (!MX_ADC1_Init()) {
        return 1;
    }

    if (!MX_TIM3_Init()) {
        return 1;
    }  
    
    HAL_ADC_Start_DMA(&hadc1, ADC_DMABuffer, ((uint32_t)(BUFFER_SIZE)));
    printf("Start DMA!\r\n");
    
    if (HAL_TIM_Base_Start(&htim3) != HAL_OK)
    {        
        printf("ERR: ADC multimode config error\r\n");
        return 1;
    }    

    printf("Main loop!\r\n");
    while (true) {
        printf("KEEP ALIVE!\r\n");
        ThisThread::sleep_for(1s);
    }
}

static bool MX_GPIO_Init() {
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = A0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    return true;
}

static bool MX_DMA_Init()
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    return true;
}

static bool MX_ADC1_Init()
{      
    __HAL_RCC_ADC_CLK_ENABLE();
    #if defined(RCC_CCIPR_ADCSEL)
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);
    #endif

    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef  sConfig = { 0 };

    /** Common config
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE; 
    hadc1.Init.NbrOfDiscConversion = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START; //ADC_EXTERNALTRIG_T3_TRGO;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; //ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        printf("ERR: ADC init error\r\n");
        return false;
    }
    
    /** Configure the ADC multi-mode
    */
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
    {
        printf("ERR: ADC multimode config error\r\n");
        return false;
    }

    /** Configure Regular Channel
    */  
    
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.Channel = ADC_CHANNEL_14;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        printf("ERR: ADC multimode error\r\n");
        return false;
    }

    while (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK);
    return true;
}

static bool MX_TIM3_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1666;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        return false;
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        return false;
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        return false;
    }

    return true;
}
