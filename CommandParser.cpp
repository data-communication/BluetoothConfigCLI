#include "CommandParser.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

CommandParser::CommandParser(BleScanner& scanner)
    : scanner(scanner)
{
}

bool CommandParser::HandleCommand(const std::string& raw) {
    std::string command = raw;

    std::transform(command.begin(), command.end(), command.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (command == "scan" || command == "start" || command == "start scan") {
        CmdScan();
    }
    else if (command == "stop" || command == "stop scan") {
        CmdStop();
    }
    else if (command == "list") {
        CmdList();
    }
    else if (command == "help") {
        CmdHelp();
    }
    else if (command == "exit") {
        scanner.Stop();
        return false;
    }
    else {
        std::cout << "Unknown command. Type 'help'.\n";
    }

    return true;
}

void CommandParser::CmdScan() {
    if (scanner.IsScanning())
        std::cout << "Already scanning.\n";
    else {
        scanner.Start();
        std::cout << "Scanning started.\n";
    }
}

void CommandParser::CmdStop()
{
    if (scanner.IsScanning()) {
        scanner.Stop();
        std::cout << "Scanning stopped.\n";
    }
    else {
        std::cout << "Not currently scanning.\n";
    }
}

void CommandParser::CmdList() {
    auto devices = scanner.GetDevices();

    if (devices.empty()) {
        std::cout << "No devices found.\n";
        return;
    }

    for (size_t i = 0; i < devices.size(); ++i){
        std::string index = "[" + std::to_string(i) + "]";

        std::cout << std::left << std::setw(5) << index
            << "| " << std::left << std::setw(20) << devices[i].name
            << "| " << devices[i].address
            << "\n";
    }
}

void CommandParser::CmdHelp() {
    std::cout
        << std::left << std::setw(30) << "scan / start / start scan"
        << "- Start scanning\n"

        << std::left << std::setw(30) << "stop / stop scan"
        << "- Stop scanning\n"

        << std::left << std::setw(30) << "list"
        << "- List discovered devices\n"

        << std::left << std::setw(30) << "help"
        << "- Show this help\n"

        << std::left << std::setw(30) << "exit"
        << "- Quit program\n";
}
