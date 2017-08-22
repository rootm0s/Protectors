#include "stdafx.h"

class ArgsParser {
public:
	bool parseMainArgs(int argc, char **argv);

	int getArgCount();
	int getArgCount(std::string name);
	std::string getArgStringOccurrence(std::string name, int idx);
	int getArgIntOccurrence(std::string name, int idx, int base=10);

	std::string getArgString(std::string name);
	int getArgInt(std::string name, int base=10);

	void print() const;
private:
	void addArg(std::string name, std::string value);
	std::vector<std::string> *getArgs(std::string name);
	std::map<std::string, std::vector<std::string>> m_arguments;
};