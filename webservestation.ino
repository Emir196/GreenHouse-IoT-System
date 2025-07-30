#include <DHT.h>
    #include <OneWire.h>
    #include <DallasTemperature.h>
    #include <WiFi.h>
    #include <WebServer.h>
    #include <ArduinoJson.h>

    // Sensor pins
    #define DHT11_PIN 21        // Temperature and humidity sensor
    #define ONE_WIRE_BUS 4      // Water temperature sensor
    #define WATER_LEVEL_TRIG 5  // Ultrasonic sensor trigger
    #define WATER_LEVEL_ECHO 18 // Ultrasonic sensor echo
    #define LDR_PIN 34         // Light sensor
    #define FLAME_SENSOR_PIN 36 // Fire detection sensor

    #define SOUND_SPEED 0.034   // Speed of sound for ultrasonic sensor

    // WiFi credentials
    const char* ssid = "WIFI";
    const char* password = "PASSWORD";    

    // Global variables
    long duration;
    int lightValue;

    // Web server setup
    WebServer server(80);

    // HTML content
    const char* html_content = R"rawliteral(
    <!DOCTYPE html>
    <html lang="tr">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>IOT Vertical Farm System</title>
        <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600&display=swap" rel="stylesheet">
        <style>
            :root {
                --primary-color: #2ecc71;
                --secondary-color: #27ae60;
                --thirdy-color: ##343434
                --text-color: #2c3e50;
                --card-bg: rgba(255,255,255,0.9);
                --header-bg: linear-gradient(135deg, #2ecc71 0%, #27ae60 100%);
            }
            
            body {
                font-family: 'Poppins', sans-serif;
                background: linear-gradient(135deg, rgba(26, 93, 26, 0.85) 0%, rgba(100, 200, 200, 0.75) 100%);
                background-blend-mode: multiply;
                color: var(--text-color);
                margin: 0;
                padding: 0;
                min-height: 100vh;
                text-shadow: 0 1px 3px rgba(0,0,0,0.5);
            } 
            
            .container {
                max-width: 1200px;
                margin: 30px auto;
                padding: 20px;
            }
            
            .header {
                background: var(--header-bg);
                color: #fff;
                border-radius: 20px;
                padding: 40px 0 30px 0;
                text-align: center;
                margin-bottom: 40px;
                box-shadow: 0 10px 30px rgba(46,204,113,0.2);
                position: relative;
                overflow: hidden;
            }
            
            .header::before {
                content: '';
                position: absolute;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background: url('data:image/svg+xml,<svg width="100" height="100" viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg"><path d="M0 0h100v100H0z" fill="none"/><path d="M0 0h100v100H0z" fill="rgba(255,255,255,0.1)"/></svg>');
                opacity: 0.1;
            }
            
            .header h1 {
                font-size: 2.8em;
                margin-bottom: 15px;
                text-shadow: 0 2px 4px rgba(0,0,0,0.1);
            }
            
            .header p {
                font-size: 1.2em;
                opacity: 0.9;
                margin: 0;
            }
            
            .sensor-grid {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
                gap: 30px;
                padding: 20px;
            }
            
            .sensor-card {
                background: var(--card-bg);
                border-radius: 20px;
                padding: 30px 20px;
                box-shadow: 0 10px 30px rgba(44,62,80,0.1);
                display: flex;
                flex-direction: column;
                align-items: center;
                transition: transform 0.3s ease, box-shadow 0.3s ease;
                position: relative;
                overflow: hidden;
            }
            
            .sensor-card:hover {
                transform: translateY(-5px);
                box-shadow: 0 15px 35px rgba(44,62,80,0.15);
            }
            
            .sensor-card::before {
                content: '';
                position: absolute;
                top: 0;
                left: 0;
                width: 100%;
                height: 5px;
                background: var(--primary-color);
            }
            
            .sensor-label {
                color: #7f8c8d;
                font-size: 1.1em;
                margin-bottom: 15px;
                text-align: center;
                font-weight: 500;
            }
            
            .sensor-value {
                font-size: 2.5em;
                font-weight: 600;
                color: var(--thirdy-color);
                margin-bottom: 15px;
                text-shadow: 0 2px 8px rgba(44,62,80,0.1);
            }
            
            .button {
                margin-top: 15px;
                padding: 12px 30px;
                background: var(--primary-color);
                color: #fff;
                border: none;
                border-radius: 10px;
                font-size: 1.1em;
                cursor: pointer;
                transition: all 0.3s ease;
                box-shadow: 0 4px 15px rgba(46,204,113,0.3);
                font-weight: 500;
            }
            
            .button:hover {
                background: var(--secondary-color);
                transform: translateY(-2px);
                box-shadow: 0 6px 20px rgba(46,204,113,0.4);
            }
            
            .last-update {
                position: fixed;      /* Sayfada sabit kalır */
                bottom: 0;            /* En alta yerleşir */
                left: 0;
                width: 100%;          /* Tam genişlik */
                text-align: center;   /* İçeriği ortalar */
                color: #7f8c8d;
                font-size: 1em;
                padding: 8px 0;
                background: var(--card-bg);
                border-radius: 15px 15px 0 0; /* Sadece üst köşeleri yuvarlak */
                box-shadow: 0 -5px 15px rgba(44,62,80,0.05);
                z-index: 1000;        /* Üstte görünmesi için */
            }

            @media (max-width: 768px) {
                .container {
                    margin: 15px;
                    padding: 10px;
                }
                
                .header {
                    padding: 30px 0 20px 0;
                    margin-bottom: 30px;
                }
                
                .header h1 {
                    font-size: 2.2em;
                }
                
                .sensor-grid {
                    gap: 20px;
                    padding: 10px;
                }
                
                .sensor-card {
                    padding: 20px 15px;
                }
            }
        </style>
    </head>
    <body>
        <div class="container">
            <div class="header">
                <h1>IOT Vertical Farm System</h1>
                <p>Environmental Monitoring Dashboard</p>
            </div>
            <div class="sensor-grid">
                <div class="sensor-card">
                    <div class="sensor-label">Ambient Temperature</div>
                    <div class="sensor-value" id="temperature">--°C</div>
                </div>
                <div class="sensor-card">
                    <div class="sensor-label">Water Temperature</div>
                    <div class="sensor-value" id="temperatureC">--°C</div>
                </div>
                <div class="sensor-card">
                    <div class="sensor-label">Humidity</div>
                    <div class="sensor-value" id="humidity">--%</div>
                </div>
                <div class="sensor-card">
                    <div class="sensor-label">Water Level</div>
                    <div class="sensor-value" id="water-level">-- cm</div>
                </div>
                <div class="sensor-card">
                    <div class="sensor-label">Fire Detection</div>
                    <div class="sensor-value" id="flame">--</div>
                </div>
                <div class="sensor-card">
                    <div class="sensor-label">Light Level</div>
                    <div class="sensor-value" id="light">--</div>
                </div>
                <div class="last-update">
                    Son Güncelleme: <span id="last-update-time">--:--:--</span><br>
                    <span class="name">Emir Kaan Demirdaş</span>
                </div>


        <script>
            function showFirePopup() {
                if (!document.getElementById('fire-popup')) {
                    var popup = document.createElement('div');
                    popup.id = 'fire-popup';
                    popup.innerHTML = `
                        <div style="display:flex;flex-direction:column;align-items:center;justify-content:center;">
                            <span style="font-size:60px;">&#128680;</span>
                            <span style="color:#fff;font-size:1.5em;font-weight:bold;margin-top:10px;">Yangın Algılandı.</span>
                        </div>
                    `;
                    popup.style.position = 'fixed';
                    popup.style.top = '0';
                    popup.style.left = '0';
                    popup.style.width = '100vw';
                    popup.style.height = '100vh';
                    popup.style.background = 'rgba(200,0,0,0.85)';
                    popup.style.display = 'flex';
                    popup.style.alignItems = 'center';
                    popup.style.justifyContent = 'center';
                    popup.style.zIndex = '9999';
                    document.body.appendChild(popup);
                }
            }
            function hideFirePopup() {
                var popup = document.getElementById('fire-popup');
                if (popup) popup.remove();
            }
            function updateSensorData() {
                fetch('/sensor-data')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('temperature').textContent = data.temperature + '°C';
                        document.getElementById('humidity').textContent = data.humidity + '%';
                        document.getElementById('water-level').textContent = data.waterLevel + ' cm';
                        document.getElementById('flame').textContent = data.flame;
                        document.getElementById('light').textContent = data.light;
                        document.getElementById('temperatureC').textContent = data.temperatureC + '°C';
                        document.getElementById('lighting-status').textContent = data.lightingStatus === 'On' ? 'Açık' : 'Kapalı';
                        // Pompa durumunu güncelle
                        let pumpStatus = document.getElementById('pump-status');
                        let pumpMode = document.getElementById('pump-mode');
                        if (data.pumpStatus === 'On') {
                            if (data.waterLevel <= 7 && data.waterLevel > 0) {
                                pumpStatus.textContent = 'Su Dolduruluyor';
                            } else if (data.waterLevel >= 10 && data.waterLevel < 15) {
                                pumpStatus.textContent = 'Su Boşaltılıyor';
                            } else {
                                pumpStatus.textContent = 'Açık';
                            }
                        } else {
                            pumpStatus.textContent = 'Kapalı';
                        }
                        // İlaçlama makinesi durumunu güncelle
                        let sprayerStatus = document.getElementById('sprayer-status');
                        sprayerStatus.textContent = data.sprayerStatus === 'On' ? 'Açık' : 'Kapalı';
                        var now = new Date();
                        document.getElementById('last-update-time').textContent = now.toLocaleTimeString('tr-TR');
                        // Yangın popup kontrolü
                        if (data.fireDetected) {
                            showFirePopup();
                        } else {
                            hideFirePopup();
                        }
                    })
                    .catch(error => console.error('Veri güncelleme hatası:', error));
            }
            
            function toggleSprayer() {
                fetch('/toggle-device?device=sprayer')
                    .then(response => response.json())
                    .then(data => {
                        if(data.success) {
                            // İlaçlama makinesi durumunu hemen güncelle
                            let sprayerStatus = document.getElementById('sprayer-status');
                            sprayerStatus.textContent = data.newStatus === 'On' ? 'Açık' : 'Kapalı';
                            
                            // Tüm sensör verilerini güncelle
                            updateSensorData();
                        } else {
                            alert('İlaçlama makinesi kontrol hatası: ' + data.message);
                        }
                    })
                    .catch(error => {
                        console.error('İlaçlama makinesi bağlantı hatası:', error);
                        alert('İlaçlama makinesi bağlantı hatası!');
                    });
            }
            
            // Her 200ms'de bir sensör verilerini güncelle
            setInterval(updateSensorData, 200);
            // Sayfa yüklendiğinde ilk güncellemeyi yap
            updateSensorData();
        </script>
    </body>
    </html>
    )rawliteral";

    OneWire oneWire(ONE_WIRE_BUS);
    DallasTemperature sensors(&oneWire);
    DHT dht11(DHT11_PIN, DHT11);

    // Sensör verilerini tutacak yapı
    struct SensorData {
        float temperature;     // Ortam sıcaklığı (°C)
        float humidity;        // Nem oranı (%)
        float waterTemperature; // Su sıcaklığı (°C)
        float waterLevel;      // Su seviyesi (cm)
        int flameValue;        // Alev sensörü değeri
        int lightValue;        // Işık seviyesi
        bool isSystemHealthy;  // Sistem sağlık durumu
    } sensorData;

    // Function to read all sensor data
    void readAllSensors() {
        // Read DHT11 (temperature and humidity)
        sensorData.temperature = dht11.readTemperature();
        sensorData.humidity = dht11.readHumidity();
        
        // Read water temperature
        sensors.requestTemperatures();
        sensorData.waterTemperature = sensors.getTempCByIndex(0);
        
        // Read water level
        digitalWrite(WATER_LEVEL_TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(WATER_LEVEL_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(WATER_LEVEL_TRIG, LOW);
        duration = pulseIn(WATER_LEVEL_ECHO, HIGH);
        sensorData.waterLevel = duration * 0.034/2;
        
        // Read flame and light sensors
        sensorData.flameValue = analogRead(FLAME_SENSOR_PIN);
        sensorData.lightValue = analogRead(LDR_PIN);
  
        // Error checking
        if (isnan(sensorData.temperature) || isnan(sensorData.humidity)) {
            Serial.println("Error: Failed to read from DHT11 sensor!");
            sensorData.isSystemHealthy = false;
        } else {
            sensorData.isSystemHealthy = true;
        }
        
        if (sensors.getTempCByIndex(0) == DEVICE_DISCONNECTED_C || sensors.getTempCByIndex(0) == -127.0) {
            Serial.println("Error: Water temperature sensor disconnected or faulty");
            sensorData.isSystemHealthy = false;
        }
}
        

    // Optimize edilmiş kontrol fonksiyonları

/*
    void waterTempControl(){
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print("Su sicakligi: ");
    Serial.print(temperatureC);
    Serial.println("°C");
    }

    void flameSensorControl() {
        //FLAME
    int infrared_value = analogRead(AO_PIN);
    Serial.print("Flame Sensor value: ");
    Serial.println(infrared_value);
    }



    void ldrSensorControl(){
    //LDR
    lightValue = analogRead(AO_PIN);
    Serial.print("Isik seviyesi: ");
    Serial.println(lightValue);
    }

    void athSensorControl(){
    //DHT-11
    // read humidity
    float humi  = dht11.readHumidity();
    // read temperature in Celsius
    float tempC = dht11.readTemperature();
    // read temperature in Fahrenheit
    float tempF = dht11.readTemperature(true);

    // check whether the reading is successful or not
    if ( isnan(tempC) || isnan(tempF) || isnan(humi)) {
        Serial.println("Failed to read from DHT11 sensor!");
    } else {
        Serial.print("Nem: ");
        Serial.print(humi);
        Serial.print("%");
        Serial.print("  |  ");
        Serial.print("Sicaklik: ");
        Serial.print(tempC);
        Serial.print("°C  ~  ");
        Serial.print(tempF);
        Serial.println("°F");
    }
    }
/*
    void waterLevelSensorControl(){
    //HCSR04
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    
    // Calculate the distance
    distanceCm = duration * SOUND_SPEED/2;
    
    // Convert to inches
    distanceInch = distanceCm * CM_TO_INCH;
    
    // Prints the distance in the Serial Monitor
    Serial.print("Su seviyesi (cm): ");
    Serial.println(distanceCm);
    }

    void lightingControl(){
        if(lightValue > 1000){
        digitalWrite(LIGHTS,LOW);
        }
        else if(lightValue <= 1000 && lightValue > 0){
        digitalWrite(LIGHTS,HIGH);
        }
        else{
        digitalWrite(LIGHTS,LOW);
        }
    }
*/

    void setup() {
    Serial.begin(115200);
    
    // Initialize sensors
    dht11.begin();    // Start DHT11
    sensors.begin();  // Start DS18B20
    
    // Configure pins
    pinMode(WATER_LEVEL_TRIG, OUTPUT);
    pinMode(WATER_LEVEL_ECHO, INPUT);
    
    // Configure ADC
    analogSetAttenuation(ADC_11db);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());



    // Web sunucusu rotaları
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html; charset=utf-8", html_content);
    });

    server.on("/sensor-data", HTTP_GET, []() {
        StaticJsonDocument<300> doc;
        
        // Convert sensor data to JSON
        doc["temperature"] = isnan(sensorData.temperature) ? 0 : sensorData.temperature;
        doc["humidity"] = isnan(sensorData.humidity) ? 0 : sensorData.humidity;
        doc["waterLevel"] = sensorData.waterLevel;
        doc["flame"] = sensorData.flameValue;
        doc["light"] = sensorData.lightValue;
        doc["temperatureC"] = isnan(sensorData.waterTemperature) ? 0 : sensorData.waterTemperature;
        doc["systemHealth"] = sensorData.isSystemHealthy;
        doc["fireDetected"] = sensorData.flameValue >= 500;
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });





    server.begin();
    Serial.println("HTTP sunucusu başlatıldı");
    }

    void loop() {
    server.handleClient();
    
    // Read all sensors
    readAllSensors();
    
    // Print sensor data to Serial Monitor
    Serial.println("=== Sensor Readings ===");
    Serial.print("Air Temperature: ");
    Serial.print(sensorData.temperature);
    Serial.println("°C");
    
    Serial.print("Humidity: ");
    Serial.print(sensorData.humidity);
    Serial.println("%");
    
    Serial.print("Water Temperature: ");
    Serial.print(sensorData.waterTemperature);
    Serial.println("°C");
    
    Serial.print("Water Level: ");
    Serial.print(sensorData.waterLevel);
    Serial.println(" cm");
    
    Serial.print("Light Level: ");
    Serial.println(sensorData.lightValue);
    
    Serial.print("Fire Sensor: ");
    Serial.println(sensorData.flameValue);
    Serial.println("==================\n");
    
    delay(200);
    }
