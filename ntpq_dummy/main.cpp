#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) 
{
    std::ifstream file("ntpq_output.txt");

    if(!file.is_open())
    {
        std::cerr << "ntpq: read: Connection refused";
        return -1;
    }

    char c = file.get();
    while(file.good()) 
    {
        std::cout << c;
        c = file.get();
    }

    file.close();

    return 0;
}
