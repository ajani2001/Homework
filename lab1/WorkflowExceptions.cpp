#include "WorkflowExceptions.h"
#include <string>
#include <sstream>

const char* IdentifierNumberException::what() const noexcept {
	return "Error: there must be exactly one command match unique number";
}

const char* RWBlocksNumberException::what() const noexcept {
	return "Error: there must be exactly one readfile and writefile command";
}

const char* FormatException::what() const noexcept {
	return "Wrong format";
}

const char* ArgumentCountException::what() const noexcept {
	return "Wrong argument number";
}

const char* UnknownCommandException::what() const noexcept {
	return "Unknown command name";
}

void CommandDefinitionException::createMessage(const std::string& desc, int lineNumber) {
	std::stringstream buffer;
	buffer << desc << std::endl << "line: " << lineNumber << std::endl;
	whatToSay = buffer.str();
}
CommandDefinitionException::CommandDefinitionException(const char* desc, int lineNumber) {
	createMessage(std::string(desc), lineNumber);
}
CommandDefinitionException::CommandDefinitionException(const std::string& desc, int lineNumber) {
	createMessage(desc, lineNumber);
}
const char* CommandDefinitionException::what() const noexcept {
	return whatToSay.c_str();
}

FileOpeningException::FileOpeningException(const char* fileName) {
	whatToSay = std::string("File ") + std::string(fileName) + std::string(" cannot be opened");
}
FileOpeningException::FileOpeningException(const std::string& fileName) {
	whatToSay = std::string("File ") + fileName + std::string(" cannot be opened");
}
const char* FileOpeningException::what() const noexcept {
	return whatToSay.c_str();
}

void CommandExecutionException::createMessage(const std::string& desc, int commandNumber) {
	std::stringstream buffer;
	buffer << desc << std::endl << "command #" << commandNumber << std::endl;
	whatToSay = buffer.str();
}
CommandExecutionException::CommandExecutionException(const char* desc, int commandNumber) {
	createMessage(std::string(desc), commandNumber);
}
CommandExecutionException::CommandExecutionException(const std::string& desc, int commandNumber) {
	createMessage(desc, commandNumber);
}
const char* CommandExecutionException::what() const noexcept {
	return whatToSay.c_str();
}
