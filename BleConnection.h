#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>

#include "BleScanner.h"

class BleConnection {
    BleScanner& scanner;

    winrt::Windows::Devices::Bluetooth::BluetoothLEDevice m_device{ nullptr };
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService m_service{ nullptr };

    std::string connectedAddress;
    std::string serviceUuid;

    std::unordered_map<std::string, std::string> charMap;

    std::string ResolveUuid(const std::string& alias) const;

    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic
        GetCharacteristic(const std::string& uuid);

public:
    BleConnection(BleScanner& scanner);

    bool Connect(const std::string& address, const std::string& pin);
    void Disconnect();
    bool IsConnected() const;
    std::vector<std::string> GetCharacteristicAliases();
    std::string GetConnectedAddress() const;

    std::string Read(const std::string& alias);
    bool Write(const std::string& alias, const std::string& value);
};
