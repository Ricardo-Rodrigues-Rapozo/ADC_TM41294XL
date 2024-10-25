#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"

uint32_t ui32ADC0Value[1]; // Armazena valores do ADC
volatile uint32_t ui32ADCAvg; // Média das leituras do ADC
uint32_t buffer[200];
uint16_t cont = 0;
//void ADCconfig(void)
//{
//}
int main(void)
{
    // Variáveis para armazenar valores do ADC
    uint32_t FS = 5000; // Frequência de amostragem

    // -------------------------- Configurações de clock e periféricos --------------------------
    // Configura o clock do sistema para 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);

    // Habilita o ADC, GPIO E e o Timer0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // habilita o ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //  habilita a porta E GPIO
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // habilita o timer 0

    // Aguarda até que os periféricos estejam prontos
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    // Configura o pino PE3 como entrada analógica (AIN0)
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    // Configura o Timer0 para gerar um trigger periódico para o ADC
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    uint32_t timerLoad = (SysCtlClockGet() / FS) - 1; // Configura para uma taxa de FS kHz
    TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true); // Permite que o Timer dispare o ADC
    TimerEnable(TIMER0_BASE, TIMER_A);

    // -------------------------- Configurações do ADC --------------------------
    // Configura o Sequenciador 1 para usar o Timer0 como trigger
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_TIMER, 0);

    // Configura as etapas do sequenciador para ler o sinal de AIN0 (PE3)
//    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0);
//    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0);
//    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    // Habilita o Sequenciador 1 e limpa qualquer interrupção pendente
    ADCSequenceEnable(ADC0_BASE, 1);
    ADCIntClear(ADC0_BASE, 1);

    // -------------------------- Loop Principal --------------------------
    while(1)
    {
        // Espera até a conversão do ADC estar completa
        if (ADCIntStatus(ADC0_BASE, 1, false))
        {
            // Limpa a flag de interrupção do ADC e obtém os dados do sequenciador
            ADCIntClear(ADC0_BASE, 1);
            ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

            // Calcula a média das leituras do ADC
            //ui32ADCAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2) / 4;
            buffer[cont] = ui32ADC0Value[0];
            cont++;
            if (cont == 200)
            {cont = 0;}
        }
    }
}
