#include "CommandParser.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

CommandParser::CommandParser(BleScanner& scanner, BleConnection& connection)
    : scanner(scanner), connection(connection)
{
}

static std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;

    for (char c : s) {
        if (c == ' ') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        }
        else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);

    return out;
}

bool CommandParser::HandleCommand(const std::string& raw) {
    auto args = split(raw);
    if (args.empty()) return true;

    if (args[0] == "scan") CmdScan(args);
    else if (args[0] == "list") CmdList(args);
    else if (args[0] == "connect") CmdConnect(args);
    else if (args[0] == "disconnect") CmdDisconnect();
    else if (args[0] == "read") CmdRead(args);
    else if (args[0] == "write") CmdWrite(args);
    else if (args[0] == "help") CmdHelp();
    else if (args[0] == "status") CmdStatus();
    else if (args[0] == "exit") return false;
    else std::cout << "Unknown command\n";

    return true;
}

void CommandParser::CmdStatus() {
	if (connection.IsConnected()) {
		std::cout << "Connected to " << connection.GetConnectedAddress() << " | ";
	}
	else {
		std::cout << "Not connected to any device. | ";
	}
	if (scanner.IsScanning()) {
		std::cout << "Scanning for devices.\n";
	}
	else {
		std::cout << "Not currently scanning.\n";
	}
}

void CommandParser::CmdScan(const std::vector<std::string>&args) {
    if (args.size() < 2) {
        std::cout << "Usage: scan start | scan stop\n";
        return;
    }

    if (args[1] == "start") {
		if (scanner.IsScanning()) {
			std::cout << "Already scanning.\n";
			return;
		}
        scanner.Start();
        std::cout << "Scanning started.\n";
    }
    else if (args[1] == "stop") {
		if (!scanner.IsScanning()) {
			std::cout << "Not currently scanning.\n";
			return;
		}
        scanner.Stop();
        std::cout << "Scanning stopped.\n";
    }
}

void CommandParser::CmdList(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: list devices | list characteristics\n";
        return;
    }

    if (args[1] == "devices") {
        auto devices = scanner.GetDevices();
        for (size_t i = 0; i < devices.size(); ++i) {
			std::string index = "[" + std::to_string(i) + "]";
            std::cout << std::left << std::setw(5) << index
                << "| " 
                << std::left << std::setw(24) << devices[i].name << "| "
                << std::left << std::setw(20) << devices[i].address << "\n";
        }
    }
	else if (args[1] == "characteristics") {
        for (const auto& characteristic : connection.GetCharacteristicAliases()) {
			std::vector<std::string> parts = split(characteristic);
            std::cout << std::left << std::setw(25) << parts[0] << "| " << parts[1] << "\n";
        }
    }
	else {
		std::cout << "Unknown list command: " << args[1] << "\n";
	}
}

void CommandParser::CmdConnect(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: connect <index> <pin>\n";
        return;
    }

    size_t index;
    try {
        index = std::stoul(args[1]);
    }
    catch (const std::exception&) {
        std::cout << "Invalid device index.\n";
        return;
    }

    auto devices = scanner.GetDevices();
    if (index >= devices.size()) {
        std::cout << "Device not found.\n";
        return;
    }

    if (args.size() < 3) {
        std::cout << "Usage: connect <index> <pin>\n";
        return;
    }

    connection.Connect(devices[index].address, args[2]);
}

void CommandParser::CmdDisconnect() {
	connection.Disconnect();
}

void CommandParser::CmdRead(const std::vector<std::string>& args) {
	if (args.size() < 2) {
		std::cout << "Usage: read <alias>\n";
		return;
	}
	std::string value = connection.Read(args[1]);
	if (!value.empty()) {
		std::cout << "Read value: " << value << "\n";
	}
	else {
		std::cout << "Failed to read value.\n";
	}
}

void CommandParser::CmdWrite(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: write <alias> <value>\n";
        return;
    }
    if (connection.Write(args[1], args[2])) {
        std::cout << "Write successful.\n";
    }
    else {
        std::cout << "Failed to write value.\n";
    }
}

void CommandParser::CmdHelp() {
    std::cout
        << std::left << std::setw(30) << "scan start"
        << "- Start scanning for BLE devices\n"

        << std::left << std::setw(30) << "scan stop"
        << "- Stop scanning\n"

        << std::left << std::setw(30) << "list devices"
        << "- List discovered BLE devices\n"

        << std::left << std::setw(30) << "list characteristics"
        << "- List characteristic aliases of the connected device\n"

        << std::left << std::setw(30) << "connect <index> <pin>"
        << "- Connect to a device by list index using the provided pairing PIN\n"

        << std::left << std::setw(30) << "disconnect"
        << "- Disconnect from the current device\n"

        << std::left << std::setw(30) << "read <alias>"
        << "- Read a characteristic using its alias\n"

        << std::left << std::setw(30) << "write <alias> <value>"
        << "- Write a value to a characteristic using its alias\n"

        << std::left << std::setw(30) << "help"
        << "- Show this help menu\n"

        << std::left << std::setw(30) << "exit"
        << "- Quit program\n";
}
