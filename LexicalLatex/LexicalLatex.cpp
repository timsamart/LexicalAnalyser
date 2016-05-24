// LexicalLatex.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "token.h"
#include <vector>
#include <algorithm>

bool loadlist(std::vector<std::string> &_wordlist, std::string myfilename) {
	FILE *fp_tokenlist;
	errno_t err;
	if ((err = fopen_s(&fp_tokenlist, myfilename.c_str(), "r")) != 0) {
		fprintf(stderr, "Cannot open config file %s!\n", myfilename);
	}
	fseek(fp_tokenlist, 0, SEEK_END);
	if (ftell(fp_tokenlist) == 0) {
		printf("File is empty.\n");
		exit(1);
	}
	else {
		rewind(fp_tokenlist);
	}
	char t;
	std::string tliststr;
	while ((t = fgetc(fp_tokenlist)) != EOF) {
		if (t == '\n') {
			_wordlist.push_back(tliststr);
			tliststr = "";
		}
		else {
			tliststr.push_back(t);
		}
	}
}

bool chkstring(std::string mstr, std::vector<std::string> &v) {
	if (std::find(v.begin(), v.end(), mstr) != v.end())
	{
		return true;
	}
	else {
		return false;
	}
}

int chkstringid(std::string mstr, std::vector<std::string> &v) {
	auto i = std::find(v.begin(), v.end(), mstr);
	if (i != v.end())
	{
		return i - v.begin();
	}
	else
	{
		return -1;
	}
}

int chktokenlist(std::vector<std::string> &tokenlist, std::vector<std::string> &v) {
	for (int i = 0; i < tokenlist.size(); i++) {
		if (std::find(v.begin(), v.end(), tokenlist[i]) != v.end())
		{
			return i;
		}
	}
	return -1;
}

bool chkopenbracket(std::vector<token> &_stub, long id) { // Open = true, Close = false;
	if (id == 0) {
		return true;
	}
	if (_stub[id].get_hierarchy() > _stub[id -1].get_hierarchy() ) {
		return true;
	}
	else {
		return false;
	}
}

// Beispiel 
// dapers1 wird zu \delta a_{pers,1} 
// Darho3 wird zu \Delta a \rho_{pers,1}
bool getwordparameters(std::string &_word, std::string &_variable, std::string &_parameter, std::string &_prefix, std::vector<std::string> _keywordlist, std::vector<std::string> _correskeywordlist) {
	int i = _word.size()-1;
	if (_word.size() == 0) { // is word bigger then 0
		return false;
	}
	if (isdigit(_word[0])) { //is the first char a digit then declare its a number
		_variable = _word;
		return true;
	}
	int wordlistid = chkstringid(_word, _keywordlist);
	if (wordlistid>=0) {
		_variable = _correskeywordlist[wordlistid];
		return true;
	}
	//check digits at the end as parameters
	while (true) {
		if (i <= 0) {
			break;
		}
		if (isdigit(_word[i])) {
			_parameter += _word[i];
		}
		else{
			break;
		}
		i--;
	}

	// check if the differentiation prefix is written
	int varstart = 0;
	switch (_word[0]) {
	case 'D': {
		varstart++;
		_prefix = "\\Delta";
		if (_word.size() > 1) {
			if (isdigit(_word[1])) { // if digit follows after differentiation it is expected to be the level of differentiation!
				varstart++;
				_prefix += "^" + _word[1];
			}
		}
		break;
	}
	case 'd': {
		varstart++;
		_prefix = "\\delta";
		if (_word.size() > 1) { // if digit follows after differentiation it is expected to be the level of differentiation!
			if (isdigit(_word[1])) {
				varstart++;
				_prefix += "^" + _word[1];
			}
		}
		break;
	}
	default: {
		_prefix = "";
	}
	}
	// check rest of parameters
	if (i <= (_word.size() - 1) && i > 0) {
		std::string mainword = _word.substr(varstart, i - varstart+1 );
		wordlistid = chkstringid(mainword, _keywordlist);
		if (wordlistid >= 0) {
			_variable = _correskeywordlist[wordlistid];
			_parameter = "_{" + _parameter + "}";
		}
		else {
			mainword = _word.substr(varstart + 1, i - varstart);
			_variable = _word[varstart];
			if (_parameter != "") {
				_parameter = "_{" + mainword + _parameter + "}";
			}
		}
	}
	else {
		_variable = _word[varstart];
	}

	
	
	return true;
}

int main(int argc, char *argv[])
{
	FILE *fp_config;
	/*---------Begin process cmd-line args and Redirection--------*/

	switch (argc) {
	case 1: // No parameters, use stdin
		printf("NO argument provided\n");
		fp_config = stdin;
		break;

	case 2: // One parameter, use .lan file supplied	
		errno_t err;
		if ((err = fopen_s(&fp_config, argv[1], "r")) != 0) {
			fprintf(stderr, "Cannot open config file %s!\n", argv[1]);
		}
		break;

	default:
		printf("Syntax: scanner [file]\n");
		return(1);
	}

	// BEGIN check if file empty
	
	fseek(fp_config, 0, SEEK_END);
	if (ftell(fp_config) == 0) {
		printf("File is empty.\n");
		exit(1);
	}
	else {
		rewind(fp_config);
	}

	// END check if file empty

	// load tokenlist
	std::vector<std::string> tokenlist;
	loadlist(tokenlist, "res_tokenlist.txt");

	std::vector<std::string> correstokenlist;
	loadlist(correstokenlist, "res_correstokenlist.txt");

	std::vector<std::string> rwordlist;
	loadlist(rwordlist, "res_wordlist.txt");

	std::vector<std::string> correswordlist;
	loadlist(correswordlist, "res_correswordlist.txt");

	std::vector<std::string> latexcommandslist;
	loadlist(latexcommandslist, "res_latexcommandslist.txt");
	// END load corresponding tokens list

	// BEGIN filewalking
	char c;
	std::vector<token> stub;
	std::vector<std::string> wordlist;
	std::string word;
	std::vector<int> hierarchytypes;
	int currenthierarchy = 0;
	int lasthierarchytype = 0;
	while ((c = fgetc(fp_config)) != EOF) {
		if ((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c==46 || c == 44) {
			word.push_back(c);
		}
		else {
			if (word != "") {
				wordlist.push_back(word);
				token token_line = token(TYPWORD, currenthierarchy, lasthierarchytype, wordlist.size()-1);
				stub.push_back(token_line);
				word = "";
			}

			if (c == '\n') {
				token token_line = token(TYPLINE, currenthierarchy, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == '(') {
				currenthierarchy++;
				lasthierarchytype = RNDBR;
				hierarchytypes.push_back(RNDBR);
				token token_line = token(TYPBRAC, currenthierarchy, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == '[') {
				currenthierarchy++;
				lasthierarchytype = SQUBR;
				hierarchytypes.push_back(SQUBR);
				token token_line = token(TYPBRAC, currenthierarchy, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == '{') {
				currenthierarchy++;
				lasthierarchytype = SWGBR;
				hierarchytypes.push_back(SWGBR);
				token token_line = token(TYPBRAC, currenthierarchy, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == '|') {
				if (lasthierarchytype == ABSBR) {
					currenthierarchy--;
					hierarchytypes.pop_back();
					if (currenthierarchy == 0) {
						lasthierarchytype = NONBR;
					}
					else {
						lasthierarchytype = hierarchytypes[hierarchytypes.size() - 1];
					}
					token token_line = token(TYPBRAC, currenthierarchy + 1, lasthierarchytype);
					stub.push_back(token_line);
				}
				else {
					currenthierarchy++;
					lasthierarchytype = ABSBR;
					hierarchytypes.push_back(ABSBR);
					token token_line = token(TYPBRAC, currenthierarchy, lasthierarchytype);
					stub.push_back(token_line);
				}

			}
			else if (c == ')') {
				currenthierarchy--;
				hierarchytypes.pop_back();
				if (currenthierarchy == 0) {
					lasthierarchytype = NONBR;
				}
				else {
					lasthierarchytype = hierarchytypes[hierarchytypes.size() - 1];
				}
				token token_line = token(TYPBRAC, currenthierarchy + 1, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == ']') {
				currenthierarchy--;
				hierarchytypes.pop_back();
				if (currenthierarchy == 0) {
					lasthierarchytype = NONBR;
				}
				else {
					lasthierarchytype = hierarchytypes[hierarchytypes.size() - 1];
				}
				token token_line = token(TYPBRAC, currenthierarchy + 1, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == '}') {
				currenthierarchy--;
				hierarchytypes.pop_back();
				if (currenthierarchy == 0) {
					lasthierarchytype = NONBR;
				}
				else {
					lasthierarchytype = hierarchytypes[hierarchytypes.size() - 1];
				}
				token token_line = token(TYPBRAC, currenthierarchy + 1, lasthierarchytype);
				stub.push_back(token_line);
			}
			else if (c == ' ') {

			}
			else {
				int tokenid = chkstringid(std::string(1, c), tokenlist);
				if (tokenid > -1) {
					token token_line = token(TYPTOKN, currenthierarchy, lasthierarchytype, tokenid);
					stub.push_back(token_line);
				}
			}
		}
	}
	// END filewalking

	// BEGIN translation

	int left = 0;
	int middle = 0;
	int right = 0;
	// Construct to map IDs in the complex environment:
	// Runner:				[left]		[middle]	[right]		----->
	// IDs:		[1]			[2]			[4]			[5]			[2]			[8]
	for (int i = 2; i < stub.size(); i++) {
		left = i - 2;
		middle = i - 1;
		right = i;
		if (stub[left].get_type() == 1 && stub[middle].get_type() == 4 && stub[right].get_type() == 1 && stub[middle].get_wordlistid() == 1) {
			int hierarchyentrypoint = 0;
			for (int a = left; a > 0; a--) {
				if (stub[a].get_hierarchy() < stub[left].get_hierarchy()) {
					hierarchyentrypoint = a + 1;
					break;
				}
			}
			token token_LCMD = token(TYPLCMD, stub[hierarchyentrypoint].get_hierarchy() - 1, stub[hierarchyentrypoint].get_hierarchy() - 1, 0);
			stub[hierarchyentrypoint].become_SWGBR_Left();
			stub[left].become_SWGBR_Right();
			stub[right].become_SWGBR_Left();
			stub.insert(stub.begin() + hierarchyentrypoint, token_LCMD);
			i++; left = i - 2;	middle = i - 1;	right = i;
			stub.erase(stub.begin() + middle);
			for (int a = right+1; a < stub.size(); a++) {
				if (stub[a].get_type()==TYPBRAC) {
					if (stub[a].get_hierarchy() == stub[middle].get_hierarchy()) {
						if(stub[a].get_lasthierarchytype() == stub[middle].get_lasthierarchytype()-1)
							i=a; left = i - 2;	middle = i - 1;	right = i;
							stub[i].become_SWGBR_Right();
							break;
					}
					
				}
			}
		}
	}

	std::string output;
	for (int i = 0; i < stub.size(); i++) {
		switch (stub[i].get_type()) {
		case TYPBRAC: {
			if (chkopenbracket(stub, i)) {
				switch (stub[i].get_lasthierarchytype()) {
				case RNDBR: {
					output += "\\left (";
					hierarchytypes.push_back(RNDBR);
					break;
				}
				case SQUBR: {
					output += "\\left [";
					hierarchytypes.push_back(SQUBR);
					break;
				}
				case SWGBR: {
					output += "\\left {"; 
					hierarchytypes.push_back(SWGBR);
					break;
				}
				case ABSBR: {
					output += "\\left |";
					hierarchytypes.push_back(ABSBR);
					break;
				}
				}
			}
			else {
					switch (hierarchytypes[hierarchytypes.size() - 1]) {
					case RNDBR: {
						output += "\\right )";
						hierarchytypes.pop_back();
						break;
					}
					case SQUBR: {
						output += "\\right ]";
						hierarchytypes.pop_back();
						break;
					}
					case SWGBR: {
						output += "\\right }";
						hierarchytypes.pop_back();
						break;
					}
					case ABSBR: {
						output += "\\right |";
						hierarchytypes.pop_back();
						break;
					}
					}
			}
			break;
		}
		case TYPWORD: {
			std::string variable = "";
			std::string parameters = "";
			std::string prefix = "";
			getwordparameters(wordlist[stub[i].get_wordlistid()], variable, parameters, prefix, rwordlist, correswordlist);
			output += prefix + " " + variable + parameters;
			break;
		}
		case TYPLINE: {
			output += "\n";
			break;
		}
		case TYPTOKN: {
			output += correstokenlist[stub[i].get_wordlistid()];
			break;
		}
		case TYPLCMD: {
			output += latexcommandslist[stub[i].get_wordlistid()];
		}
		}
	}
	std::ofstream saveFile("Save.txt");
	saveFile << output;
	saveFile.close();
    return 0;
}