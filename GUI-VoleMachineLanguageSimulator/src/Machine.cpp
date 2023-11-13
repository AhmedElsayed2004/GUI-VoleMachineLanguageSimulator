#include "Machine.h"
#include <iostream>
#include <string>
#include <regex>


void Machine::Run()
{
    int choice{};

    ResetMachine();
    while (!LoadNewProgram())
        std::cout << "Please load a new program" << std::endl;

    cpu.programCounter = m_startAddress;

    while (choice != 5)
    {
        std::cout << "Enter 1 to load a new program" << std::endl;
        std::cout << "Enter 2 to execute next step" << std::endl;
        std::cout << "Enter 3 to execute entire program" << std::endl;
        std::cout << "Enter 4 to Display information about the register, PC, IR and memory" << std::endl;
        std::cout << "Enter 5 to exit program" << std::endl;

        std::cin >> choice;

        while (choice < 1 || choice > 5)
        {
            std::cout << "Please enter a valid choice" << std::endl;
            std::cin >> choice;
        }

        switch (choice)
        {
        case 1:
            ResetMachine();
            LoadNewProgram();

            cpu.programCounter = m_startAddress;

            break;
        case 2:
            cpu.FetchInstruction(m_memory);

            if (cpu.IsValidInstruction() && !cpu.isHalt)
            {
                cpu.ExecuteInstruction(m_memory);
            }
            else if (cpu.isHalt)
            {
                std::cout << "Program has halted" << std::endl;
            }
            else if (!cpu.IsValidInstruction())
            {
                std::cout << "Invalid Instruction at address " << cpu.programCounter << std::endl;
            }
            break;
        case 3:
            while (true)
            {
                cpu.FetchInstruction(m_memory);
                if (cpu.IsValidInstruction() && !cpu.isHalt)
                {
                    cpu.ExecuteInstruction(m_memory);
                }

                else if (cpu.isHalt)
                {
                    std::cout << "Program has halted" << std::endl;
                    break;
                }

                else if (!cpu.IsValidInstruction())
                {
                    std::cout << "Invalid Instruction at address " << cpu.programCounter << std::endl;
                    break;
                }
            }
            break;
        case 4:
            DisplayInfo();
            break;
        case 5:
            break;
        }
    }
}

bool Machine::LoadNewProgram()
{
	std::string fileName;

	std::cout << "Enter a file"<< std::endl;
	std::cin >> fileName;

    m_inputFile.open(fileName);

    // Check if the file has been opened successfully
    if (!m_inputFile.is_open()) 
    {
        std::cout << "Error opening the file" << std::endl;
        return false;
    }
    else
    {
        if (LoadAllInstructions())
        {
            std::cout << "Enter the start address" << std::endl;

            while (!SetStartAddress())
                std::cout << "Instructions don't fit inside of the machine, Please enter another start address" << std::endl;
        }
        else
        {
            return false;
        }
    }

    LoadInstructionsIntoMemory();

    return true;
}

bool Machine::LoadAllInstructions()
{
    std::string line;

    Byte firstByte;
    Byte secondByte;
    
    while (std::getline(m_inputFile, line))
    {
        // Sets up filters to check if any invalid instructions are put
        std::string stringFilter = "([1-6]|[B-C])[0-9A-F][0-9A-F][0-9A-F]";
        std::regex regexFilter(stringFilter);

        // Checks if the given line matches the filter
        if (std::regex_match(line, regexFilter))
        {
            firstByte.nibble[0] = line[0];
            firstByte.nibble[1] = line[1];

            secondByte.nibble[0] = line[2];
            secondByte.nibble[1] = line[3];

            m_loadedInstructions.push_back(firstByte);
            m_loadedInstructions.push_back(secondByte);

        }
        else
        {
            m_loadedInstructions.clear();
            std::cout << "Invalid instructions" << std::endl;
            return false;
        }

        if (m_loadedInstructions.size() > 256)
        {
            m_loadedInstructions.clear();
            std::cout << "Too many instructions" << std::endl;
            return false;
        }
    }
    return true;
}

void Machine::LoadInstructionsIntoMemory()
{
    for (int i = m_startAddress;i < m_startAddress + m_loadedInstructions.size();++i)
    {
        m_memory[i] = m_loadedInstructions[i - m_startAddress];
    }
}

bool Machine::SetStartAddress()
{
    int startAddress;
    std::cin >> startAddress;

    // if the loaded instructions won't fit inside of the machine
    if (startAddress + m_loadedInstructions.size() > 256)
        return false;

    m_startAddress = startAddress;
    return true;
}

void Machine::DisplayInfo()
{
    std::cout << "        CPU        " << std::endl;
    for (int i = 0; i < 18; i++)
    {
        if (i > 0 && i < 11)
        {
            if (i == 8)
            {
                std::cout << "|  " << 'R' << i - 1 << "  " << cpu.cpuRegister[i - 1].nibble[0]
                    << cpu.cpuRegister[i - 1].nibble[1] << "  PC  " << cpu.programCounter;

                // Calculate the number of digits
                int NumOfDigits;
                if (cpu.programCounter == 0)
                {
                    NumOfDigits = 1;
                }
                else
                {
                    NumOfDigits = log10(cpu.programCounter) + 1;
                }

                for (int i = 0; i < 5 - NumOfDigits; i++) {
                    std::cout << " ";
                }

                std::cout << '|' << std::endl;

                continue;
            }
            else if (i == 9)
            {
                std::cout << "|  " << 'R' << i - 1 << "  " << cpu.cpuRegister[i - 1].nibble[0]
                    << cpu.cpuRegister[i - 1].nibble[1] << "  IR  " << cpu.IR[0] << cpu.IR[1] << cpu.IR[2] << cpu.IR[3] << " |" << std::endl;

                continue;
            }
            std::cout << "|  " << 'R' << i - 1 << "  " << cpu.cpuRegister[i - 1].nibble[0] 
                << cpu.cpuRegister[i - 1].nibble[1] << "           |" << std::endl;
        }
        else if (i > 10 && i < 17)
        {
            std::cout << "|  " << 'R' << char((i - 1) - 10 + 'A') << "  " << cpu.cpuRegister[i - 1].nibble[0] 
                << cpu.cpuRegister[i - 1].nibble[1] << "           |" << std::endl;
        }
        else
        {
            std::cout << "---------------------" << std::endl;
        }
    }
	
    for (int i = 0; i < 35; i++)
    {
        std::cout << " ";
    }
    std::cout << "Main Memory";
    for (int i = 0; i < 35; i++)
    {
        std::cout << " ";
    }
    
    std::cout << std::endl;

    for (int i = 0; i < 90; i++)
    {
        std::cout << "-";
    }

    std::cout << std::endl;
    
    std::cout << "|      ";
    for (int i = 0; i < 16; i++)
    {
        if (i < 10)
            std::cout << i << "    ";
        else
            std::cout << char('A' + (i - 10)) << "    ";
    }
    
    std::cout << "  |" << std::endl;

    for (int i = 0; i < 16; i++)
    {
        if (i < 10)
            std::cout << "|  " << i << "   ";
        else
            std::cout << "|  " << char('A' + (i - 10)) << "   ";

        for (int j = i * 16; j < (i + 1) * 16; j++)
        {
            std::cout << m_memory[j].nibble[0] << m_memory[j].nibble[1] << "   ";
        }
        std::cout << "  |" << std::endl;
    }

    for (int i = 0; i < 90; i++)
    {
        std::cout << "-";
    }
    
    std::cout << std::endl;
}

void Machine::ResetMachine()
{
    for (int i = 0; i < 256; ++i)
        m_memory[i] = { {'0','0'} };

    cpu.ResetCPU();


}
