#pragma once
#include <string>
#include "BleScanner.h"

class CommandParser {
    BleScanner& scanner;

    void CmdScan();
    void CmdStop();
    void CmdList();
    void CmdHelp();

    public:
        CommandParser(BleScanner& scanner);

        bool HandleCommand(const std::string& command);
};
