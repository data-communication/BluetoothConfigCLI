#include "BleConnection.h"
#include <iostream>
#include <sstream>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Enumeration.h> 
#include <winrt/Windows.Storage.Streams.h>

BleConnection::BleConnection(BleScanner& scanner)
    : scanner(scanner)
{
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    serviceUuid = "9b43d9ad-bbac-4bf2-bfbc-715c00000000";

    charMap = {
        {"device_id",               "9b43d9ad-bbac-4bf2-bfbc-715c00000001"},
        {"device_location",         "9b43d9ad-bbac-4bf2-bfbc-715c00000002"},
        {"measurement_interval",    "9b43d9ad-bbac-4bf2-bfbc-715c00000003"},

        {"wifi_ssid",               "9b43d9ad-bbac-4bf2-bfbc-715c00000004"},
        {"wifi_password",           "9b43d9ad-bbac-4bf2-bfbc-715c00000005"},

        {"mqtt_username",           "9b43d9ad-bbac-4bf2-bfbc-715c00000006"},
        {"mqtt_password",           "9b43d9ad-bbac-4bf2-bfbc-715c00000007"},
        {"mqtt_host",               "9b43d9ad-bbac-4bf2-bfbc-715c00000008"},
        {"mqtt_topic",              "9b43d9ad-bbac-4bf2-bfbc-715c00000009"},
        {"mqtt_port",               "9b43d9ad-bbac-4bf2-bfbc-715c00000010"},

        {"command",                 "9b43d9ad-bbac-4bf2-bfbc-715c00000011"},
        {"result",                  "9b43d9ad-bbac-4bf2-bfbc-715c00000012"}
    };
}

static uint64_t ParseBluetoothAddress(const std::string& hex)
{
    uint64_t addr = 0;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> addr;
    return addr;
}

bool BleConnection::Connect(const std::string& mac, const std::string& pin)
{
    std::cout << "Connecting to " << mac << "...\n";

    uint64_t btAddr = ParseBluetoothAddress(mac);

    auto dev = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(btAddr).get();
    if (!dev)
    {
        std::cout << "Initial connection failed.\n";
        return false;
    }

    auto pairing = dev.DeviceInformation().Pairing();
    auto custom = pairing.Custom();

    custom.PairingRequested([pin](auto const&, auto const& args) {
        args.Accept(winrt::to_hstring(pin));
        });

    using Kind = winrt::Windows::Devices::Enumeration::DevicePairingKinds;
    using Level = winrt::Windows::Devices::Enumeration::DevicePairingProtectionLevel;
    using Status = winrt::Windows::Devices::Enumeration::DevicePairingResultStatus;

    auto result = custom.PairAsync(
        Kind::ProvidePin,
        Level::EncryptionAndAuthentication
    ).get();

    if (result.Status() != Status::Paired &&
        result.Status() != Status::AlreadyPaired)
    {
        std::cout << "Pairing failed.\n";
        return false;
    }

    std::cout << "Paired successfully.\n";

    dev = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(btAddr).get();
    if (!dev)
    {
        std::cout << "Authenticated reconnect failed.\n";
        return false;
    }

    m_device = dev;

    auto servicesResult = m_device.GetGattServicesAsync().get();
    if (servicesResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
    {
        std::cout << "Failed to get GATT services.\n";
        return false;
    }
    
    for (auto const& svc : servicesResult.Services())
    {
        if (svc.Uuid() == winrt::guid(serviceUuid))
        {
            m_service = svc;
            connectedAddress = mac;
            std::cout << "Connected.\n";
            return true;
        }
    }

    std::cout << "Service UUID not found.\n";
    return false;
}

std::string  BleConnection::GetConnectedAddress() const
{
	return connectedAddress;
}

void BleConnection::Disconnect()
{
    if (!IsConnected())
        return;

    try {
        if (m_service) {
            m_service.Close();
            m_service = nullptr;
        }

        if (m_device) {
            m_device.Close();
            m_device = nullptr;
        }
    }
    catch (...) {
        std::cout << "Warning: exception during disconnect.\n";
    }

    connectedAddress.clear();
    std::cout << "Disconnected.\n";
}

bool BleConnection::IsConnected() const
{
    return m_device != nullptr;
}

std::string BleConnection::ResolveUuid(const std::string& alias) const
{
    auto it = charMap.find(alias);
    if (it == charMap.end()) return "";
    return it->second;
}

winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic
BleConnection::GetCharacteristic(const std::string& uuid)
{
    auto target = winrt::guid(uuid);

    auto charsResult = m_service.GetCharacteristicsAsync().get();
    if (charsResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
        return nullptr;

    for (auto const& ch : charsResult.Characteristics())
    {
        if (ch.Uuid() == target)
            return ch;
    }

    return nullptr;
}

std::string BleConnection::Read(const std::string& alias)
{
    if (!IsConnected()) {
        std::cout << "Not connected.\n";
        return "";
    }

    auto uuid = ResolveUuid(alias);
    if (uuid.empty()) {
        std::cout << "Unknown alias: " << alias << "\n";
        return "";
    }

    auto ch = GetCharacteristic(uuid);
    if (!ch) {
        std::cout << "Characteristic not found.\n";
        return "";
    }

    auto result = ch.ReadValueAsync().get();
    if (result.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
        std::cout << "Read failed.\n";
        return "";
    }

    auto buffer = result.Value();
    auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);

    std::vector<uint8_t> bytes(reader.UnconsumedBufferLength());
    reader.ReadBytes(bytes);

    return std::string(bytes.begin(), bytes.end());
}

bool BleConnection::Write(const std::string& alias, const std::string& value)
{
    if (!IsConnected()) {
        std::cout << "Not connected.\n";
        return false;
    }

    auto uuid = ResolveUuid(alias);
    if (uuid.empty()) {
        std::cout << "Unknown alias: " << alias << "\n";
        return false;
    }

    auto ch = GetCharacteristic(uuid);
    if (!ch) {
        std::cout << "Characteristic not found.\n";
        return false;
    }

    winrt::Windows::Storage::Streams::DataWriter writer;
    writer.WriteString(winrt::to_hstring(value));

    auto status = ch.WriteValueAsync(writer.DetachBuffer()).get();

    if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
        std::cout << "Write failed.\n";
        return false;
    }

    return true;
}

std::vector<std::string> BleConnection::GetCharacteristicAliases()
{
    std::vector<std::string> result;
    for (const auto& [alias, uuid] : charMap) {
        result.push_back(alias + " " + uuid);
    }
    return result;
}
