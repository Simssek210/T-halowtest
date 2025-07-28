/*
Original code by LilyGo
Modified by me 

*/


#include "utilities.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "esp_camera.h"

#define BUF_MAX_LEN 20

#define AH_Rx00P_RESPONE_OK 1
#define AH_Rx00P_RESPONE_ERROR 2

bool tx_ah_ret = false;
char buf[BUF_MAX_LEN] = {0};
char recv_data[128] = {0};

//************************************[ TX-AH ]******************************************
#if 1
int8_t waitResponse(uint32_t timeouts, String &data, const char *r1 = "OK", const char *r2 = "ERROR")
{
    int index = 0;
    uint32_t start_tick = millis();

    do
    {
        while (SerialAT.available() > 0)
        {
            int a = SerialAT.read();
            if (a < 0)
                continue; // Skip 0x00 bytes, just in case

            data.reserve(1024);
            data += static_cast<char>(a);

            // SerialMon.println(data.c_str());

            if (data.endsWith(r1))
            {
                index = AH_Rx00P_RESPONE_OK;
                SerialMon.println(data.c_str());
                goto finish;
            }
            else if (data.endsWith(r2))
            {
                index = AH_Rx00P_RESPONE_ERROR;
                SerialMon.println(data.c_str());
                goto finish;
            }
        }
    } while (millis() - start_tick < timeouts);

finish:
    return index;
}

int8_t waitResponse(uint32_t timeouts)
{
    String data;
    return waitResponse(timeouts, data);
}
int8_t waitResponse(void)
{
    return waitResponse(1000);
}
void sendAT(String s)
{
    s = "AT" + s;
    SerialAT.write(s.c_str());
}
bool TX_AH_init(void)
{
    int at_cnt = 0;

    sendAT("+SYSDBG=LMAC,0");
    if (waitResponse() == AH_Rx00P_RESPONE_OK)
        SerialMon.println("AT+SYSDBG SUCCEED");
    else
    {
        at_cnt++;
        SerialMon.println("AT+SYSDBG ERROR");
    }

    sendAT("+BSS_BW=8");
    if (waitResponse() == AH_Rx00P_RESPONE_OK)
        SerialMon.println("AT+BSS_BW SUCCEED");
    else
    {
        at_cnt++;
        SerialMon.println("AT+BSS_BW FAILD");
    }

    sendAT("+MODE=STA");
    if (waitResponse() == AH_Rx00P_RESPONE_OK)
        SerialMon.println("AT+MODE=STA SUCCEED");
    else
    {
        at_cnt++;
        SerialMon.println("AT+MODE=STA FAILD");
    }

    return (at_cnt == 0);
}
#endif

//************************************[ Other fun ]******************************************
char *line_align(char *buf, const char *str1, const char *str2)
{
    int max_line_size = BUF_MAX_LEN - 1;
    int16_t w2 = strlen(str2);
    int16_t w1 = max_line_size - w2;
    snprintf(buf, BUF_MAX_LEN, "%-*s%-*s", w1, str1, w2, str2);
    return buf;
}

bool tx_ah_conn_status = false;
char rssi_buf[16];

void setup()
{
    Serial.begin(115200);

    delay(3000);

    SerialAT.begin(115200, SERIAL_8N1, SERIAL_AT_RXD, SERIAL_AT_TXD);

    tx_ah_ret = TX_AH_init();

    Serial.println("******************************");
    Serial.println((tx_ah_ret == true ? "TX-AH   PASS" : "TX-AH    ---"));
    Serial.println(" ");

    Serial.println(line_align(buf, "Role:", "STA "));

    if (tx_ah_conn_status)
    {
        Serial.println(line_align(buf, "RSSI:", rssi_buf));
    }
    else
    {
        Serial.println("Disconnect!!!");
    }
}

uint32_t last_tick = 0;
uint32_t rssi_tick = 0;

String recv_str = "";
int indx = 0;
bool led_flag = 0;

void loop()
{
    if (millis() - last_tick > 1000)
    {
        last_tick = millis();
        digitalWrite(BOARD_LED, led_flag);
        led_flag = !led_flag;
    }

    if (millis() - rssi_tick > 3000)
    {
        rssi_tick = millis();

        String data;
        sendAT("+CONN_STATE");
        if (waitResponse(1000, data, "+CONNECTED", "+DISCONNECT") == AH_Rx00P_RESPONE_OK)
        {
            tx_ah_conn_status = true;
        }
        else
        {
            tx_ah_conn_status = false;
        }

        if (tx_ah_conn_status)
        {
            String rssi_data;
            sendAT("+RSSI=1");
            if (waitResponse(1000, rssi_data) == AH_Rx00P_RESPONE_OK)
            {
                int startIndex = rssi_data.indexOf(':');
                int endIndex = rssi_data.lastIndexOf('\n');
                String substr = rssi_data.substring(startIndex + 1, endIndex - 1);
                strcpy(rssi_buf, substr.c_str());
            }
        }

        Serial.println("******************************");
        Serial.println((tx_ah_ret == true ? "TX-AH   PASS" : "TX-AH    ---"));
        Serial.println(" ");

        Serial.println(line_align(buf, "Role:", "STA "));

        if (tx_ah_conn_status)
        {
            Serial.println(line_align(buf, "RSSI:", rssi_buf));
        }
        else
        {
            Serial.println("Disconnect!!!");
        }
    }

    while (SerialAT.available())
    {
        // SerialMon.write(SerialAT.read());

        recv_str += (char)SerialAT.read();
        // SerialMon.write(recv_str.c_str());
    }
    while (SerialMon.available())
    {
        SerialAT.write(SerialMon.read());
    }

    if (recv_str.equals("") == 0)
    {
        if (recv_str.startsWith("+RXDATA:"))
        {
            char *p = (char *)recv_str.c_str();

            while (*p != '\n')
            {
                *p++;
            }

            Serial.print("recv:");
            Serial.println((p + 15));
            memcpy(recv_data, (p + 15), 128);
        }
        SerialMon.write(recv_str.c_str());
        recv_str = "";
    }
    delay(1);
}
