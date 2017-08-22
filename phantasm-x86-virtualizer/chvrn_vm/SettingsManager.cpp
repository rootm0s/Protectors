#include "stdafx.h"
#include "SettingsManager.h"

// Input is of the form "variable=value"
bool SettingsManager::setVariableValuePair(const char *szStr) {
		const char* szValue = strchr(szStr, '=');
		if (!szValue) {
			printf("Missing value for command %s\n", szStr);
			return false;
		}
		
		char *szVariable = strncpy(new char[szValue - szStr + 1], szStr, szValue - szStr);
		szVariable[szValue - szStr] = '\0';
		szValue++;	
		
		if (!setVariable(szVariable, szValue)) {
			printf("Unknown variable \"%s\" - ignoring\n", szVariable); 
		}
		
		delete[] szVariable;
		return true;
}

bool SettingsManager::parseFile(const char *szFile) {
	bool success = true;
	return success;
}

bool SettingsManager::parseMainArgs(int argc, const char **argv) {
	bool success = true;
	for (int i = 1; i < argc; i++) {
		if (!setVariableValuePair(argv[i]))
			success = false;
	}
	return success;
}

SettingsManager::~SettingsManager() {
	for (unsigned int i = 0; i < m_vVariables.size(); i++)
		delete m_vVariables[i];
}

const SettingsVariable* SettingsManager::getVariable(const char *szName) {
	for (unsigned int i = 0; i < m_vVariables.size(); i++) {
		if (strcmp(m_vVariables[i]->szName, szName) == 0) {
				return m_vVariables[i];
			}
	}
	return NULL;
}

const char* SettingsManager::getVariableAsString(const char *szName) {
	const SettingsVariable *var = getVariable(szName);
	return var != NULL ? var->szValue : NULL;
}

int SettingsManager::getVariableAsInteger(const char *szName) {
	const SettingsVariable *var = getVariable(szName);
	return var != NULL ? var->iValue : 0;
}

bool SettingsManager::setVariable(const char *szName, const char *szValue) {
	std::cout << "Setting variable " << szName << " to " << szValue << std::endl;
	
	if (SettingsVariable *variable = (SettingsVariable*)getVariable(szName)) {
		
		if (variable->kType == VAR_STRING) {
			delete[] variable->szValue;
			variable->szValue = strcpy(new char[strlen(szValue) + 1], szValue);
		}
		if (variable->kType == VAR_INTEGER) {
			std::stringstream ss;
			ss << std::hex << szValue;
			ss >> variable->iValue;
		}
		return true;
	}
	return false;
}

SettingsVariable* SettingsManager::addVariable(SettingsVariable *variable) {
	m_vVariables.push_back(variable);
	return variable;
}

SettingsVariable* SettingsManager::addVariable(const char *szName, const char *szDescription, const char *szValue) {
	
	SettingsVariable *variable = new SettingsVariable;
	variable->szName = szName;
	variable->szDescription = szDescription;
	variable->kType = VAR_STRING;
	variable->szValue = strcpy(new char[strlen(szValue) + 1], szValue);
	
	return addVariable(variable);
}

SettingsVariable* SettingsManager::addVariable(const char *szName, const char *szDescription, int iValue) {
	
	SettingsVariable *variable = new SettingsVariable;
	variable->szName = szName;
	variable->szDescription = szDescription;
	variable->kType = VAR_INTEGER;
	variable->szValue = itoa(iValue, new char[11], 10);
	variable->iValue = iValue;
	
	return addVariable(variable);
}

void SettingsManager::printHelp() {
	if (m_vVariables.size() == 0) {
		printf("No settings available\n");
		return;
	}

	std::cout << "Available settings and their values: " << std::endl;
	
	for (unsigned int i = 0; i < m_vVariables.size(); i++)
	{
		std::cout << m_vVariables[i]->szName << "=";
		
		if (m_vVariables[i]->kType == VAR_STRING) {
			std::cout << "[String]";
		} else {
			std::cout << "[Integer]" ;
		}
		
		std::cout << " (" << m_vVariables[i]->szValue << ")" << " - " << m_vVariables[i]->szDescription << std::endl;
	}
}