#include <iostream>
#include <string>
#include "BleScanner.h"
#include "CommandParser.h"

int main(void) {
    BleScanner scanner;
    CommandParser parser(scanner);

    std::cout << "BluetoothConfigCLI\n"
        << "Commands: scan, stop scan, list, exit\n\n";

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
