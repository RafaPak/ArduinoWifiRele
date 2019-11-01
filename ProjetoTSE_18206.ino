 // Projeto TSE - Rafael Pak Bragagnolo | 18206 | PD18

// Inclusão de bibliotecas
#include "WiFiEsp.h"

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(8, 9); // Porta serial virtual
#endif

char ssid[] = "TSE_RafaelPak"; // Nome da rede
char pass[] = "00018206"; // Senha da rede
int status = WL_IDLE_STATUS; // Status da rede

WiFiEspServer server(80); // Servidor Wifi
RingBuffer buf(8); // Leitor de respostas para do servidor

int pRele = 2; // Rele que será controlado e seu pino
boolean aceso = false; // Variável auxiliar para saber o status do led

void setup() {
  // put your setup code here, to run once:
  
  pinMode(pRele, OUTPUT); // Define o pino como saída
  digitalWrite(pRele, HIGH); // Inicia o led do rele desligado
  
  Serial.begin(115200); // Inicializa a porta para debbugar
  Serial1.begin(9600); // Inicializa a porta serial virtual
  
  WiFi.init(&Serial1); // Inicializa o módulo WiFi
  
  if (WiFi.status() == WL_NO_SHIELD) // Verificação da conexão do módulo
  {
    Serial.println();
    Serial.println("Sem modulo");
    Serial.println();
    while(true); // Trava a execução 
  }

  Serial.println();
  Serial.print("Tentando inciar a rede"); // Informa que está iniciando a rede
  Serial.println(ssid);
  Serial.println();

  IPAddress localIp(192, 168, 0, 1); // Cria um IP
  WiFi.configAP(localIp); // Configura o IP como sendo o da rede WiFi

  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK); // Inicia a rede WiFi 

  Serial.println();
  Serial.println("Ponto de acesso iniciado");
  Serial.println();
  printWifiStatus(); // Função que informa o IP e como se conectar

  server.begin(); // Inicia o servidor e avisa
  Serial.println();
  Serial.println("Servidor iniciado");
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  WiFiEspClient client = server.available(); // Instancia um cliente
  
  if (client) // Checa a conexão do cliente
  {
    Serial.println("Bem-vindo, novo cliente!");
    buf.init(); // Inicializa o leitor de respostas do servidor

    while (client.connected()) // Enquanto o cliente está conectado
    { 
      // As respostas do cliente serão lidas e terão
      // reações para cada uma, enquanto ele estiver conectado
      
      if (client.available()) // Se o cliente enviou alguma resposta
      {
        char c = client.read(); // Usuário que está acessando a rede e que será informado no Serial
        buf.push(c);
        Serial.print(c);

        // Gera a página HTML
        if (buf.endsWith("\r\n\r\n"))
        {
          sendHttpResponse(client); // Gera a página HTML para o cliente
          break;
        }
        
        // Liga o Led
        if (buf.endsWith("GET /A"))
        {
          digitalWrite(pRele, LOW); // Liga o Led e informa
          Serial.println();
          Serial.println("Led Ligado");
          Serial.println();
          aceso = true; // Altera o valor da variável do status do Led
          buf.reset(); // Reseta o valor lido pelo leitor
        }
        
        // Desliga o Led
        if (buf.endsWith("GET /B"))
        {
          digitalWrite(pRele, HIGH); // Desliga o Led e informa
          Serial.println();
          Serial.println("Led Desligado");
          Serial.println();
          aceso = false; // Altera o valor da variável do status do Led
          buf.reset(); // Reseta o valor lido pelo leitor
        }
      }
    } // while
    
    delay(10); // Dá um tempo para receber as informações
    
    client.stop(); // Desconecta o cliente e o avisa
    Serial.println();
    Serial.println("Cliente desconectado");
    Serial.println();
  }
}

void sendHttpResponse(WiFiEspClient client)
{
  // Página HTML gerada pelo Arduino para controlar o status do Led
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "Refresh: 20\r\n"
    "\r\n");
  client.print("<!DOCTYPE HTML><html>\r\n");
  client.print("<head><title> Projeto de Tópicos em Sistemas Embarcados </title></head>\r\n");
  client.print("<body><center> <br><br>\r\n");
  client.print("<h1> Projeto TSE: </h1> <br><br>\r\n");
  client.print("<h3> O projeto consiste no acionamento de um rele ligado a um arduino via rede wifi, a partir de uma pagina web provida pelo esp8266. </h3>\r\n");

  // De acordo com o estado do Led, a página gera uma resposta para alterar seu valor e a outra estará apenas visualizável como link, porém não pode ser selecionada
  if (aceso)
  {
    client.print("<p><font color='green'> Led Ligado </font></p>\r\n");
    client.print("<a><font color='yellow'> Ligar Led </font></a>\r\n");
    client.print("<a href=\"B\"><font color='grey'> Desligar Led </font></a>\r\n"); // B - requisição para desligar
  }
  if (!aceso)
  {
    client.print("<p><font color='red'> Led Desligado </p>\r\n");
    client.print("<a href=\"A\"><font color='yellow'> Ligar Led </font></a>\r\n"); // A - requisição para ligar
    client.print("<a><font color='grey'> Desligar Led </font></a>\r\n");
  }
  
  client.print("</center></body></html>\r\n");
}

void printWifiStatus()
{
  // Informa o IP da rede Wifi
  IPAddress ip = WiFi.localIP();
  Serial.println();
  Serial.print("Endereço IP: ");
  Serial.print(ip);
  Serial.println();

  // Informa como acessar a página HTML para controlar o Led
  Serial.print("Para controlar o Led, conecte-se em: ");
  Serial.print(ssid);
  Serial.print(" e abra seu navegador em http://");
  Serial.print(ip);
  Serial.println();
}
