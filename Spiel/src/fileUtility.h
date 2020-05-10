#pragma once
#include<iostream>
#include<fstream>
#include<string>

bool findHeadPosition(std::ifstream& filename_, std::string_view searchterm_)
{
	std::string term;
	while (!filename_.eof()) {
		std::getline(filename_, term);
		if (searchterm_.compare(term.c_str()) == 0)
		{
			return true;
		}
	}
	return false;
}

bool findInHeadPosition(std::ifstream& filename_, std::string_view searchterm_)
{
	std::string term;
	while (term != "#") {
		std::getline(filename_, term);
		if (searchterm_.compare(term.c_str()) == 0)
		{
			return true;
		}
	}
	return false;
}

void insertInFile(std::ofstream& filename_, std::string_view insertTerm_)
{
	filename_ << '\n' << insertTerm_;
}