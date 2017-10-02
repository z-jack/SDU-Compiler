// SDUCompiler.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

enum SYMBOLS {
	EXCEPTION = 0,
	NUMBER,
	FUNCTION,
	VARIABLE,
	CONST,
	IDENTIFIER,
	ASSIGN,
	BEGIN,
	END,
	IF,
	THEN,
	WHILE,
	DO,
	CALL,
	READ,
	WRITE,
	ODD,
	EQ,
	NE,
	LT,
	LE,
	GT,
	GE,
	ADD,
	SUB,
	MUL,
	DIV,
	COMMA,
	SEMICOLON,
	BRACKETSTART,
	BRACKETEND,
	EOS
};

struct Token
{
	SYMBOLS sym;
	void* value;
};

using namespace std;
void Lexer(string prog, Token*& tarr, int& size);
Token ReadBlock(string::iterator& it, string::iterator& end);

int main(int argc, char** argv)
{
	if (argc != 2) {
		printf("Usage: SDUCompiler filename");
		return 1;
	}
	ifstream file(argv[1]);
	string prog(static_cast<stringstream const&>(stringstream() << file.rdbuf()).str());
	Token* tarr;
	int size;
	Lexer(prog, tarr, size);
	return 0;
}

void Lexer(string prog, Token*& tarr, int& size) {
	string::iterator it;
	queue<Token> q;
	for (it = prog.begin(); it < prog.end(); it++) {
		q.push(ReadBlock(it, prog.end()));
	}
	tarr = new Token[q.size()];
	size = q.size();
	for (int i = 0; i < size; i++) {
		tarr[i] = q.front();
		q.pop();
	}
}

Token ReadBlock(string::iterator& it, string::iterator& end) {
	char ch = *it;
	while ((ch == ' ' || ch == '\n') && it < end)
	{
		it++;
		if (it != end)
			ch = *it;
	}
	if (it == end) {
		return Token{ EOS ,nullptr };
	}
	if (ch >= 'a' && ch <= 'z') {
		string current = "";
		while (((ch >= 'a' && ch <= 'z') || (ch >= '0'&&ch <= '9')) && it < end)
		{
			current += string(1, ch);
			it++;
			if (it != end) {
				ch = *it;
			}
		}
		if (ch != ' '&&ch != '\n') {
			it--;
		}
		if (current == "begin") {
			return Token{ BEGIN,nullptr };
		}
		else if (current == "end")
		{
			return Token{ END,nullptr };
		}
		else if (current == "const") {
			return Token{ CONST,nullptr };
		}
		else if (current == "var") {
			return Token{ VARIABLE,nullptr };
		}
		else if (current == "procedure") {
			return Token{ FUNCTION,nullptr };
		}
		else if (current == "odd") {
			return Token{ ODD,nullptr };
		}
		else if (current == "if") {
			return Token{ IF,nullptr };
		}
		else if (current == "then") {
			return Token{ THEN,nullptr };
		}
		else if (current == "while")
		{
			return Token{ WHILE,nullptr };
		}
		else if (current == "do") {
			return Token{ DO,nullptr };
		}
		else if (current == "call") {
			return Token{ CALL,nullptr };
		}
		else if (current == "read") {
			return Token{ READ,nullptr };
		}
		else if (current == "write") {
			return Token{ WRITE,nullptr };
		}
		else
		{
			return Token{ IDENTIFIER,(void*)current.c_str() };
		}
	}
	if(ch>='0'&&ch<='9')
	{
		string current = "";
		while ((ch >= '0'&&ch <= '9') && it < end)
		{
			current += string(1, ch);
			it++;
			if (it != end) {
				ch = *it;
			}
		}
		if (ch != ' '&&ch != '\n') {
			it--;
		}
		unsigned int i = stoi(current);
		return Token{ NUMBER, (void*)&i };
	}
	if (ch == '(')
		return Token{ BRACKETSTART,nullptr };
	if (ch == ')')
		return Token{ BRACKETEND,nullptr };
	if (ch == ',')
		return Token{ COMMA,nullptr };
	if (ch == ';')
		return Token{ SEMICOLON,nullptr };
	if (ch == '+')
		return Token{ ADD,nullptr };
	if (ch == '-')
		return Token{ SUB,nullptr };
	if (ch == '*')
		return Token{ MUL,nullptr };
	if (ch == '/')
		return Token{ DIV,nullptr };
	if (ch == '=')
		return Token{ EQ,nullptr };
	if (ch == '#')
		return Token{ NE,nullptr };
	if (ch == '<') {
		if (it + 1 == end) {
			return Token{ LT,nullptr };
		}
		if (*(it + 1) == '=') {
			it++;
			return Token{ LE,nullptr };
		}
		return Token{ LT,nullptr };
	}
	if (ch == '>') {
		if (it + 1 == end) {
			return Token{ GT,nullptr };
		}
		if (*(it + 1) == '=') {
			it++;
			return Token{ GE,nullptr };
		}
		return Token{ GT,nullptr };
	}
	if (ch == ':') {
		if (it + 1 == end) {
			return Token{ EXCEPTION,nullptr };
		}
		if (*(it + 1) == '=') {
			it++;
			return Token{ ASSIGN,nullptr };
		}
		return Token{ EXCEPTION,nullptr };
	}
}

