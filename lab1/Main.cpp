#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <forward_list>
#include <list>
using namespace std;

class Worker {
protected:
	const vector<string> params;
public:
	Worker(const vector<string>& params) : params(params) {}
	virtual void work(string& textStorage) = 0;
};
class FileReader : public Worker {
	static size_t instanceCount;
public:
	FileReader(const vector<string>& params) : Worker(params) {
		if (++instanceCount > 1) throw 2;
	}
	virtual void work(string& textStorage) {
		ifstream file;
		file.open(params[0]);
		if (!file.is_open()) throw 3;
		stringstream buffer;
		file >> buffer.rdbuf();
		textStorage = buffer.str();
	}
};
size_t FileReader::instanceCount = 0;
class FileWriter : public Worker {
	static size_t instanceCount;
public:
	FileWriter(const vector<string>& params) : Worker(params) {
		if (++instanceCount > 1) throw 4;
	}
	virtual void work(string& textStorage) {
		ofstream file;
		file.open(params[0]);
		if (!file.is_open()) throw 5;
		file << textStorage;
	}
};
size_t FileWriter::instanceCount = 0;
class GrepWorker : public Worker {
public:
	GrepWorker(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		string buffer;
		istringstream oldStorageStream;
		//oldStorageStream.rdbuf()->pubsetbuf(const_cast<char*>(textStorage.data()), textStorage.length() + 1);
		oldStorageStream.str(textStorage);
		ostringstream newStorageStream;
		while (!oldStorageStream.eof()) {
			getline(oldStorageStream, buffer);
			if (buffer.find(params[0]) != string::npos)newStorageStream << buffer << endl;
		}
		textStorage = newStorageStream.str();
	}
};
class Sorter : public Worker {
public:
	Sorter(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		string buffer;
		istringstream oldStorageStream;
		//oldStorageStream.rdbuf()->pubsetbuf(const_cast<char*>(textStorage.data()), textStorage.length() + 1);
		oldStorageStream.str(textStorage);
		ostringstream newStorageStream;
		forward_list<string> sorter;
		while (!oldStorageStream.eof()) {
			getline(oldStorageStream, buffer);
			sorter.push_front(buffer);
		}
		sorter.sort();
		for (forward_list<string>::iterator iter = sorter.begin(); iter != sorter.end(); ++iter) {
			newStorageStream << (*iter) << endl;
		}
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
class Dumper : public Worker {
public:
	Dumper(const vector<string>& params) : Worker(params) {}
	virtual void work(string& textStorage) {
		ofstream file;
		file.open(params[0]);
		if (!file.is_open()) throw 6;
		stringstream buffer;
		file.set_rdbuf(buffer.rdbuf());
		buffer.str(textStorage);
	}
};
class Executor {
	list<unsigned int> commandList;
	string textStorage;
	size_t textStorageCapacity;
public:
	void pushCommand(unsigned int cNumber) {
		commandList.push_back(cNumber);
	}
	void validateCommandList(const map<unsigned int, Worker*>& workerStorage) {
		if (commandList.size() < 2) throw 7;
		string ty1, ty2;
		ty1 = typeid(*workerStorage.at(*(commandList.begin()))).name();
		ty2 = typeid(FileReader).name();
		if (typeid(*workerStorage.at(*(commandList.begin()))) != typeid(FileReader) ||
			typeid(*workerStorage.at(*(--(commandList.end())))) != typeid(FileWriter)) throw 8;
	}
	void run(map<unsigned int, Worker*>& workerStorage) {
		for (list<unsigned int>::iterator iter = commandList.begin(); iter != commandList.end(); ++iter) {
			workerStorage.at(*iter)->work(textStorage);
		}
	}
};
class Xxxeption {};
class Parser {
private:
	ifstream file;
	void instantiateWorker(map<unsigned int, Worker*>& workerStorage, const string& workerConfig) {
		istringstream stream(workerConfig);
		unsigned int workerNumber;
		stream >> workerNumber;
		if (workerStorage.count(workerNumber) != 0) throw 9;
		if (stream.fail()) throw 10;
		char c;
		stream >> c;
		if (c != '=') throw 11;
		string buffer;
		stream >> buffer;
		vector<string> params;
		if (buffer.compare("readfile") == 0) {
			stream >> buffer;
			if (buffer.empty()) throw 12;
			params.push_back(buffer);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new FileReader(params)));
		}
		else if (buffer.compare("writefile") == 0) {
			stream >> buffer;
			if (buffer.empty()) throw 13;
			params.push_back(buffer);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new FileWriter(params)));
		}
		else if (buffer.compare("grep") == 0) {
			stream >> buffer;
			if (buffer.empty()) throw 14;
			params.push_back(buffer);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new GrepWorker(params)));
		}
		else if (buffer.compare("sort") == 0) {
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new Sorter(params)));
		}
		else if (buffer.compare("replace") == 0) {
			stream >> buffer;
			if (buffer.empty()) throw 15;
			params.push_back(buffer);
			stream >> buffer;
			if (buffer.empty()) throw 16;
			params.push_back(buffer);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new Replacer(params)));
		}
		else if (buffer.compare("dump") == 0) {
			stream >> buffer;
			if (buffer.empty()) throw 17;
			params.push_back(buffer);
			workerStorage.insert(pair<unsigned int, Worker*>(workerNumber, new Dumper(params)));
		}
		else {
			throw 18;
		}
	}
public:
	Parser(const string& scriptPath) {
		file.open(scriptPath);
		if (!file.is_open()) {
			throw 19;
		}
	}
	void parse(map<unsigned int, Worker*>& workerStorage, Executor& commandStorage) {
		string buffer;
		getline(file, buffer);
		if (buffer.compare("desc") != 0) throw 20;
		getline(file, buffer);
		while (buffer.compare("csed") != 0) {
			instantiateWorker(workerStorage, buffer);
			getline(file, buffer);
		}
		while (!file.eof()) {
			unsigned int workerNumber;
			file >> workerNumber;
			if (file.fail()) throw 21;
			if (workerStorage.count(workerNumber) == 0) throw 22;
			commandStorage.pushCommand(workerNumber);
			file >> buffer;
		}
		commandStorage.validateCommandList(workerStorage);
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
		return 0;
	}
	catch (int a) {
		cout << "myException: " << a << endl;
		return 0;
	}
	catch (...) {
		cout << "Unknown error" << endl;
		return 0;
	}
}