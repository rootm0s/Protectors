#include "stdafx.h"
#include "ArgsParser.h"

using namespace std;

bool ArgsParser::parseMainArgs(int argc, char **argv) {
	for (int i = 0; i < argc; i++) {
		char *valuePtr = strchr(argv[i], '=');
		if (!valuePtr) {
			addArg(argv[i], "");
			continue;
		}
		valuePtr++;
		int nameLen = valuePtr-argv[i]-1;

		string name(argv[i], nameLen);
		string value(valuePtr);

		addArg(name, value);
	}

	return true;
}

void ArgsParser::addArg(string name, string value) {
	if (m_arguments.find(name) == m_arguments.end()) {
		m_arguments[name] = vector<string>();
	}
	m_arguments[name].push_back(value);
}

vector<string> *ArgsParser::getArgs(string name) {
	if (m_arguments.find(name) == m_arguments.end()) {
		return nullptr;
	}
	return &m_arguments[name];
}

int ArgsParser::getArgCount() {
	return m_arguments.size();
}

int ArgsParser::getArgCount(string name) {
	if (!getArgs(name)) {
		return 0; 
	} else {
		return getArgs(name)->size();
	}
}

string ArgsParser::getArgStringOccurrence(string name, int idx) {
	if (!getArgs(name)) {
		return "";
	} else {
		return (*getArgs(name))[idx];
	}
}

int ArgsParser::getArgIntOccurrence(string name, int idx, int base) {
	if (!getArgs(name)) {
		return 0;
	} else {
		try {
		return stoi((*getArgs(name))[idx], 0, base);
		} catch (exception e) {
			logger.write(LOG_WARN, "Unexpected format for integer argument\n");
			return 0;
		}
	}
}

string ArgsParser::getArgString(string name) {
	return getArgStringOccurrence(name, 0);
}

int ArgsParser::getArgInt(string name, int base) {
	return getArgIntOccurrence(name, 0, base);
}

void ArgsParser::print() const {
	for (auto& it0 : m_arguments) {
		logger.write(LOG_MSG, "%s\n", it0.first.c_str());
		for (auto& it1 : it0.second) {
			if (it1.length()) {
				logger.write(LOG_MSG, "\t%s\n", it1.c_str());
			}
		}
	}
}