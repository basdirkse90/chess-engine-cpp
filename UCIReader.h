#pragma once
#include <string>

class UCIReader {
private:
	static const std::string ENGINENAME; 
	static const std::string ENGINEAUTHOR;

	static void myPerft(bool runall = false, bool deep = false);

public:
	static void uciCommunication();
};


