#include "stdafx.h"

enum VariableType {
	VAR_STRING,
	VAR_INTEGER
};

struct SettingsVariable {
	~SettingsVariable() { delete[] szValue; }
	const char *szName;
	const char *szDescription;
	VariableType kType;
	const char *szValue;
	int iValue;
};

class SettingsManager {
public:
	~SettingsManager();
	bool parseMainArgs(int argc, const char **argv);
	bool parseFile(const char *szFile);
	SettingsVariable* addVariable(const char *szName, const char *szDescription, const char *szValue);
	SettingsVariable* addVariable(const char *szName, const char *szDescription, int iValue);
	const SettingsVariable* getVariable(const char *szName);
	const char *getVariableAsString(const char *szName);
	int getVariableAsInteger(const char *szName);
	bool setVariable(const char *szName, const char *szValue);
	void printHelp();
private:
	SettingsVariable* addVariable(SettingsVariable *variable);
	bool setVariableValuePair(const char *szStr);
	std::vector<SettingsVariable*> m_vVariables;
};