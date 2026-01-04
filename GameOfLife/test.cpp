#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

void fileLinesToHex(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::cout << line.size() << std::endl;
        for (unsigned char c : line)
        {
            std::cout << std::hex
                      << std::setw(2)
                      << std::setfill('0')
                      << static_cast<int>(c)
                      << " ";
        }
        std::cout << std::endl;
    }

    std::cout << std::dec; // restore default formatting
}


int main(){
    fileLinesToHex("GameOfLife/gamelabel.txt");
    return 0;
}