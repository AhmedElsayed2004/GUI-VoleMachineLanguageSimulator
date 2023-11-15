#pragma once
#include "Event.h"
#include "Byte.h"
#include <vector>
#include <fstream>
#include "CPU.h"

class Machine
{
private:
	std::ifstream m_inputFile;

	int m_startAddress{};
	std::vector<Byte> m_loadedInstructions;
	Event m_event = Event::NONE;

	// Loads all the instructions in a given file returns false if there was an error
	bool LoadAllInstructions();

	//void DisplayInfo();

	void ResetMachine();

	// Returns false if it couldn't load a new program
	bool LoadNewProgram(char *instructions);

	void LoadInstructionsIntoMemory();

	bool SetStartAddress(char startAddress[]);

	bool ValidHex(char digit);

public:
	CPU cpu;
	Byte m_memory[256];
	char screen = '0';

	Machine();

	void Run(int choice,char *instructions, char *startAddress);
	Event GetEvent();
};

