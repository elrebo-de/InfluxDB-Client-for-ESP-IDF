/**
 * Basic Write Example code for InfluxDBClient library for Arduino
 * Data can be immediately seen in a InfluxDB UI: wifi_status measurement
 * Enter WiFi and InfluxDB parameters below
 *
 * Measures signal level of the actually connected WiFi network
 * This example supports only InfluxDB running from unsecure (http://...)
 * For secure (https://...) or Influx Cloud 2 use SecureWrite example
 **/

#include <wifi_manager.h>
#include <ssid_manager.h>

#include <time.h>
#include <sys/time.h>
#include <esp_sntp.h>

#include "esp_log.h"
#include "nvs_flash.h"

static const char *tag = "InfluxDB-Client BasicWrite";

bool startConfigurationAp = false;  // set to true if BUTTON_LONG_PRESS (5000 ms) during start-up
bool startStation = false;

#define DEVICE "ESP32"

#include <InfluxDbClient.h>

// WiFi AP SSID
//#define WIFI_SSID "33"
// WiFi password
//#define WIFI_PASSWORD "90292532924275594434"
// InfluxDB  server url. Don't use localhost, always server name or ip address.
// E.g. http://192.168.1.48:8086 (In InfluxDB 2 UI -> Load Data -> Client Libraries), 
#define INFLUXDB_URL "https://www.elrebo.de:8181"
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "apiv3_J1PgX2tDA5vL2VsCGEQ9NTy5DN2chnqNP8aJWPw0tx1ybRM0wEQam6mqQGjLSFZeXKoS82Xs-5DixP9QGLCK5Q"
// InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "elrebo"
// InfluxDB 2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_BUCKET "sensors"
// InfluxDB v1 database name 
//#define INFLUXDB_DB_NAME "database"

const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFCzCCAvOgAwIBAgIQf/AFoHxM3tEArZ1mpRB7mDANBgkqhkiG9w0BAQsFADBH\n"
    "MQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExM\n"
    "QzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMjMxMjEzMDkwMDAwWhcNMjkwMjIw\n"
    "MTQwMDAwWjA7MQswCQYDVQQGEwJVUzEeMBwGA1UEChMVR29vZ2xlIFRydXN0IFNl\n"
    "cnZpY2VzMQwwCgYDVQQDEwNXUjIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
    "AoIBAQCp/5x/RR5wqFOfytnlDd5GV1d9vI+aWqxG8YSau5HbyfsvAfuSCQAWXqAc\n"
    "+MGr+XgvSszYhaLYWTwO0xj7sfUkDSbutltkdnwUxy96zqhMt/TZCPzfhyM1IKji\n"
    "aeKMTj+xWfpgoh6zySBTGYLKNlNtYE3pAJH8do1cCA8Kwtzxc2vFE24KT3rC8gIc\n"
    "LrRjg9ox9i11MLL7q8Ju26nADrn5Z9TDJVd06wW06Y613ijNzHoU5HEDy01hLmFX\n"
    "xRmpC5iEGuh5KdmyjS//V2pm4M6rlagplmNwEmceOuHbsCFx13ye/aoXbv4r+zgX\n"
    "FNFmp6+atXDMyGOBOozAKql2N87jAgMBAAGjgf4wgfswDgYDVR0PAQH/BAQDAgGG\n"
    "MB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/\n"
    "AgEAMB0GA1UdDgQWBBTeGx7teRXUPjckwyG77DQ5bUKyMDAfBgNVHSMEGDAWgBTk\n"
    "rysmcRorSCeFL1JmLO/wiRNxPjA0BggrBgEFBQcBAQQoMCYwJAYIKwYBBQUHMAKG\n"
    "GGh0dHA6Ly9pLnBraS5nb29nL3IxLmNydDArBgNVHR8EJDAiMCCgHqAchhpodHRw\n"
    "Oi8vYy5wa2kuZ29vZy9yL3IxLmNybDATBgNVHSAEDDAKMAgGBmeBDAECATANBgkq\n"
    "hkiG9w0BAQsFAAOCAgEARXWL5R87RBOWGqtY8TXJbz3S0DNKhjO6V1FP7sQ02hYS\n"
    "TL8Tnw3UVOlIecAwPJQl8hr0ujKUtjNyC4XuCRElNJThb0Lbgpt7fyqaqf9/qdLe\n"
    "SiDLs/sDA7j4BwXaWZIvGEaYzq9yviQmsR4ATb0IrZNBRAq7x9UBhb+TV+PfdBJT\n"
    "DhEl05vc3ssnbrPCuTNiOcLgNeFbpwkuGcuRKnZc8d/KI4RApW//mkHgte8y0YWu\n"
    "ryUJ8GLFbsLIbjL9uNrizkqRSvOFVU6xddZIMy9vhNkSXJ/UcZhjJY1pXAprffJB\n"
    "vei7j+Qi151lRehMCofa6WBmiA4fx+FOVsV2/7R6V2nyAiIJJkEd2nSi5SnzxJrl\n"
    "Xdaqev3htytmOPvoKWa676ATL/hzfvDaQBEcXd2Ppvy+275W+DKcH0FBbX62xevG\n"
    "iza3F4ydzxl6NJ8hk8R+dDXSqv1MbRT1ybB5W0k8878XSOjvmiYTDIfyc9acxVJr\n"
    "Y/cykHipa+te1pOhv7wYPYtZ9orGBV5SGOJm4NrB3K1aJar0RfzxC3ikr7Dyc6Qw\n"
    "qDTBU39CluVIQeuQRgwG3MuSxl7zRERDRilGoKb8uY45JzmxWuKxrfwT/478JuHU\n"
    "/oTxUFqOl2stKnn7QGTq8z29W+GgBLCXSBxC9epaHM0myFH/FJlniXJfHeytWt0=\n"
    "-----END CERTIFICATE-----\n";

// Function to initialize SNTP with multiple servers
void initialize_sntp(void)
{
    ESP_LOGI(tag, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // Set multiple NTP servers
    esp_sntp_setservername(0, "pool.ntp.org");    // Primary server
    esp_sntp_setservername(1, "time.nist.gov");   // Secondary server
    esp_sntp_setservername(2, "time.google.com"); // Tertiary server

    esp_sntp_init();
}

// Function to obtain and print time
void obtain_time(void)
{
    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2025 - 1900) && ++retry < retry_count)
    {
        ESP_LOGD(tag, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry == retry_count)
    {
        ESP_LOGE(tag, "Failed to obtain time.");
        return;
    }

    ESP_LOGI(tag, "Time synchronized.");
}

// Function to set timezone
void set_timezone(const char *timezone)
{
    ESP_LOGI(tag, "Setting timezone to: %s", timezone);
    setenv("TZ", timezone, 1); // Set the TZ environment variable
    tzset();                   // Apply the timezone change
}

// Print current calendar information
void print_calendar()
{
    time_t now;
    struct tm timeinfo;
    char buffer[64];

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(buffer, sizeof(buffer), "Date: %Y-%m-%d, Time: %H:%M:%S", &timeinfo);
    ESP_LOGI(tag, "%s", buffer);
    ESP_LOGI(tag, "Weekday: %d (0=Sunday, 6=Saturday)", timeinfo.tm_wday);
    ESP_LOGI(tag, "Day of the year: %d", timeinfo.tm_yday + 1);
}

static void timeTask(void *pc)
{
    // Synchronize time
    obtain_time();

    while (1)
    {
        print_calendar();
        vTaskDelay(pdMS_TO_TICKS(300000)); // Print calendar every 5 minutes
    }
}

extern "C" void app_main()
{
    // Connect WiFi
	ESP_LOGI(tag, "Projekt cpp_test - Wifi");
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for Wi-Fi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Get the WifiManager singleton
    auto& wifiManager = WifiManager::GetInstance();
    // Initialize with configuration
    WifiManagerConfig config;
    config.ssid_prefix = "ESP32";  // AP mode SSID prefix
    config.language = "de-DE";     // Web UI language
    wifiManager.Initialize(config);
    // Set event callback to handle WiFi events
    wifiManager.SetEventCallback([](WifiEvent event) {
        switch (event) {
            case WifiEvent::Scanning:
                //ESP_LOGI("WiFi", "Scanning for networks...");
                break;
            case WifiEvent::Connecting:
                //ESP_LOGI("WiFi", "Connecting to network...");
                break;
            case WifiEvent::Connected:
                //ESP_LOGI("WiFi", "Connected successfully!");
                break;
            case WifiEvent::Disconnected:
                //ESP_LOGW("WiFi", "Disconnected from network");
                break;
            case WifiEvent::ConfigModeEnter:
                //ESP_LOGI("WiFi", "Entered config mode");
                break;
            case WifiEvent::ConfigModeExit:
                //ESP_LOGI("WiFi", "Exited config mode");
                break;
        }
    });

    // Check if there are saved Wi-Fi credentials
    auto& ssid_list = SsidManager::GetInstance().GetSsidList();
    if (ssid_list.empty()) {
        // No credentials saved, start config AP mode
        startConfigurationAp = true;
        wifiManager.StartConfigAp();
    } else {
        // Try to connect to the saved Wi-Fi network
        startStation = true;
        wifiManager.StartStation();
    }

    // hier muss wifiManager.StartConfigAp bzw StartSession aufgerufen werden
    ESP_LOGI(tag, "Projekt cpp_test - WifiConfiguration Start startConfigurationAp=%s startStation=%s", startConfigurationAp ? "true":"false", startStation ? "true":"false");

    // if not connected then wait 20 seconds for BUTTON_LONG_PRESS_UP (5000 ms) to enter Wifi configuration mode
    for (int i=20;i>0;i--) {
        // wenn nach 20 sek. die WiFi-Station noch nicht connected ist, dann wird
        // der Configuration Access Point gestartet

        if (startStation == true && wifiManager.IsConnected()) {
            break;
        }
        printf("Programstart in %2i Sekunden (Button für 5 sek. drücken, um die Wifi-Konfiguration zu starten)\n", i);
        // 1 Sekunde warten
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (startStation == true && !wifiManager.IsConnected()) {
        startStation = false;
        startConfigurationAp = true;
        printf("Die Wifi-Konfiguration wird gestartet\n");
        wifiManager.StartConfigAp();
    }

    while(startStation == false && !wifiManager.IsConnected()) {
        printf("1 Sekunde warten, dann prüfen, ob WifiStation gestartet ist\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // ende wifi

    printf("InfluxDB-Client starten\n");

    initialize_sntp();
    // Set timezone (e.g., Asia/Kolkata for UTC+5:30)
    set_timezone("CET");
    xTaskCreate(timeTask, "time_task", 4096, NULL, 5, NULL);


    // InfluxDB client instance
    InfluxDBClient client;
    client.setConnectionParams(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
    //client.setInsecure(true);

    while(!client.validateConnection()) {
        printf("1 Sekunde warten, ob InfluxDB-Client-Connection valid\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Data point
    Point sensor("wifi_status");


    // Set InfluxDB 1 authentication params
    //client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);

    // Add constant tags - only once
    sensor.addTag("device", DEVICE);
    sensor.addTag("SSID", wifiManager.GetSsid().c_str());

    // Check server connection
    if (client.validateConnection()) {
      printf("Connected to InfluxDB: %s\n", client.getServerUrl().c_str());
    } else {
      printf("InfluxDB connection failed: %s\n", client.getLastErrorMessage().c_str());
    }

    while(true) {
        // Store measured value into point
        sensor.clearFields();
        // Report RSSI of currently connected network
        sensor.addField("rssi", wifiManager.GetRssi());
        // Print what are we exactly writing
        printf("Writing: %s\n", client.pointToLineProtocol(sensor).c_str());
        // If no Wifi signal, try to reconnect it
        if (!wifiManager.IsConnected()) {
            printf("Wifi connection lost\n");
            // Check server connection
            if (client.validateConnection()) {
              printf("(Re-)Connected to InfluxDB: %s\n", client.getServerUrl().c_str());
            } else {
              printf("InfluxDB connection failed: %s\n", client.getLastErrorMessage().c_str());
            }
        }
        // Write point
        if (!client.writePoint(sensor)) {
          printf("InfluxDB write failed: %s\n", client.getLastErrorMessage().c_str());
        }
           //Wait 10s
        printf("Wait 10s\n");
        vTaskDelay(pdMS_TO_TICKS(10000)); // delay 10 seconds
    }
}