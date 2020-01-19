#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "WorkflowExceptions.h"
using namespace std;

class Worker {
protected:
	const vector<string> params;
public:
	Worker(const vector<string>& params) : params(params) {}
	virtual void work(string& textStorage) = 0;
};

class Dumper : public Worker {
public:
	Dumper(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		ofstream file(params[0]);
		if (!file.is_open()) throw FileOpeningException(params[0]);
		file << textStorage;
	}
};

class FileReader : public Worker {
	static size_t instanceCount;
public:
	FileReader(const vector<string>& params) : Worker(params) {
		if (++instanceCount > 1) throw RWBlocksNumberException();
	}
	virtual void work(string& textStorage) {
		ifstream file(params[0]);
		if (!file.is_open()) throw FileOpeningException(params[0]);
		stringstream buffer;
		file >> buffer.rdbuf();
		textStorage = buffer.str();
	}
};
size_t FileReader::instanceCount = 0;

class FileWriter : public Dumper {
	static size_t instanceCount;
public:
	FileWriter(const vector<string>& params) : Dumper(params) {
		if (++instanceCount > 1) throw RWBlocksNumberException();
	}
};
size_t FileWriter::instanceCount = 0;

class GrepWorker : public Worker {
public:
	GrepWorker(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		string buffer;
		istringstream oldStorageStream(textStorage);
		ostringstream newStorageStream;
		bool firstLine = true;
		while (!oldStorageStream.eof()) {
			getline(oldStorageStream, buffer);
			if (buffer.find(params[0]) != string::npos) {
				if (firstLine) {
					newStorageStream << buffer;
					firstLine = false;
				}
				else {
					newStorageStream << endl << buffer;
				}
			}
		}
		textStorage = newStorageStream.str();
	}
};

class Sorter : public Worker {
public:
	Sorter(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		string buffer;
		istringstream oldStorageStream(textStorage);
		ostringstream newStorageStream;
		vector<string> stringStorage;
		oldStorageStream.peek();
		while(!oldStorageStream.eof()) {
			getline(oldStorageStream, buffer);
			stringStorage.push_back(buffer);
		}
		sort(stringStorage.begin(), stringStorage.end());
		for_each(stringStorage.begin(), --stringStorage.end(), [](string& notLast) {notLast += "\n"; });
		for (string& str : stringStorage) newStorageStream << str;
		textStorage = newStorageStream.str();
	}
};

class Replacer : public Worker {
public:
	Replacer(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		for (size_t pos = textStorage.find(params[0]); pos != string::npos; pos = textStorage.find(params[0])) {
			textStorage.replace(pos, params[0].length(), params[1]);
		}
	}
};

class Executor {
	vector<unsigned int> commands;
	string textStorage;
public:
	void pushCommand(unsigned int cNumber) {
		commands.push_back(cNumber);
	}
	void validateCommandList(const map<unsigned int, Worker*>& workerStorage) {
		if (commands.size() < 2) throw RWBlocksNumberException();
		if (typeid(*workerStorage.at(commands[0])) != typeid(FileReader) ||
			typeid(*workerStorage.at(commands[commands.size() - 1 ])) != typeid(FileWriter)) throw RWBlocksNumberException();
	}
	void run(map<unsigned int, Worker*>& workerStorage) {
		for (unsigned int commandNumber : commands) {
			try {
				workerStorage.at(commandNumber)->work(textStorage);
			}
			catch (FileOpeningException& errInfo) {
				throw CommandExecutionException(errInfo.what(), commandNumber);
			}
			catch (exception & errInfo) {
				throw CommandExecutionException(string("Unexpected:\n") + string(errInfo.what()), commandNumber);
			}
			catch (...) {
				throw CommandExecutionException("Unknown error", commandNumber);
			}
		}
	}
};

class Parser {
private:
	ifstream file;
	string getRemaining(istringstream& src) {
		return src.eof() ? string() : src.str().substr(src.tellg());
	}
	vector<string> parseWorkerArgs(const string& src, int argCount = 0) {
		vector<string> result;
		if (src.find_first_of('"') != string::npos) {
			size_t q1Pos = src.find_first_of('"');
			size_t q2Pos = src.find_first_of('"', q1Pos + 1);
			if ((q2Pos == string::npos) ||
			   (q1Pos != 0 && src[q1Pos - 1] != ' ') ||
			   (q2Pos != src.length() - 1 && src[q2Pos + 1] != ' ')) throw FormatException();

			result = parseWorkerArgs(src.substr(0, q1Pos));
			result.push_back(src.substr(q1Pos + 1, q2Pos - q1Pos - 1));
			vector<string> rightSide = parseWorkerArgs(src.substr(q2Pos + 1, string::npos));
			result.insert(result.end(), rightSide.begin(), rightSide.end());
		}
		else {
			istringstream helper(src);
			string buffer;
			for (;;) {
				helper >> buffer;
				if (buffer.empty()) break;
				result.push_back(buffer);
				if (helper.eof()) break;
			}
		}
		if (argCount != 0 && argCount != result.size()) throw ArgumentCountException();
		return result;
	}
	void instantiateWorker(map<unsigned int, Worker*>& workerStorage, const string& workerConfig) {
		istringstream stream(workerConfig);
		stream.exceptions(istringstream::failbit);
		unsigned int workerNumber;
		stream >> workerNumber;
		if (workerStorage.count(workerNumber) != 0) throw IdentifierNumberException();
		char c;
		stream >> c;
		if (c != '=') throw FormatException();
		string buffer;
		stream >> buffer;
		if (buffer.compare("readfile") == 0) {
			vector<string> params = parseWorkerArgs(getRemaining(stream), 1);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new FileReader(params)));
		}
		else if (buffer.compare("writefile") == 0) {
			vector<string> params = parseWorkerArgs(getRemaining(stream), 1);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new FileWriter(params)));
		}
		else if (buffer.compare("grep") == 0) {
			vector<string> params = parseWorkerArgs(getRemaining(stream), 1);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new GrepWorker(params)));
		}
		else if (buffer.compare("sort") == 0) {
			vector<string> params = parseWorkerArgs(getRemaining(stream), 0);
			if (!params.empty()) throw ArgumentCountException();
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new Sorter(params)));
		}
		else if (buffer.compare("replace") == 0) {
			vector<string> params = parseWorkerArgs(getRemaining(stream), 2);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new Replacer(params)));
		}
		else if (buffer.compare("dump") == 0) {
			vector<string> params = parseWorkerArgs(getRemaining(stream), 1);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new Dumper(params)));
		}
		else {
			throw UnknownCommandException();
		}
	}
public:
	Parser(const string& scriptPath) {
		file.exceptions(ifstream::failbit);
		file.open(scriptPath);
		if (!file.is_open()) {
			throw FileOpeningException(scriptPath);
		}
	}
	void parse(map<unsigned int, Worker*>& workerStorage, Executor& commandStorage) {
		int lineNumber = 1;
		string buffer;
		try {
			getline(file, buffer);
			if (buffer.compare("desc") != 0) throw FormatException();
			getline(file, buffer);
			++lineNumber;
			while (buffer.compare("csed") != 0) {
				instantiateWorker(workerStorage, buffer);
				getline(file, buffer);
				++lineNumber;
			}
			++lineNumber;
			while (!file.eof()) {
				unsigned int workerNumber;
				file >> workerNumber;
				if (workerStorage.count(workerNumber) == 0) throw IdentifierNumberException();
				commandStorage.pushCommand(workerNumber);
				file.exceptions(ifstream::goodbit);
				file >> buffer;
				file.exceptions(ifstream::badbit);
			}
			commandStorage.validateCommandList(workerStorage);
		}
		catch (SimpleParseException& errInfo) {
			throw CommandDefinitionException(errInfo.what(), lineNumber);
		}
		catch (ifstream::failure& errInfo) {
			throw CommandDefinitionException("Wrong format", lineNumber);
		}
		catch (exception & errInfo) {
			throw CommandDefinitionException(string("Unexpected:\n") + string(errInfo.what()), lineNumber);
		}
		catch (...) {
			throw CommandDefinitionException("Unknown error", lineNumber);
		}
	}
};
int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "Invalid input" << endl;
		return 0;
	}
	try {
		Parser parser = Parser(string(argv[1]));
		map<unsigned int, Worker*>workerStorage;
		Executor environment;
		parser.parse(workerStorage, environment);
		environment.run(workerStorage);
		cout << "Done!" << endl;
	}
	catch (FileOpeningException& errInfo) {
		cout << errInfo.what() << endl;
	}
	catch (CommandDefinitionException & errInfo) {
		cout << "Parsing error:" << endl << errInfo.what() << endl;
	}
	catch (CommandExecutionException & errInfo) {
		cout << "Execution error:" << endl << errInfo.what() << endl;
	}
	catch (exception & errInfo) {
		cout << "Unexpected exception:" << endl << errInfo.what() << endl;
	}
	catch (...) {
		cout << "WTF" << endl;
	}
	return 0;
}