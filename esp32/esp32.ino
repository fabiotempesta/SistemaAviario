//---Bibliotecas---
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "ESPAsyncWebServer.h"


//---Pinos de entrada e saída---
#define PIN_DS18B20 2
#define PIN_DHT11 4
#define PIN_NIPPLE 27  
#define PIN_FAN 19 
#define PIN_NEBULIZER 21

//---Configuração de bibliotecas---
#define DHTYPE DHT11
DHT dht(PIN_DHT11, DHTYPE);
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);
AsyncWebServer server(80);

//---Conexão na rede local sem fio---
const char* ssid     = "Bethpsi";
const char* password = "raulfabio21";

//---Variáveis globais---
//Valores dos parâmetros
float deltat_nipple_parameter = 5.4;
float temperature_nebulizer_parameter = 20.0;
float temperature_fan_parameter = 22.2;
int humidity_nebulizer_parameter = 90;
//Valores atuais rebido dos sensores
float current_nipple_temp = 0;
float current_box_temp = 0;
float current_climate_temp = 0;
float current_climate_humidity = 0;
//Variáveis de tempo para função millis()
long update_time = 0;
long exchanger_time = 0;
//Variável para auxiliar desligar ou manter o ventilador ligado após desligar o nebulizador
bool fan_on_directly = 0; 
//Modos de operação
bool operation_mode_nebulizer = false; //false = automático | true = manual
bool operation_mode_fan = false; //false = automático | true = manual
bool operation_mode_exchanger = true; //false = automático | true = manual


//PROGMEM para armazenar na memória flash | R"rawliteral" = trate tudo como uma "raw string"
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="pt-BR">
<!-- Required meta tags -->
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">

<!-- Bootstrap CSS -->
<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet"
  integrity="sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3" crossorigin="anonymous">

<title>Controle Aviário</title>
<style>
  img {
    display: block;
    margin-left: auto;
    margin-right: auto
  }

  .navbar .dropdown-menu {
    background-color: #f0ad4e;
  }
</style>
</head>

<body>
  <nav class="navbar navbar-expand-sm navbar-light bg-warning">
    <div class="container-fluid">
      <a class="navbar-brand" href="#"><img src="C:\Users\gig9\Desktop\download-removebg-preview.ico" height="30" /></a>
      <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarNav"
        aria-controls="navbarNav" aria-expanded="false" aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
      </button>
      <div class="collapse navbar-collapse" id="navbarNav">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="nav-link active" aria-current="page" href="#">Página Principal</a>
          </li>
          <li class="nav-item dropdown">
            <a class="nav-link dropdown-toggle" href="#" id="sensors_dropdown" role="button" data-toggle="dropdown"
              aria-haspopup="true" aria-expanded="false">
              Monitoramento
            </a>
            <div class="dropdown-menu" aria-labelledby="sensors_dropdown">
              <a class="dropdown-item" href="#">Temperatura Ambiente</a>
              <a class="dropdown-item" href="#">Umidade relativa</a>
              <a class="dropdown-item" href="#">Temperatura da Água do Bebedouro</a>
              <a class="dropdown-item" href="#">Temperatura da Água da Caixa</a>

            </div>
          </li>
          <li class="nav-item dropdown">
            <a class="nav-link dropdown-toggle" href="#" id="actuators_dropdown" role="button" data-toggle="dropdown"
              aria-haspopup="true" aria-expanded="false">
              Controle
            </a>
            <div class="dropdown-menu" aria-labelledby="actuators_dropdown">
              <a class="dropdown-item" href="#">Nebulizador</a>
              <a class="dropdown-item" href="#">Trocador de Água</a>
              <a class="dropdown-item" href="#">Ventilador</a>

            </div>
          </li>

        </ul>
      </div>


    </div>
  </nav>
  <p class="h1 text-center text-uppercase">Monitoramento</p>
  <div class="container-fluid border border-4 border-dark">
    <div class="row">
      <div class="col border-end border-4 border-dark">
        <p class="h5 text-center text-uppercase">Temperatura Ambiente</p>
      </div>
      <div class="col border-end border-4 border-dark">
        <p class="h5 text-center text-uppercase">Umidade relativa</p>
      </div>
      <div class="col border-end border-4 border-dark">
        <p class="h5 text-center text-uppercase">Temperatura da Água do Bebedouro</p>
      </div>
      <div class="col">
        <p class="h5 text-center text-uppercase">Temperatura da Água da Caixa</p>
      </div>
    </div>
  </div>
  <p class="h1 text-center text-uppercase">controle</p>
  <div class="container-fluid border border-4 border-dark">
    <div class="row ">
      <div class="col col border-end border-4 border-dark">
        <p class="h5 text-center text-uppercase">Nebulizador</p>
      </div>
      <div class="col col border-end border-4 border-dark">
        <p class="h5 text-center text-uppercase">Trocador de Água</p>
      </div>
      <div class="col">
        <p class="h5 text-center text-uppercase">Ventilador</p>
      </div>
    </div>
  </div>
  </div>







  <!-- Optional JavaScript -->
  <!-- jQuery first, then Popper.js, then Bootstrap JS -->

  <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js"
    integrity="sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj"
    crossorigin="anonymous"></script>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/js/bootstrap.bundle.min.js"
    integrity="sha384-ho+j7jyWK8fNQe+A12Hb8AhRq26LrZ/JpcUGGOn+Y7RsweNrtN/tE3MoK7ZeZDyx"
    crossorigin="anonymous"></script>


  <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js"
    integrity="sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj"
    crossorigin="anonymous"></script>
  <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.1/dist/umd/popper.min.js"
    integrity="sha384-9/reFTGAW83EW2RDu2S0VKaIzap3H66lZH81PoYlFhbGU+6BZp6G7niu735Sk7lN"
    crossorigin="anonymous"></script>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/js/bootstrap.min.js"
    integrity="sha384-w1Q4orYjBQndcko6MimVbzY0tgp4pWB4lZ7lr30WKz0vr/aWKhXdBNmNb5D92v7s"
    crossorigin="anonymous"></script>

</body>

</html>)rawliteral";

int stringToBool(String n){ //auxiliar para transformar em booleano
  if(n == "1")
    return 1;
  else{
    if(n == "0"){
      return 0;
    }else{
      return 2;
    }
  }
    
}

String getStatusActuator(char n){
  //'n' para nebulizador; 'e' para o trocador de agua; 'f' para o ventilador
  int pin;
  switch (n) {
    case 'n':
      pin=PIN_NEBULIZER;
      break;
    case 'e':
      pin=PIN_NIPPLE;
      break;
    case 'f':
      pin=PIN_FAN;
      break;
  }

  if(digitalRead(pin)==HIGH)
    return "0";
  else
    return "1";
  
}

void setNebulizer(bool state){
  if (state == 1){
    digitalWrite(PIN_NEBULIZER, HIGH);
    if (digitalRead(PIN_FAN)==HIGH)
      fan_on_directly = 1;
    else{
      fan_on_directly=0;
      digitalWrite(PIN_FAN, HIGH);
    }
  }
  if (state == 0){
    digitalWrite(PIN_NEBULIZER, LOW);
    if(fan_on_directly == 0)
      digitalWrite(PIN_FAN, LOW);
  }
}

void setExchanger(bool state){
  //Liga trocador de água
	if (state){
    digitalWrite(PIN_NIPPLE, HIGH);
    exchanger_time = millis() + 20000;
	}else{ 
    digitalWrite(PIN_NIPPLE, LOW);
	}
}

void automaticModeNebulizer(){
  if(!operation_mode_nebulizer){
    //Liga Nebulizador
    if(digitalRead(PIN_NEBULIZER) == LOW){
      if((current_climate_humidity < humidity_nebulizer_parameter) && (current_climate_temp > temperature_nebulizer_parameter)){
        setNebulizer(1);
        Serial.println("Nebulizador ligado via modo automático");
      }

    //Desliga Nebulizador
    }else{
      if((current_climate_humidity > (humidity_nebulizer_parameter + 3)) || (current_climate_temp < (temperature_nebulizer_parameter + 0.5)) ){
        setNebulizer(0);
        Serial.println("Nebulizador desligado via modo automático");
      }
    }
  }
}

void automaticModeExchanger(){
  if(!operation_mode_exchanger){
    if( (current_nipple_temp - current_box_temp) > deltat_nipple_parameter){
      Serial.println("Água trocada via modo automático");
      setExchanger(1);
    }
  }
}

void automaticModeFan(){
  if(!operation_mode_fan)  {//Liga Ventilador
    if (digitalRead(PIN_FAN) == LOW){
      if (current_climate_temp>temperature_fan_parameter){
        digitalWrite(PIN_FAN, HIGH);
        Serial.println("Ventilaador ligado via modo automático");
      }
    //Desliga Ventilador
    }else{
      if ((current_climate_temp < (temperature_fan_parameter - 2)) && (digitalRead(PIN_NEBULIZER) == LOW)){
        digitalWrite(PIN_FAN, LOW);
        Serial.println("Ventilador ligado via modo automático");
      }
    }
  }
}

void changeWaterOff(){
  if((digitalRead(PIN_NIPPLE)==HIGH) && (exchanger_time < millis())){
    digitalWrite(PIN_NIPPLE, LOW);
    Serial.println("Trocador de água terminou sua ação!");
	}
}

void printSensorsValue(){
	Serial.println("VALORES DOS SENSORES");
	// Sensores DS18B20
	Serial.print("Temperatura do bebedouro: ");
	Serial.print(current_nipple_temp);
	Serial.println("ºC");

	Serial.print("Temperatura da água da caixa: ");
	Serial.print(current_box_temp);
	Serial.println("ºC");


  //Sensor DHT11
  
	if (isnan(current_climate_temp) || isnan(current_climate_humidity)) 
    	Serial.println("Falha em ler o sensor DHT11!");
  	else{
		Serial.print("Temperatura do aviário: ");
		Serial.print(current_climate_temp);
		Serial.print(" ºC\n");
		Serial.print("Umidade do aviário: ");
		Serial.print(current_climate_humidity);
		Serial.println(" % ");
		Serial.println();
  	}
  
  
}

void updateSensorsValue(){

  Serial.println("Atualizando valores dos sensores.");
	ds18b20.requestTemperatures(); 
	current_nipple_temp = ds18b20.getTempCByIndex(0);
	current_box_temp = ds18b20.getTempCByIndex(1);
	current_climate_temp = dht.readTemperature();
	current_climate_humidity = dht.readHumidity();
}

void setup() {

	Serial.begin(9600);
	pinMode(PIN_NIPPLE, OUTPUT);
	pinMode(PIN_FAN, OUTPUT);
	pinMode(PIN_NEBULIZER, OUTPUT);
	digitalWrite(PIN_NIPPLE, LOW);
	digitalWrite(PIN_FAN, LOW);
	digitalWrite(PIN_NEBULIZER, LOW);
	
  ds18b20.begin();
	dht.begin();
  
  Serial.print("Conectando em ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  long time_out_connect = 30000 + millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (time_out_connect > millis()){
      WiFi.reconnect();
      long time_out_connect = 30000 + millis();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());



  // URL para raiz (index)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/html", index_html, processor);
    request->send_P(200, "text/html", index_html);
  });

  //---Rotas para requisições de origem do JavaScript---

  //URL para requisitar a temperatura   temperatura em c_str, porque deve ser mandado em vetor de char   request é um ponteiro
  //request é um ponteiro que aponta pra qual tipo de requisição vai ser feita  send_P é para enviar uma pagina web grande  send é para respostas simples

  //URL para requisitar o valor atual de cada sensor
  server.on("/sensors/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(current_climate_temp) + " " + String(current_climate_humidity) + " " + String(current_box_temp) + " " + String(current_nipple_temp);
    request->send(200, "text/plain", response.c_str());
  });

  //URL para acionar/desligar o nebulizador
  server.on(
    "/actuators/status/set/nebulizer",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(stringToBool(p->value()) != 2){
            setNebulizer(stringToBool(p->value()));
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  //URL para ligar o trocador de agua
  server.on(
    "/actuators/status/set/exchanger", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(stringToBool(p->value()) != 2){
            setExchanger(stringToBool(p->value()));
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  //URL para ligar/desligar o ventilador
  server.on(
    "/actuators/status/set/fan", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(stringToBool(p->value()) != 2){
            if (stringToBool(p->value())){
              digitalWrite(PIN_FAN,HIGH);
              request->send(200);
              Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
            }else{
              if(digitalRead(PIN_NEBULIZER)==HIGH)
                request->send(409, "text/plain", String("Ventilador não pode ser desligado quando o nebulizador estiver em funcionamento!").c_str());
            }
            
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304, "text/plain", String("Erro interface!").c_str());
          }
        }
    }
  });

  //URL para capturar o status atual de cada atuador
  server.on("/actuators/status/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(getStatusActuator('e')) +" " + String(getStatusActuator('f')) + " " + String( getStatusActuator('n'));
    request->send(200, "text/plain", response.c_str());
  });

  //URL para capturar o valor atual de cada parâmetro de acionamento
  server.on("/actuators/parameter/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(humidity_nebulizer_parameter) +" " + String(temperature_nebulizer_parameter) + " " + String(deltat_nipple_parameter) + " " + String(temperature_fan_parameter);
    request->send(200, "text/plain", response.c_str());
  });

  //URL para capturar o modo de operação de cada atuador
  server.on("/actuators/opmode/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(operation_mode_nebulizer) + " " + String(operation_mode_exchanger) + " " + String(operation_mode_fan);
    request->send(200, "text/plain", response.c_str());
  });

  //URL para modificar o modo de operação do nebulizador
  server.on(
    "/actuators/opmode/set/nebulizer", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(stringToBool(p->value()) != 2){
            operation_mode_nebulizer = stringToBool(p->value());
            Serial.println(stringToBool(p->value()));
            request->send(200);
            Serial.println("Nebulizador trocou seu modo de operação!");
          }else{
            Serial.println("Erro em mudar o modo de operação do Nebulizador. (Valor inválido) not int");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o modo de operação do trocador de água
  server.on(
    "/actuators/opmode/set/exchanger", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(stringToBool(p->value()) != 2){
            operation_mode_exchanger = stringToBool(p->value());
            request->send(200);
            Serial.println("Trocador trocou seu modo de operação!");
          }else{
            Serial.println("Erro em mudar o modo de operação do Trocador. (Valor inválido)");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o modo de operação do ventilador
  server.on(
    "/actuators/opmode/set/fan", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(stringToBool(p->value()) != 2){
            operation_mode_fan = stringToBool(p->value());
            request->send(200);
            Serial.println("Ventilador trocou seu modo de operação!");
          }else{
            Serial.println("Erro em mudar o modo de operação do Ventilador. (Valor inválido)");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o parâmetro de temperatura do ventilador
  server.on(
    "/actuators/parameter/set/fan/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          
          if(float current_value = (p->value().toFloat())){
            temperature_fan_parameter = current_value;
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o parâmetro de variação da temperatura do trocador de água
  server.on(
    "/actuators/parameter/set/exchanger/deltatemp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name() == "value"){
          if(float current_value = (p->value().toFloat())){
            deltat_nipple_parameter = current_value;
            request->send(200);
            Serial.println("Parâmetro de acionamento do trocador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do trocador via interface Web!");
          }
        }
    }
  });

  //URL para modificar o parâmetro de umidade do nebulizador
  server.on(
    "/actuators/parameter/set/nebulizer/humidity",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            humidity_nebulizer_parameter = current_value;
            request->send(200);
            Serial.println("Parâmetro de umidade de acionamento do nebulizador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do nebulizador via interface Web!");
          }
        }
    }
  });
  
  //URL para modificar o parâmetro de temperatura do nebulizador
  server.on(
    "/actuators/parameter/nebulizer/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            temperature_nebulizer_parameter = current_value;
            request->send(200);
            Serial.println("Parâmetro de temperatura de acionamento do nebulizador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });
  
  server.begin();

}

void loop() {

	if (update_time < millis()){
    
    updateSensorsValue();
    printSensorsValue();
    
    automaticModeExchanger();
    automaticModeFan();
    automaticModeNebulizer();

    update_time = millis()+5000;
  }

  changeWaterOff();
	
}
