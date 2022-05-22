#include "wifiProvider.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include "../memory/memory_provider.h"

#define KEY_CREDENTIALS "/wifi/credentials"
#define JSON_STEP 3

String WiFiProvider::getSettingKey()
{
    return SETTINGS_WIFI_KEY;
}

void WiFiProvider::parseValue(String key, String value)
{
    printlnD("WiFi parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);
    if (key.startsWith(PREFIX_SETTINGS + String(KEY_CREDENTIALS)))
    {
        parseCredentialsValue(key, value);
    }
}

void WiFiProvider::parseJson(FirebaseJson *json, String path)
{
    printlnA("WIFI - parseJson");
    printlnA("Path = " + path);

    // whole json with all settings - must select only wifi

    FirebaseJson *credentialsJson = new FirebaseJson();

    if (path.equals(PREFIX_SETTINGS))
    {
        FirebaseJsonData jsonData;
        json->get(jsonData, KEY_CREDENTIALS, true);
        jsonData.getJSON(*credentialsJson);
        printlnA("All settings -> parsed: ");
        parseCredentialsJson(credentialsJson);
    }

    // Only one credentials changed
    if (path.equals(PREFIX_SETTINGS + String(KEY_CREDENTIALS)))
    {
        String buffer;
        json->toString(buffer, true);
        credentialsJson = json;
        printlnA("One credentials -> parsed: ");
        parseCredentialsJson(credentialsJson, false);
    }
}

void WiFiProvider::parseCredentialsJson(FirebaseJson *json, bool deleteMissing)
{
    printlnA("Iterating through credentials json");
    std::vector<String> toDelete;

    // 1. check all existing triggers;
    for (auto &&it : _credentials)
    {
        printlnA("Checking key = " + it.first);
        FirebaseJsonData credentialsData;

        // If triggers with key exists in json
        if (json->get(credentialsData, "/" + it.first, true))
        {

            printlnA("Found json");
            FirebaseJson innerJson;
            credentialsData.getJSON(innerJson);
            bool triggerChanged = updateCredentialsFromJson(it.second, &innerJson);

            if (triggerChanged)
            {
                saveCredentialsToNVS(it.second);
            }
        }
        else
        {
            if (deleteMissing)
            {
                printlnA("Deleting trigger " + it.first);
                toDelete.push_back(it.first);
            }
        }
    }

    if (deleteMissing)
    {
        printlnA("Deleting all missing triggers")
            // Delete all marked triggers
            for (String s : toDelete)
        {
            removeCredentials(s);
        }
    }

    // check for new triggers
    createNewCredentialsFromJson(json);
}

void WiFiProvider::parseCredentialsJson(FirebaseJson *json, String path)
{
    FirebaseJsonData jsonData;
    printlnI("WiFi  parseCredentialsJson");
    int index = path.lastIndexOf("/");
    String credentialKey = path.substring(index + 1);

    printlnV("Parse credentials json callback");
    printlnV("CredentialKey = " + credentialKey);

    auto it = _credentials.find(credentialKey);
    if (it == _credentials.end())
    {
        createCredentialsFromJson(json, credentialKey);
    }
}

void WiFiProvider::createCredentialsFromJson(FirebaseJson *json, String key)
{
    printlnA("create new credentials from this json: ");
    String buffer;
    json->toString(buffer, true);
    printlnA(buffer);
    auto credentials = std::make_shared<WifiCredentials>();
    FirebaseJsonData jsonData;
    key.toCharArray(credentials->firebaseKey, FIREBASE_LENGTH);

    if (json->get(jsonData, "/ssid", false))
    {
        jsonData.stringValue.toCharArray(credentials->ssid, SSID_LENGTH);
    }
    else
    {
        printlnW("No ssid in json");
        return;
    }

    if (json->get(jsonData, "/pass", false))
    {
        jsonData.stringValue.toCharArray(credentials->pass, SSID_LENGTH);
    }
    else
    {
        printlnW("No pass in json");
        return;
    }

    credentials->storageId = getAvailableMemoryId();

    if (credentials->storageId != INVALID_MEMORY_ID)
    {
        printlnA("NO error... Proceed to insert new credentials");
        printCredentials(credentials);
        _credentials.insert({key, credentials});
        saveCredentialsToNVS(credentials);
    }
}

void WiFiProvider::createNewCredentialsFromJson(FirebaseJson *json)
{

    printlnA("Create new credentials from json");
    int type;
    String key;
    String value;
    FirebaseJsonData innerData;
    FirebaseJson innerJson;

    int len = json->iteratorBegin();
    if (len % JSON_STEP == 0)
    {
        printlnA("JSON Valid");
        for (int i = 0; i < len; i += JSON_STEP)
        {
            json->iteratorGet(i, type, key, value);
            printlnA("Key = " + key);
            auto it = _credentials.find(key);
            if (it == _credentials.end()) // Check if map contain this key
            {
                printlnA("No credentials found, creating new one!") if (type == FirebaseJson::JSON_OBJECT)
                {
                    innerJson.setJsonData(value);
                    createCredentialsFromJson(&innerJson, key);
                }
            }
        }
    }
}

bool WiFiProvider::updateCredentialsFromJson(std::shared_ptr<WifiCredentials> credentials, FirebaseJson *json)
{

    bool changed = false;
    FirebaseJsonData data;

    if (json->get(data, "/pass"))
    {
        data.stringValue.toCharArray(credentials->pass, PASS_LENGTH);
        changed = true;
    }

    if (json->get(data, "/ssid"))
    {
        data.stringValue.toCharArray(credentials->ssid, PASS_LENGTH);
        changed = true;
    }

    return changed;
}

void WiFiProvider::parseCredentialsValue(String key, String value)
{
    debugA("Parsing value, key = %s, value = %s\n", key.c_str(), value.c_str());

    int index = key.lastIndexOf("/");

    String jsonKey = key.substring(index + 1);
    String credentialKey = key.substring(0, index);
    credentialKey = credentialKey.substring(credentialKey.lastIndexOf("/") + 1);

    printlnA("jsonKey = " + jsonKey);
    printlnA("credentialKey = " + credentialKey);
    printlnA("value = " + value);

    if (value == "null")
    {
        printlnD("Removing key");
        removeCredentials(jsonKey);
    }
    else
    {
        auto it = _credentials.find(credentialKey);
        if (it != _credentials.end())
        {
            debugA("Found credentials");
            printCredentials(it->second);
            if (jsonKey.equals("pass"))
            {
                strcpy(it->second->pass, value.c_str());
            }

            if (jsonKey.equals("ssid"))
            {
                strcpy(it->second->ssid, value.c_str());
            }
            printCredentials(it->second);
            saveCredentialsToNVS(it->second);
        }
    }
}

void WiFiProvider::removeCredentials(String firebaseKey)
{
    auto it = _credentials.find(firebaseKey);

    if (it != _credentials.end())
    {
        printlnA("Removing trigger - " + firebaseKey);
        if (memoryProvider != nullptr)
        {
            memoryProvider->removeKey(String(WIFI_MEMORY_PREFIX) + String(it->second->storageId));
        }
        freeMemoryId(it->second->storageId);
        _credentials.erase(it); // remove record from map
    }
}

void WiFiProvider::freeMemoryId(int id)
{
    if (id < MAX_CREDENTIALS && id >= 0)
    {
        availableIds[id] = true;
    }
}