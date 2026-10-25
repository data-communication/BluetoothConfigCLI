#pragma once
#include <string>
#include "BleScanner.h"
#include "BleConnection.h"

class CommandParser {
    BleScanner& scanner;
	BleConnection& connection;

    void CmdScan(const std::vector<std::string>& args);
    void CmdList(const std::vector<std::string>& args);
    void CmdHelp();
	void CmdConnect(const std::vector<std::string>& args);
    void CmdDisconnect();
	void CmdRead(const std::vector<std::string>& args);
	void CmdWrite(const std::vector<std::string>& args);
	void CmdStatus();

    public:
        CommandParser(BleScanner& scanner, BleConnection& connection);

        bool HandleCommand(const std::string& command);
};
