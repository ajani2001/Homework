#pragma once
#include <stdexcept>
#include <string>

class SimpleParseException :public std::exception {
public:
	virtual const char* what() const noexcept = 0;
};

class IdentifierNumberException : public SimpleParseException {
public:
	virtual const char* what() const noexcept;
};

class RWBlocksNumberException : public SimpleParseException {
public:
	virtual const char* what() const noexcept;
};

class FormatException :public SimpleParseException {
public:
	virtual const char* what() const noexcept;
};

class ArgumentCountException :public SimpleParseException {
public:
	virtual const char* what() const noexcept;
};

class UnknownCommandException :public SimpleParseException {
public:
	virtual const char* what() const noexcept;
};

class CommandDefinitionException : public std::exception {
	std::string whatToSay;
	void createMessage(const std::string& desc, int lineNumber);
public:
	CommandDefinitionException(const char* desc, int lineNumber);
	CommandDefinitionException(const std::string& desc, int lineNumber);
	virtual const char* what() const noexcept;
};

class FileOpeningException :public std::exception {
	std::string whatToSay;
public:
	FileOpeningException(const char* fileName);
	FileOpeningException(const std::string& fileName);
	virtual const char* what() const noexcept;
};

class CommandExecutionException : public std::exception {
	std::string whatToSay;
	void createMessage(const std::string& desc, int commandNumber);
public:
	CommandExecutionException(const char* desc, int commandNumber);
	CommandExecutionException(const std::string& desc, int commandNumber);
	virtual const char* what() const noexcept;
};