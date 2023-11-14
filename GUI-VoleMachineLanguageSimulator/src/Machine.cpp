#include "Machine.h"
#include <iostream>
#include <string>
#include <regex>


void Machine::Run(int choice,char* instructions)
{

    cpu.programCounter = m_startAddress;

    //std::cout << "Enter 1 to load a new program" << std::endl;
    //std::cout << "Enter 2 to execute next step" << std::endl;
    //std::cout << "Enter 3 to execute entire program" << std::endl;
    //std::cout << "Enter 4 to Display information about the register, PC, IR and memory" << std::endl;
    //std::cout << "Enter 5 to exit program" << std::endl;

    //std::cin >> choice;

    //while (choice < 1 || choice > 5)
    //{
    //    std::cout << "Please enter a valid choice" << std::endl;
    //    std::cin >> choice;
    //}

    switch (choice)
    {
    case 1:
        ResetMachine();
        LoadNewProgram( instructions);

        cpu.programCounter = m_startAddress;

        break;
    case 2:
        /*cpu.FetchInstruction(m_memory);

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
        }*/
        break;
    case 3:
        /*while (true)
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
        }*/
        break;
    case 4:
        //DisplayInfo();
        break;
    case 5:
        break;
    }
}

bool Machine::LoadNewProgram(char* instructions)
{
    for (int i = 0;i < 1000;i+=2)
    {
        if (instructions[i] == 0|| instructions[i+1] == 0) break;
        m_loadedInstructions.push_back({ instructions[i],instructions[i + 1] });
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

void Machine::ResetMachine()
{
    m_loadedInstructions.clear();
    for (int i = 0; i < 256; ++i)
        m_memory[i] = { {'0','0'} };

    cpu.ResetCPU();


}
