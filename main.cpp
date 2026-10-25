#include <iostream>
#include <string>
#include "BleScanner.h"
#include "CommandParser.h"

int main(void) {
    BleScanner scanner;
    BleConnection connection(scanner);
    CommandParser parser(scanner, connection);

    std::cout << "BluetoothConfigCLI\n"
        << "Commands: scan, list, connect, disconnect, read, write, status, help, exit\n\n";

    std::string command;

    while (true) {
        std::cout << "> ";

        if (!std::getline(std::cin, command))
            break;

        if (!parser.HandleCommand(command))
            break;
    }

    return 0;
}
