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
SYMBOLS GetSymbol(Token* t);
SYMBOLS NextSymbol(Token*& tarr, int& size, bool flag);
bool IsRelOpt(SYMBOLS s);
bool IsAddSub(SYMBOLS s);
bool IsMulDiv(SYMBOLS s);
void Parser(Token*& tarr, int& size);
void SubProgram(Token*& tarr, int& size);
void ConstDec(Token*& tarr, int& size);
void ConstDef(Token*& tarr, int& size);
void VarDec(Token*& tarr, int& size);
void FuncDec(Token*& tarr, int& size);
void FuncHead(Token*& tarr, int& size);
void Statement(Token*& tarr, int& size);
void AssignStm(Token*& tarr, int& size);
void CondStm(Token*& tarr, int& size);
void WhileStm(Token*& tarr, int& size);
void CallStm(Token*& tarr, int& size);
void ReadStm(Token*& tarr, int& size);
void WriteStm(Token*& tarr, int& size);
void CompStm(Token*& tarr, int& size);
void EmptyStm(Token*& tarr, int& size);
void Cond(Token*& tarr, int& size);
void Expression(Token*& tarr, int& size);
void Term(Token*& tarr, int& size);
void Factor(Token*& tarr, int& size);

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
	Token* t = tarr;
	int s = size;
	Parser(t, s);
	return 0;
}

void Lexer(string prog, Token*& tarr, int& size) {
	string::iterator it;
	queue<Token> q;
	for (it = prog.begin(); it < prog.end(); it++) {
		q.push(ReadBlock(it, prog.end()));
	}
	q.push(Token{ EOS, nullptr });
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

SYMBOLS GetSymbol(Token* t) {
	return t[0].sym;
}

SYMBOLS NextSymbol(Token*& tarr, int& size, bool flag = true) {
	if (size <= 0)
		if (flag)
			throw exception();
		else
			return EXCEPTION;
	tarr++;
	size--;
	return GetSymbol(tarr);
}

bool IsRelOpt(SYMBOLS s) {
	return s == EQ || s == NE || s == LT || s == LE || s == GT || s == GE;
}

bool IsAddSub(SYMBOLS s) {
	return s == ADD || s == SUB;
}

bool IsMulDiv(SYMBOLS s) {
	return s == MUL || s == DIV;
}

void Parser(Token*& tarr, int& size) {
	SubProgram(tarr, size);
	if (size > 0 && GetSymbol(tarr) != EOS) {
		throw exception();
	}
	printf("parser finish");
}

void SubProgram(Token*& tarr, int& size) {
	if (GetSymbol(tarr) == CONST)
		ConstDec(tarr, size);
	if (GetSymbol(tarr) == VARIABLE)
		VarDec(tarr, size);
	if (GetSymbol(tarr) == FUNCTION)
		FuncDec(tarr, size);
	Statement(tarr, size);
}

void ConstDec(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != CONST) {
		throw exception();
	}
	NextSymbol(tarr, size);
	ConstDef(tarr, size);
	while (GetSymbol(tarr) == COMMA)
	{
		NextSymbol(tarr, size);
		ConstDef(tarr, size);
	}
	if (GetSymbol(tarr) != SEMICOLON) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void ConstDef(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != IDENTIFIER) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != EQ) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != NUMBER) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void VarDec(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != VARIABLE) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != IDENTIFIER) {
		throw exception();
	}
	while (NextSymbol(tarr,size)==COMMA)
	{
		if (NextSymbol(tarr, size) != IDENTIFIER) {
			throw exception();
		}
	}
	if (GetSymbol(tarr) != SEMICOLON) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void FuncDec(Token *& tarr, int & size)
{
	FuncHead(tarr, size);
	SubProgram(tarr, size);
	if (GetSymbol(tarr) != SEMICOLON) {
		throw exception();
	}
	NextSymbol(tarr, size);
	if (GetSymbol(tarr) == FUNCTION) {
		FuncDec(tarr, size);
	}
}

void FuncHead(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != FUNCTION) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != IDENTIFIER) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != SEMICOLON) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void Statement(Token *& tarr, int & size)
{
	SYMBOLS sym = GetSymbol(tarr);
	if (sym == IDENTIFIER) {
		AssignStm(tarr, size);
	}
	else if(sym ==IF)
	{
		CondStm(tarr, size);
	}
	else if (sym == WHILE) {
		WhileStm(tarr, size);
	}
	else if (sym == CALL) {
		CallStm(tarr, size);
	}
	else if (sym == READ) {
		ReadStm(tarr, size);
	}
	else if (sym == WRITE) {
		WriteStm(tarr, size);
	}
	else if (sym == BEGIN) {
		CompStm(tarr, size);
	}
}

void AssignStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != IDENTIFIER) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != ASSIGN) {
		throw exception();
	}
	NextSymbol(tarr, size);
	Expression(tarr, size);
}

void CondStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != IF) {
		throw exception();
	}
	NextSymbol(tarr, size);
	Cond(tarr, size);
	if (GetSymbol(tarr) != THEN) {
		throw exception();
	}
	NextSymbol(tarr, size);
	Statement(tarr, size);
}

void WhileStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != WHILE) {
		throw exception();
	}
	NextSymbol(tarr, size);
	Cond(tarr, size);
	if (GetSymbol(tarr) != DO) {
		throw exception();
	}
	NextSymbol(tarr, size);
	Statement(tarr, size);
}

void CallStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != CALL) {
		throw exception();
	}
	if (NextSymbol(tarr,size) != IDENTIFIER) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void ReadStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != READ) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != BRACKETSTART) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != IDENTIFIER) {
		throw exception();
	}
	while (NextSymbol(tarr, size) == COMMA) {
		if (NextSymbol(tarr, size) != IDENTIFIER) {
			throw exception();
		}
	}
	if (GetSymbol(tarr) != BRACKETEND) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void WriteStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != WRITE) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != BRACKETSTART) {
		throw exception();
	}
	if (NextSymbol(tarr, size) != IDENTIFIER) {
		throw exception();
	}
	while (NextSymbol(tarr, size) == COMMA) {
		if (NextSymbol(tarr, size) != IDENTIFIER) {
			throw exception();
		}
	}
	if (GetSymbol(tarr) != BRACKETEND) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void CompStm(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) != BEGIN) {
		throw exception();
	}
	NextSymbol(tarr, size);
	Statement(tarr, size);
	while (GetSymbol(tarr) == SEMICOLON) {
		NextSymbol(tarr, size);
		Statement(tarr, size);
	}
	if (GetSymbol(tarr) != END) {
		throw exception();
	}
	NextSymbol(tarr, size);
}

void EmptyStm(Token *& tarr, int & size)
{
}

void Cond(Token *& tarr, int & size)
{
	if (GetSymbol(tarr) == ODD) {
		NextSymbol(tarr, size);
		Expression(tarr, size);
	}
	else {
		Expression(tarr, size);
		if (!IsRelOpt(GetSymbol(tarr))) {
			throw exception();
		}
		NextSymbol(tarr, size);
		Expression(tarr, size);
	}
}

void Expression(Token *& tarr, int & size)
{
	if (IsAddSub(GetSymbol(tarr))) {
		NextSymbol(tarr, size);
	}
	Term(tarr, size);
	while (IsAddSub(GetSymbol(tarr)))
	{
		NextSymbol(tarr, size);
		Term(tarr, size);
	}
}

void Term(Token *& tarr, int & size)
{
	Factor(tarr, size);
	while (IsMulDiv(GetSymbol(tarr)))
	{
		NextSymbol(tarr, size);
		Factor(tarr, size);
	}
}

void Factor(Token *& tarr, int & size)
{
	SYMBOLS sym = GetSymbol(tarr);
	if (sym == IDENTIFIER || sym==NUMBER) {
		NextSymbol(tarr, size);
	}
	else if (sym == BRACKETSTART) {
		NextSymbol(tarr, size);
		Expression(tarr, size);
		if (GetSymbol(tarr) != BRACKETEND) {
			throw exception();
		}
		NextSymbol(tarr, size);
	}
	else {
		throw exception();
	}
}
