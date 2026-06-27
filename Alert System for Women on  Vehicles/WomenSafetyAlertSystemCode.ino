 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include <TinyGPS++.h> 
#include <HardwareSerial.h> 
 
#define BUZZER   26 
#define GSM_RX   16 
#define GSM_TX   17 
#define GPS_RX   4 
#define GPS_TX   2 
 
#define BUZZER_FREQUENCY  2000 
 
#define BUZZER_RESOLUTION 8 
#define BUZZER_DUTY       128 
 
String phoneNumber = "+91XXXXXXXXXX"; 
 
#define LCD_ADDRESS  0x27 
 
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2); 
TinyGPSPlus gps; 
HardwareSerial gsmModule(2); 
HardwareSerial gpsModule(1); 
 
String typedText = ""; 
bool alertDone   = false; 
 
void buzzerON() { 
  ledcWrite(BUZZER, BUZZER_DUTY); 
} 
 
void buzzerOFF() { 
  ledcWrite(BUZZER, 0); 
} 
 
void buzzerBeep(int times) { 
  for (int i = 0; i < times; i++) { 
    buzzerON(); 
    delay(300); 
    buzzerOFF(); 
    delay(200); 
  } 
} 
 
void setup() { 
 
  Serial.begin(115200); 
  delay(1000); 
  Serial.println("============================="); 
  Serial.println("  WOMEN ALERT SYSTEM STARTING"); 
  Serial.println("============================="); 
 
  ledcAttach(BUZZER, BUZZER_FREQUENCY, 
BUZZER_RESOLUTION); 
  buzzerOFF(); 
 
  Serial.println("[BUZZER] Testing... (you should hear 2 beeps)"); 
  buzzerBeep(2); 
  Serial.println("[BUZZER] Test done."); 
 
  Serial.println("[LCD] Starting..."); 
  Wire.begin(21, 22); 
  lcd.init(); 
  lcd.backlight(); 
  delay(500); 
 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Women Alert Sys"); 
  lcd.setCursor(0, 1); 
  lcd.print("Starting..."); 
  Serial.println("[LCD] Started. Can you see text on LCD?"); 
 
  delay(2000); 
 
  Serial.println("[GSM] Starting..."); 
  gsmModule.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX); 
  delay(3000); 
 
  Serial.println("[GSM] Sending AT command..."); 
  gsmModule.println("AT"); 
  delay(1000); 
  String gsmResponse = readGSM(2000); 
  Serial.println("[GSM] Response: " + gsmResponse); 
 
  if (gsmResponse.indexOf("OK") != -1) { 
    Serial.println("[GSM] GSM module is working!"); 
  } else { 
    Serial.println("[GSM] WARNING: GSM not responding. Check 
wiring and power."); 
  } 
 
  gsmModule.println("ATE0"); 
  delay(500); 
  readGSM(500); 
 
  Serial.println("[GSM] Checking SIM card..."); 
  gsmModule.println("AT+CPIN?"); 
  delay(1000); 
  String simStatus = readGSM(2000); 
  Serial.println("[GSM] SIM Status: " + simStatus); 
 
  if (simStatus.indexOf("READY") != -1) { 
    Serial.println("[GSM] SIM card is OK!"); 
  } else { 
    Serial.println("[GSM] WARNING: SIM card problem. Check if 
SIM is inserted."); 
  } 
 
  Serial.println("[GSM] Checking network..."); 
  gsmModule.println("AT+CREG?"); 
  delay(1000); 
  String netStatus = readGSM(2000); 
  Serial.println("[GSM] Network: " + netStatus); 
 
  gsmModule.println("AT+CMGF=1"); 
  delay(500); 
  readGSM(500); 
  Serial.println("[GSM] Setup complete."); 
 
  Serial.println("[GPS] Starting..."); 
  gpsModule.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX); 
  Serial.println("[GPS] Started. GPS needs open sky to get signal."); 
 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Type 'start' in"); 
  lcd.setCursor(0, 1); 
  lcd.print("Serial Monitor"); 
 
  Serial.println(""); 
  Serial.println("============================="); 
  Serial.println("       SYSTEM IS READY       "); 
  Serial.println(" Type  start  + press Enter  "); 
  Serial.println("============================="); 
} 
 
void loop() { 
 
  while (gpsModule.available()) { 
    gps.encode(gpsModule.read()); 
  } 
 
  while (Serial.available()) { 
    char letter = (char)Serial.read(); 
 
    if (letter == '\n' || letter == '\r') { 
 
      typedText.trim(); 
      typedText.toLowerCase(); 
 
      if (typedText == "start" && alertDone == false) { 
        alertDone = true; 
        runAlertSystem(); 
      } else if (alertDone == false) { 
        Serial.println("Type  start  and press Enter to trigger alert."); 
      } 
 
      typedText = ""; 
 
    } else { 
      typedText += letter; 
    } 
  } 
} 
 
void runAlertSystem() { 
 
  Serial.println(""); 
  Serial.println("==== ALERT TRIGGERED! ===="); 
 
  buzzerON(); 
  Serial.println("[BUZZER] Buzzer is ON and buzzing loudly!"); 
 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("System Initiated"); 
  lcd.setCursor(0, 1); 
  lcd.print("Please wait..."); 
  Serial.println("[LCD] System Initiated"); 
  delay(1500); 
 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("System is Ready!"); 
  lcd.setCursor(0, 1); 
  lcd.print("Getting GPS..."); 
  Serial.println("[LCD] System is Ready!"); 
  Serial.println("[GPS] Getting live location..."); 
 
  double myLatitude  = 0.0; 
  double myLongitude = 0.0; 
  bool gotLocation = getGPSCoordinates(myLatitude, myLongitude); 
 
  if (gotLocation == true) { 
 
    String mapLink = "https: 
                     + String(myLatitude, 6) 
                     + "," 
                     + String(myLongitude, 6); 
 
    Serial.println("[GPS] Location found: " + mapLink); 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("Sending SMS..."); 
    lcd.setCursor(0, 1); 
    lcd.print("Please wait..."); 
 
    String smsText = "EMERGENCY ALERT!\n\n Need Immmediate 
Assistance.\n\n  live location:\n" 
                     + mapLink 
                     + "\n\n Please reach immediately."; 
    sendSMS(smsText); 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("Alert SMS Sent!"); 
    lcd.setCursor(0, 1); 
    lcd.print("Stay Safe!"); 
    Serial.println("[SMS] Alert sent successfully!"); 
 
  } else { 
 
    Serial.println("[GPS] Failed. Sending SMS without location."); 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("No GPS Signal"); 
    lcd.setCursor(0, 1); 
    lcd.print("Sending SMS..."); 
 
    sendSMS("ALERT! She needs HELP! GPS location not available. 
Please call her immediately!"); 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("Alert Sent!"); 
    lcd.setCursor(0, 1); 
    lcd.print("(No GPS)"); 
  } 
 
  delay(2000); 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("ALERT ACTIVE"); 
  lcd.setCursor(0, 1); 
  lcd.print("Buzzer is ON!"); 
 
  Serial.println(""); 
  Serial.println("[BUZZER] Buzzer stays ON continuously."); 
  Serial.println("[INFO] Press RESET button on ESP32 to restart 
system."); 
} 
 
bool getGPSCoordinates(double &lat, double &lon) { 
 
  Serial.println("[GPS] Waiting for GPS fix... (max 60 seconds)"); 
  Serial.println("[GPS] NOTE: GPS needs open sky. Move outside if 
indoors."); 
 
  unsigned long startTime = millis(); 
  unsigned long waitTime  = 60000; 
 
  while (millis() - startTime < waitTime) { 
 
    while (gpsModule.available()) { 
      gps.encode(gpsModule.read()); 
    } 
 
    if (gps.location.isValid()) { 
      lat = gps.location.lat(); 
      lon = gps.location.lng(); 
      Serial.println("[GPS] Fix found!"); 
      Serial.println("[GPS] Latitude:  " + String(lat, 6)); 
      Serial.println("[GPS] Longitude: " + String(lon, 6)); 
      return true; 
    } 
 
    int secondsPassed = (millis() - startTime) / 1000; 
 
    if (secondsPassed % 5 == 0) { 
      Serial.println("[GPS] Still searching... " + String(secondsPassed) 
+ " seconds passed."); 
    } 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("Getting GPS..."); 
    lcd.setCursor(0, 1); 
    lcd.print("Wait: " + String(secondsPassed) + "s"); 
 
    delay(1000); 
  } 
 
  Serial.println("[GPS] Timed out. No GPS fix after 60 seconds."); 
  return false; 
} 
 
void sendSMS(String message) { 
 
  Serial.println("[SMS] Sending to: " + phoneNumber); 
  Serial.println("[SMS] Message: " + message); 
 
  gsmModule.println("AT+CMGF=1"); 
  delay(1000); 
  readGSM(1000); 
 
  gsmModule.print("AT+CMGS=\""); 
  gsmModule.print(phoneNumber); 
  gsmModule.println("\""); 
  delay(2000); 
 
  String prompt = readGSM(2000); 
  Serial.println("[SMS] GSM prompt: " + prompt); 
 
  if (prompt.indexOf(">") != -1) { 
 
    Serial.println("[SMS] Typing message..."); 
    gsmModule.print(message); 
    delay(500); 
 
    gsmModule.write(26); 
    delay(5000); 
 
    String result = readGSM(5000); 
    Serial.println("[SMS] Result: " + result); 
 
    if (result.indexOf("+CMGS") != -1) { 
      Serial.println("[SMS] SMS SENT SUCCESSFULLY!"); 
    } else { 
      Serial.println("[SMS] SMS may have failed. Check SIM balance 
and signal."); 
    } 
 
  } else { 
    Serial.println("[SMS] ERROR: GSM did not respond with > 
prompt."); 
    Serial.println("[SMS] Check: 1) GSM power  2) SIM card  3) GSM 
wiring"); 
  } 
} 
 
String readGSM(unsigned long waitMs) { 
  String response = ""; 
  unsigned long start = millis(); 
  while (millis() - start < waitMs) { 
    while (gsmModule.available()) { 
      response += (char)gsmModule.read(); 
    } 
  } 
  response.trim(); 
  return response; 
} 
 