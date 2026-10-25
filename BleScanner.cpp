
#include "BleScanner.h"

#include <iomanip>
#include <sstream>
#include <utility>

#include <winrt/Windows.Foundation.h>

#pragma comment(lib, "runtimeobject.lib")

using namespace winrt;
using namespace Windows::Devices::Bluetooth::Advertisement;

BleScanner::BleScanner() {
    init_apartment(apartment_type::multi_threaded);
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

    m_watcher = BluetoothLEAdvertisementWatcher();
    m_watcher.ScanningMode(BluetoothLEScanningMode::Active);

    m_receivedToken = m_watcher.Received(
        [this](
            BluetoothLEAdvertisementWatcher const& sender,
            BluetoothLEAdvertisementReceivedEventArgs const& args)
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
    BluetoothLEAdvertisementWatcher const&,
    BluetoothLEAdvertisementReceivedEventArgs const& args)
{
    const auto address = args.BluetoothAddress();

    std::string name = winrt::to_string(
        args.Advertisement().LocalName());

    std::wstringstream addressStream;
    addressStream << std::hex << std::uppercase
        << std::setfill(L'0')
        << std::setw(12) << address;

    std::string addressString =
        winrt::to_string(addressStream.str());

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
