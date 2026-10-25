#include "BleScanner.h"

#include <iomanip>
#include <sstream>
#include <utility>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>

#pragma comment(lib, "runtimeobject.lib")

BleScanner::BleScanner() {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
}

BleScanner::~BleScanner() {
    Stop();
}

void BleScanner::Start() {
    std::lock_guard lock(m_mutex);

    if (m_isScanning)
        return;

    m_devices.clear();
    m_seenAddresses.clear();

    m_watcher = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher();
    m_watcher.ScanningMode(
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEScanningMode::Active
    );

    m_receivedToken = m_watcher.Received(
        [this](
            winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& sender,
            winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args)
        {
            OnAdvertisementReceived(sender, args);
        });

    try {
        m_watcher.Start();
        m_isScanning = true;
    }
    catch (...) {
        m_watcher.Received(m_receivedToken);
        m_watcher = nullptr;
        throw;
    }
}

void BleScanner::Stop() {
    std::lock_guard lock(m_mutex);

    if (!m_isScanning)
        return;

    m_isScanning = false;

    m_watcher.Received(m_receivedToken);
    m_watcher.Stop();
    m_watcher = nullptr;
}

bool BleScanner::IsScanning() const {
    std::lock_guard lock(m_mutex);
    return m_isScanning;
}

std::vector<BleDevice> BleScanner::GetDevices() const {
    std::lock_guard lock(m_mutex);
    return m_devices;
}

void BleScanner::OnAdvertisementReceived(
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const&,
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args)
{
    const auto address = args.BluetoothAddress();

    std::string name = winrt::to_string(args.Advertisement().LocalName());

    std::wstringstream addressStream;
    addressStream << std::hex << std::uppercase
        << std::setfill(L'0')
        << std::setw(12) << address;

    std::string addressString = winrt::to_string(addressStream.str());

    std::lock_guard lock(m_mutex);

    if (!m_isScanning)
        return;

    if (!m_seenAddresses.insert(address).second) {
        for (auto& device : m_devices) {
            if (device.address == addressString) {
                if (!name.empty())
                    device.name = name;
                return;
            }
        }
        return;
    }

    if (name.empty())
        name = "(Unknown)";

    m_devices.push_back({
        std::move(name),
        std::move(addressString)
        });
}