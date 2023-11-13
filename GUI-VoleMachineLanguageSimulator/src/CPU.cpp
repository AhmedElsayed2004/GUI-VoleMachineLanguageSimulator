#include "CPU.h"


CPU::CPU()
{

	//Save all valid instructions
	m_validInstructions[0] = "1RXY";
	m_validInstructions[1] = "2RXY";
	m_validInstructions[2] = "3RXY";
	m_validInstructions[3] = "40RS";
	m_validInstructions[4] = "5RST";
	m_validInstructions[5] = "6RST";
	m_validInstructions[6] = "BRXY";
	m_validInstructions[7] = "C000";
}

int CPU::ToDecimal(std::string HEX)
{
	int decimal = 0, power = 1;

	for (int i = 0;i < HEX.size();++i)
	{
		decimal *= power;
		if (HEX[i] >= '0' && HEX[i] <= '9') decimal += HEX[i] - '0';
		else decimal += HEX[i] - 'A' + 10;
		power *= 16;
	}
	return decimal;
}
std::string CPU::ToHEX(int number)
{
	std::string result = "";
	while (number)
	{
		int m = number % 16;
		if (m >= 10) result.push_back('A' + m - 10);
		else result.push_back(m + '0');
		number /= 16;
	}
	std::reverse(result.begin(), result.end());
	return result;
}

Byte CPU::add(Byte b1, Byte b2)
{
	std::string HEXNum1 = "";
	std::string HEXNum2 = "";

	// Save numbers in string to be easy to deal with it
	HEXNum1.push_back(b1.nibble[0]);
	HEXNum1.push_back(b1.nibble[1]);
	HEXNum2.push_back(b2.nibble[0]);
	HEXNum2.push_back(b2.nibble[1]);

	// Convert numbers to decimal
	int num1 = ToDecimal(HEXNum1);
	int num2 = ToDecimal(HEXNum2);


	int adding = num1 + num2;

	// to erase carry
	adding &= ((1 << 8) - 1);

	// Convert the adding to Hexadecimal number to convert it to Byte
	std::string HEXAdding = ToHEX(adding);

	Byte result;

	// Save the final adding to Byte
	if (HEXAdding.size() == 1)
	{
		result.nibble[0] = '0';
		result.nibble[1] = HEXAdding[0];
	}
	else
	{
		result.nibble[0] = HEXAdding[0];
		result.nibble[1] = HEXAdding[1];
	}

	return result;

}
void CPU::FetchInstruction(Byte mainMemory[])
{
	// Take instruction and put it in IR
	IR[0] = (mainMemory + programCounter)->nibble[0];
	IR[1] = (mainMemory + programCounter)->nibble[1];
	IR[2] = (mainMemory + programCounter + 1)->nibble[0];
	IR[3] = (mainMemory + programCounter + 1)->nibble[1];
}


bool CPU::IsValidInstruction()
{
	for (auto& instruction : m_validInstructions)
	{
		if (IR[0] == instruction[0])
		{
			for (int i = 0;i < 4;++i)
			{
				// Check if there is an odd entiry in instruction
				if (instruction[i] == 'R' || instruction[i] == 'S' || instruction[i] == 'T' || instruction[i] == 'X' || instruction[i] == 'Y')
				{
					if (!(IR[i] >= '0' && IR[i] <= '9') && !(IR[i] >= 'A' && IR[i] <= 'F')) return false;
				}
				else
				{
					if (IR[i] != instruction[i]) return false;
				}
			}
			return true;
		}
	}
	return false;

}

void CPU::ExecuteInstruction(Byte mainMemory[])
{
	if (IR[0] == '1')
	{

		std::string numberOfRegister = "";
		std::string address = "";

		// Take a number of register from instruction
		numberOfRegister.push_back(IR[1]);

		// Take a address from instruction
		address.push_back(IR[2]);
		address.push_back(IR[3]);

		// Load in regiter a new value
		cpuRegister[ToDecimal(numberOfRegister)] = *(mainMemory + ToDecimal(address));
		
		programCounter += 2;
	}
	if (IR[0] == '2')
	{
		std::string numberOfRegister = "";
		Byte bitPattern;

		// Take a number of register from instruction
		numberOfRegister.push_back(IR[1]);
		
		// Take a value i want to load from instruction
		bitPattern.nibble[0] = IR[2];
		bitPattern.nibble[1] = IR[3];

		// Load in regiter a new value
		cpuRegister[ToDecimal(numberOfRegister)] = bitPattern;

		programCounter += 2;
	}
	if (IR[0] == '3')
	{

		// implement if input is 3R00 (show on the screen)
		std::string numberOfRegister = "";
		std::string address = "";


		// Take a number of register from instruction
		numberOfRegister.push_back(IR[1]);

		// Take a address from instruction
		address.push_back(IR[2]);
		address.push_back(IR[3]);

		// Load in memory a new value
		*(mainMemory + ToDecimal(address)) = cpuRegister[ToDecimal(numberOfRegister)];

		programCounter += 2;
	}
	if (IR[0] == '4')
	{
		std::string numberOfRegister1 = "";
		std::string numberOfRegister2 = "";

		// Take a number of register from instruction
		numberOfRegister1.push_back(IR[2]);
		numberOfRegister2.push_back(IR[3]);

		// Load in regiter a new value
		cpuRegister[ToDecimal(numberOfRegister2)] = cpuRegister[ToDecimal(numberOfRegister1)];

		programCounter += 2;
	}

	if (IR[0] == '5')
	{
		std::string numberOfRegister1 = "";
		std::string numberOfRegister2 = "";
		std::string numberOfRegister3 = "";

		// Take a number of register from instruction
		numberOfRegister1.push_back(IR[1]);
		numberOfRegister2.push_back(IR[2]);
		numberOfRegister3.push_back(IR[3]);

		// Load in regiter a new value
		cpuRegister[ToDecimal(numberOfRegister1)] = add(cpuRegister[ToDecimal(numberOfRegister2)], cpuRegister[ToDecimal(numberOfRegister3)]);

		programCounter += 2;
	}

	if (IR[0] == 'B')
	{
		std::string numberOfRegister1 = "";
		std::string address = "";

		// Take a number of register from instruction
		numberOfRegister1.push_back(IR[1]);

		// Take a address from instruction
		address.push_back(IR[2]);
		address.push_back(IR[3]);

		// Compare between register IR[1] and register 0
		if (cpuRegister[ToDecimal(numberOfRegister1)].nibble[0] == cpuRegister[0].nibble[0] && 
			cpuRegister[ToDecimal(numberOfRegister1)].nibble[1] == cpuRegister[0].nibble[1])
		{
			programCounter = ToDecimal(address);
		}
		else
		{
			programCounter += 2;
		}
	}
	if (IR[0] == 'C')
	{
		isHalt = true;

		programCounter += 2;
	}


}

void CPU::ResetCPU()
{
	for (int i = 0; i < 16; ++i)
		cpuRegister[i] = { {'0','0'} };
	IR = "0000";
}
