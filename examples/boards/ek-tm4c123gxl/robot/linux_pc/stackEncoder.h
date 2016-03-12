#ifndef STACK_ENCODER_H
#define STACK_ENCODER_H

#include <iostream>
#include <string>
#include <vector>

class StackEncoder
{
public:
	struct SymTableEntry
	{
		uint32_t address;
		uint32_t size;
		std::string name;
	};

	StackEncoder(bool _isMaster, uint8_t _partition);
	void decodeStacktrace(std::string& line);
	std::string getFuncName(uint32_t address);
private:
	bool isAddrInFunc(const SymTableEntry& func, uint32_t address);
	bool buildSymTableFromAxf();
	void sortSymTable();
	bool isMaster;
	uint8_t partition;
	std::vector<SymTableEntry> symbolTable;
	bool isSymTableBuilt = false;
};


#endif // STACK_ENCODER_H
