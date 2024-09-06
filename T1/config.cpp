#include "config.hpp"
#include "log.hpp"

#include <FS.h>
#include <SPIFFS.h>

std::map<String, String> mainConfig;
bool validateConfig(const std::map<String, String> &cfg) {
    if (cfg.size() == NUM_PARAMETERS) {
        for (const auto &it : cfg)
            if (it.second.length() == 0) return false;

        return true;
    }
    return false;
}

/*
 * Carrega as configurações presentes em CONFIG_FILE
 */
bool carregaConfig() {
    File file = SPIFFS.open(CONFIG_FILE, "r");
    String line;
    if (file) {
        while ((line = file.readStringUntil('\n')).length() > 0) {
            line.trim();
            int index;
            if ((index = line.indexOf('=')) > 0) {
                String key = line.substring(0, index);
                String value = line.substring(index + 1);
                mainConfig[key] = (value);
            }
        }
        file.close();
        return validateConfig(mainConfig);
    }
    return false;
}

/*
 * Grava as configurações em CONFIG_FILE
 */
void aplicaConfig() {
    Serial.println("Aplicando novas configurações...");
    File file = SPIFFS.open(CONFIG_FILE, "w");
    file.println(String(KEY_STA_SSID) + "=" + mainConfig[KEY_STA_SSID]);
    file.println(String(KEY_STA_PASS )+ "=" + mainConfig[KEY_STA_PASS]);
    file.println(String(KEY_ADMIN_LOGIN) + "=" + mainConfig[KEY_ADMIN_LOGIN]);
    file.println(String(KEY_ADMIN_PASS) + "=" + mainConfig[KEY_ADMIN_PASS]);
    file.println(String(KEY_AP_SSID) + "=" + mainConfig[KEY_AP_SSID]);
    file.println(String(KEY_AP_PASS) + "=" + mainConfig[KEY_AP_PASS]);
    file.close();
    Serial.println(INFO_UPDATE);
    ESP.restart();
}