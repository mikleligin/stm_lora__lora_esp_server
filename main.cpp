#include "stm32f4xx.h"
#include <string>

const int DATA_Size=128; // ?????? ???????????? ??????
char DATA_Buffer[DATA_Size]; // ??????????? ?????
int DATA_Head=0; // ??????? ??? ????????? ??????
int DATA_Tail=0; // ??????? ??? ?????????? ??????
int data = 0;
char gas[6];
int cnt = 0;
int gas_buffer[6];
void SetAltFunc(GPIO_TypeDef* Port, int Channel, int AF)
{
 Port->MODER &= ~(3<<(2*Channel)); // Сброс режима
 Port->MODER |= 2<<(2*Channel); // Установка альт. Режима
 if(Channel<8) // Выбор регистра зависит от номера контакта
 {
 Port->AFR[0] &= ~(15<<4*Channel); // Сброс альт. функции
 Port->AFR[0] |= AF<<(4*Channel); // Установка альт. функции
 }
 else
 {
 Port->AFR[1] &= ~(15<<4*(Channel-8)); // Сброс альт. функции
 Port->AFR[1] |= AF<<(4*(Channel-8)); // Установка альт. функции
 }
}

int UART2_Recv(char* Data, int Size, bool WaitAll=false) // Функция приёма байт
{
 int size; // Размер принятых данных
 for(size=0; size<Size; size++) // Цикл приёма данных с учётом допустимого размера
 {
 if(WaitAll) while(DATA_Tail==DATA_Head) { } // Ждать прихода данных
 else if(DATA_Tail==DATA_Head) break; // Данных больше нет, выходим из цикла
 Data[size]=DATA_Buffer[DATA_Tail++]; // Читаем байт и задаём след. позицию
 if(DATA_Tail>=DATA_Size) DATA_Tail=0; // Превышение размера, идём сначала
 }
 return size; // Вернуть размер полученных данных
}

extern "C" void USART2_IRQHandler() // Функция обработки прерывания
{
 int test=USART2->SR; // Читаем SR чтобы обработать прерывание
 DATA_Buffer[DATA_Head++]=USART2->DR; // Читаем данные и задаём след. позицию
 if(DATA_Head>=DATA_Size) DATA_Head=0; // Начинаем с начала если превысили размер
}

int UART2_GetString(char* Data, int Size) // Функция приёма строки
{
 int size; // Размер принятых данных
 for(size=0; size<(Size-1); size++) // Цикл приёма данных с учётом допустимого размера
 {
 while(DATA_Tail==DATA_Head); // Ждать прихода данных
 Data[size]=DATA_Buffer[DATA_Tail++]; // Читаем байт и задаём след. позицию
 if(DATA_Tail>=DATA_Size) DATA_Tail=0; // Превышение размера, идём сначала
 if(Data[size]=='\n') { size++; break; } // Обнаружить новую строку
 }
 Data[size]='\0'; // Установить конец строки
 return size; // Вернуть размер полученных данных
}

void UART2_Send(char* Data, int Size) // Функция передачи байт
{
 while(Size--) // Цикл передачи данных
 {
 while(!(USART2->SR & USART_SR_TXE)) { } // Ждать возможности передавать
 USART2->DR=*Data++; // Передать данные и задать след. позицию
 }
 while(!(USART2->SR & USART_SR_TC)) { } // Ждать завершения передачи
}
int AnalogRead(int N) // Функция принимает номер канала для преобразования
{
ADC1->SQR3 = N; // Выбран полученный из аргумента канал
for(int a=0; a<100; a++) { asm("NOP"); } // Ожидать больше 100 тактов
ADC1->CR2 |= ADC_CR2_SWSTART; // Начать преобразование
while(!(ADC1->SR & ADC_SR_EOC)) { asm("NOP"); } // Ждать установки бита конца операции
return ADC1->DR; // Вернуть результат преобразования
}
int main()
{
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // ADC задействован
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Порт C задействован
  GPIOA->MODER &= ~GPIO_MODER_MODER4; // Сброс режима для PC3
  GPIOA->MODER |= GPIO_MODER_MODER4; // Аналоговый вход для PC3
  ADC1->CR2 = ADC_CR2_ADON; // ADC активен
  ADC1->SQR3 = 4; // Выбран 13 канал
  for(int a=0; a<20; a++) { } // Ожидать больше 20 тактов
  ADC1->CR2 |= ADC_CR2_SWSTART; // Начать преобразование
  while(!(ADC1->SR & ADC_SR_EOC)) { } // Ждать установки бита конца операции
  
  
  
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // UART 2 задействован (APB1=42МГц)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  SetAltFunc(GPIOA, 2, 7); // Установка альт. режима AF7 для TX(PA2) (см. лаб. 2)
  SetAltFunc(GPIOA, 3, 7); // Установка альт. режима AF7 для RX(PA3) (см. лаб. 2)
  USART2->BRR = (16000000)/9600; // Делитель установлен на 9600
  USART2->CR1 = USART_CR1_RE | USART_CR1_TE; // Приём и передача активны
  USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_UE; // Прерывание на приём и запуск устройства
  NVIC_SetPriority(USART2_IRQn, 0); // Высший приоритет прерывания
  NVIC_EnableIRQ(USART2_IRQn); // Прерывание активно
  while(1)
  {
    //gas[0] = AnalogRead(4);
    if(cnt>5){cnt=0;}
    gas_buffer[cnt] = AnalogRead(4);
    cnt++;
    if((gas_buffer[5]-gas_buffer[0])>10 && gas_buffer[5]!=0){
      data = AnalogRead(4);
      sprintf(gas, "%s \n", "!gaslogger");
      UART2_Send(gas, strlen(gas));
    }
    for(int i = 0; i<1000000; i++){}
  }
  return 0;
}
