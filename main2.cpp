// A Simple Text Editor
// Author philip0000000
// MIT license

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void ShowAll(std::vector<std::string>& lines) {
    std::cout << "File content:" << std::endl;
    for (const auto& line : lines) {
        std::cout << line << std::endl;
    }
}

void Show(std::vector<std::string>& lines) {
    std::string lineNumber;
    std::cout << "Enter the line number to show (1-based index): ";
    std::getline(std::cin, lineNumber);
    int num = std::stoi(lineNumber);
    std::cout << lines[num - 1] << std::endl;
}

void Edit(std::vector<std::string>& lines) {
    int lineNumber;
    std::cout << "Enter the line number to edit (1-based index): ";
    std::cin >> lineNumber;

    if (lineNumber > 0 && lineNumber <= lines.size()) {
        std::cout << "Current line: " << lines[lineNumber - 1] << std::endl;
        std::cout << "Enter the new line content: ";
        std::cin.ignore();
        std::getline(std::cin, lines[lineNumber - 1]);
    }
    else
        std::cerr << "Invalid line number" << std::endl;
}

void OpenFile(std::string& filename, std::vector<std::string>& lines) {
    std::cout << "Enter the file name: ";
    std::cin >> filename;

    // Clear the vector to reuse it
    lines.clear();

    // Read file
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
}

void SaveFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream outFile(filename);
    for (const auto& line : lines) {
        outFile << line << std::endl;
    }
    outFile.close();

    std::cout << "File saved" << std::endl;
}

int main() {
    std::string filename;
    std::vector<std::string> lines;

    OpenFile(filename, lines);

    while (true) {
        // Read a command from the user
        std::cout << "> "; // Print a prompt to the screen
        std::string command;
        std::getline(std::cin, command);

        if (command == "help")
            std::cout << "Available commands: help, show all, show, edit, open, save, exit" << std::endl;
        else if (command == "show all")
            ShowAll(lines);
        else if (command == "show")
            Show(lines);
        else if (command == "edit")
            Edit(lines);
        else if (command == "open")
            OpenFile(filename, lines);
        else if (command == "save")
            SaveFile(filename, lines);
        else if (command == "exit")
            break;
        else
            std::cout << "Unknown command. Type 'help' for a list of commands." << std::endl;
    }

    return 0;
}
