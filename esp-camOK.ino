#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#define BUTTON_PIN 13

const char* WIFI_SSID = "xxx";
const char* WIFI_PASS = "xxxxxx";

WebServer server(80);

static auto loRes = esp32cam::Resolution::find(800, 600);
static auto hiRes = esp32cam::Resolution::find(1600, 1200);
const uint16_t port = 8091;
const char * host = "10.54.234.126";
int Switchmode =0;
//button
int currentState; 
int ModeN=0;

void handleBmp()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }

  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  if (!frame->toBmp()) {
    Serial.println("CONVERT FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CONVERT OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/bmp");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleJpg()
{
  server.sendHeader("Location", "/cam-hi.jpg");
  server.send(302, "", "");
}

void handleMjpeg()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }

  Serial.println("STREAM BEGIN");
  WiFiClient client = server.client();
  auto startTime = millis();
  int res = esp32cam::Camera.streamMjpeg(client);
  if (res <= 0) {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
  auto duration = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam.bmp");
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
 

  server.on("/cam.bmp", handleBmp);
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);


  server.begin();
  //button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
//Sendin Socket
  server.handleClient();
//SwitchingMode
  SwitchButtons();
  
}




void sendsocket(String Mode)
{
   WiFiClient client;
    if (!client.connect(host, port)) {
 
        Serial.println("Connection to host failed");
 
        delay(100);
        return;
    }
    Serial.println("Connected to server successful!");
 
    client.print(Mode);
    Serial.println(touchRead(12));
    Serial.println("Disconnecting...");
    client.stop();
    delay(1000);
}
void SwitchButtons(){
   //touch Switching Mode
   int SwitchM=touchRead(12);
   int Valid=touchRead(14);
  if(SwitchM<10 && SwitchM>3 && Valid>40 ){
    delay(50);
    if(SwitchM == touchRead(12) ){
    Serial.println(SwitchM);
    Serial.println(touchRead(12));
    Serial.println("mode1");
    ModeN++;
    if(ModeN == 4 ){ModeN =1;}
    Serial.println(ModeN);
    sendsocket(String(ModeN));
    delay(1000);
    }
  }
  //touch validation 
  
if(Valid<10 && Valid>3  && SwitchM>40){
    delay(50);
    if(Valid == touchRead(14) ){
    Serial.println(Valid);
    Serial.println(touchRead(14));
    Serial.println("valid");
    sendsocket(String("OK"));
  delay(1000);
    }
  }
  //alert
  if(Valid<10 && Valid>3 && SwitchM<10 && SwitchM>3 ){
    delay(50);
    
    Serial.println(Valid);
    Serial.println(touchRead(14));
    Serial.println("ALERT");
    sendsocket(String("ALERT"));
  delay(1000);
    
  }
  }
