
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>

struct BleDevice
{
    std::string name;
    std::string address;
};

class BleScanner
{
public:
    BleScanner();
    ~BleScanner();

    void Start();
    void Stop();

    bool IsScanning() const;
    std::vector<BleDevice> GetDevices() const;

private:
    void OnAdvertisementReceived(
        winrt::Windows::Devices::Bluetooth::Advertisement::
        BluetoothLEAdvertisementWatcher const& sender,
        winrt::Windows::Devices::Bluetooth::Advertisement::
        BluetoothLEAdvertisementReceivedEventArgs const& args);

    winrt::Windows::Devices::Bluetooth::Advertisement::
        BluetoothLEAdvertisementWatcher m_watcher{ nullptr };

    winrt::event_token m_receivedToken{};

    mutable std::mutex m_mutex;
    std::vector<BleDevice> m_devices;
    std::unordered_set<std::uint64_t> m_seenAddresses;
    bool m_isScanning = false;
};
