#include "Machine.h"
#include <iostream>
#include <string>
#include <regex>


Machine::Machine()
{
    ResetMachine();
}

void Machine::Run(int choice,char* instructions, char* startAddress)
{
    m_event = Event::NONE;

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
        if (LoadNewProgram(instructions) && SetStartAddress(startAddress))
        {
            LoadInstructionsIntoMemory();
            cpu.programCounter = m_startAddress;
            cpu.FetchInstruction(m_memory);
        }

        break;
    case 2:
        if (cpu.IsValidInstruction() && !cpu.isHalt)
        {
            cpu.ExecuteInstruction(m_memory, screen);
            cpu.FetchInstruction(m_memory);
        }
        else if (cpu.isHalt)
        {
            m_event = Event::PROGRAM_HALTED;
        }
        else if (!cpu.IsValidInstruction())
        {
            m_event = Event::INVALID_INSTRUCTION;
        }

        break;
    case 3:
        while (true)
        {
            cpu.FetchInstruction(m_memory);
            if (cpu.IsValidInstruction() && !cpu.isHalt)
            {
                cpu.ExecuteInstruction(m_memory, screen);
            }

            else if (cpu.isHalt)
            {
                m_event = Event::PROGRAM_HALTED;
                break;
            }

            else if (!cpu.IsValidInstruction())
            {
                m_event = Event::INVALID_INSTRUCTION;
                break;
            }
        }
        break;
    case 4:
        ResetMachine();
        break;
    case 5:
        break;
    }
}

bool Machine::LoadNewProgram(char* instructions)
{
    m_loadedInstructions.clear();
    for (int i = 0;i < 1000;)
    {
        if (instructions[i] == '\n')
        {
            ++i;
            continue;
        }
        if (instructions[i] == 0)
        {
            break;
        }
        if (!ValidHex(instructions[i]))
        {
            m_loadedInstructions.clear();
            m_event = Event::INVALID_INSTRUCTION;
            return true;
        }
        if (!ValidHex(instructions[i + 1]))
        {
            m_loadedInstructions.clear();
            m_event = Event::INVALID_INSTRUCTION;
            return true;
        }
        m_loadedInstructions.push_back({ instructions[i] ,instructions[i + 1] });
        i += 2;
    }
   
    return true;
}

void Machine::LoadInstructionsIntoMemory()
{
    for (int i = m_startAddress;i < m_startAddress + m_loadedInstructions.size();++i)
    {
        m_memory[i] = m_loadedInstructions[i - m_startAddress];
    }
    m_loadedInstructions.clear();
}

Event Machine::GetEvent()
{
    return m_event;
}

void Machine::ResetMachine()
{
    m_loadedInstructions.clear();
    for (int i = 0; i < 256; ++i)
        m_memory[i] = { {'0','0'} };
    
    m_startAddress = 0;
    cpu.ResetCPU();
}

bool Machine::SetStartAddress(char *startAddress)
{
    if (!ValidHex(startAddress[0]) || !ValidHex(startAddress[1]))
    {
        m_event = Event::INVALID_START_ADDRESS;
        return false;
    }
    std::string address = "";
    address.push_back(startAddress[0]);
    address.push_back(startAddress[1]);
    m_startAddress = stoi(address, 0, 16);
    return true;
}

bool Machine::ValidHex(char digit)
{
    return (digit >= '0' && digit <= '9') || (digit >= 'A' && digit <= 'F');
}