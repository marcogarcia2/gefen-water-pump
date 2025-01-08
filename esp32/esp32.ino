#include <time.h>

// --- VARIÁVEIS --- //

// Pino de leitura do sensor
const int pin = 34;

// Horários de operação (em segundos desde meia-noite) e 
// const int irrigationTimes[3] = {8 * 3600, 14 * 3600, 19 * 3600}; // 08:00, 14:00, 19:00
const int irrigationTimes[3] = {8 * 3600, 14 * 3600, 19 * 3600}; // 08:00, 14:00, 19:00

// Margem de tempo, 5 minutos antes do funcionamento da bomba
const int offset = 5 * 60;

// Variável de tempo atual
tm timeinfo;

// Índice persistente entre reboots
RTC_DATA_ATTR int timePointer = 0;


// --- PROGRAMA PRINCIPAL --- //

void setup() {

  Serial.begin(115200);

  // Conecta ao WiFi para sincronizar o relógio
  connectWiFi();
  configTime(-3 * 3600, 0, "pool.ntp.org"); // Configura NTP com fuso horário de Brasília (-3 horas)
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Erro ao obter horário");
    deepSleepUntilMidnight();
  }

  // Descobrindo o horário atual
  int currentTime = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

  // Com base no horário atual, existem três opções:

  // 1. Já foram todas as irrigações
  if (currentTime > irrigationTimes[2]){
    Serial.println("Está tarde, dormindo até meia noite...");
    deepSleepUntilMidnight();
  }

  // Se chegou até aqui, currentTime está antes do tempo mais tarde. Ou seja, está esperando algum horário chegar

  // Precisamos descobrir qual é a hora que estamos aguardando
  int nextTime = -1;
  for (int i = 0; i < 3; i++){
    if (currentTime < irrigationTimes[i]){
      nextTime = irrigationTimes[i];
      Serial.printf("nextTime = %d\n", nextTime);
      break;
    }
  }

  // A hora que estamos aguardando é nextTime.
  // Precisamos saber se falta muito ou se já está na hora

  // 2. Ainda não está na hora, precisa dormir
  if (currentTime < nextTime - offset){
    int sleepTime = nextTime - offset - currentTime;
    Serial.printf("Está cedo ainda, dormindo por %d segundos...\n", sleepTime);
    deepSleep(sleepTime);
  }

  // 3. Está na hora certa, vamos coletar os dados
  Serial.println("Chegou a hora! Vou coletar os dados do sensor.");
  collectData(nextTime + offset);
  Serial.println("Acabou, vou voltar a dormir.");

  // Por fim, precisamos dormir até a próxima hora ou até a meia-noite.
  nextTime = -1;
  for (int i = 0; i < 3; i++){
    if (currentTime < irrigationTimes[i]){
      int nextTime = irrigationTimes[i];
      break;
    }
  }

  // Dormir até a hora determinada
  if(nextTime == -1){
    deepSleepUntilMidnight();
    Serial.println("Está tarde, vou dormir até meia noite...");
  }
  else{
    int sleepTime = nextTime - offset - currentTime;
    Serial.printf("Hoje ainda tem trabalho, vou dormir por %d segundos...\n", sleepTime);
    deepSleep(sleepTime);
  }

}

// Função que coleta dados do sensor
void collectData(int final) {

  int currentTime = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

  while (currentTime < final) {

    int value = analogRead(pin);
    Serial.printf("Valor lido pelo sensor: %d\n", value);
    delay(500);

    // Atualiza o tempo atual
    if (getLocalTime(&timeinfo)) {
      currentTime = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
    } 
    else {
      Serial.println("Erro ao atualizar o tempo!");
      break;
    }

  }

}


void loop(){
    // Não utilizado porque estamos usando deep sleep
}