#pragma once
#include "Byte.h"
#include <vector>
#include <fstream>
#include "CPU.h"

class Machine
{
private:
	std::ifstream m_inputFile;

	Byte m_memory[256];

	int m_startAddress;
	std::vector<Byte> m_loadedInstructions;

	CPU cpu;

	// Loads all the instructions in a given file returns false if there was an error
	bool LoadAllInstructions();
	
	// Sets the start address of the program returns false if the start address is invalid
	bool SetStartAddress();

	void DisplayInfo();

	void ResetMachine();

	// Returns false if it couldn't load a new program
	bool LoadNewProgram();


	void LoadInstructionsIntoMemory();

public:
	void Run();

};

