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

enum ASSEMBLE {
	LIT,
	LOD,
	STO,
	CAL,
	INT,
	JMP,
	JPC,
	OPR
};

struct Token
{
	SYMBOLS sym;
	void* value;
};

struct Entry
{
	std::string name;
	SYMBOLS sym;
	int value;
	int level;
	unsigned int address;
};

#define Assert(a,b) \
if(a!=b){throw std::exception("Unexpected Symbol.");}\
else
#define AssertCond(a) \
if(!a){throw std::exception("Unexpected Symbol.");}\
else

using namespace std;
void Lexer(string prog, Token*& tarr, int& size);
Token ReadBlock(string::iterator& it, string::iterator& end);
SYMBOLS GetSymbol(Token* t);
SYMBOLS NextSymbol(Token*& tarr, int& size, bool flag);
void Maintain(string name, SYMBOLS s, int value, void* ptr);
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

int level = 0;
Entry* etable = new struct Entry[1024];
int idx = 0;
int vct = 3;

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
	try {
		Parser(t, s);
	}
	catch (exception e) {
		fprintf(stderr, "<Parser Error> %s\n", e.what());
		return 1;
	}
	delete[] tarr;
	return 0;
}

void Lexer(string prog, Token*& tarr, int& size) {
	string::iterator it;
	queue<Token> q;
	for (it = prog.begin(); it < prog.end(); it++) {
		q.push(ReadBlock(it, prog.end()));
		char* c = (char*)q.back().value;
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
			char* c = new char[current.size() + 1];
			strcpy(c, current.c_str());
			return Token{ IDENTIFIER,(void*)c };
		}
	}
	if (ch >= '0'&&ch <= '9')
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
		unsigned int* i = new unsigned int(stoi(current));
		return Token{ NUMBER, (void*)i };
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
	return Token{ EXCEPTION, nullptr };
}

SYMBOLS GetSymbol(Token* t) {
	return t[0].sym;
}

SYMBOLS NextSymbol(Token*& tarr, int& size, bool flag = true) {
	if (size <= 0)
		if (flag)
			throw exception("Access Denied: Out of Index.");
		else
			return EXCEPTION;
	tarr++;
	size--;
	return GetSymbol(tarr);
}

void Maintain(string name, SYMBOLS s, int value = 0, unsigned int ptr = 0)
{
	if (s != CONST&&s != VARIABLE&&s != FUNCTION)
		throw exception("Not Supported Symbol.");
	for (int i = idx - 1; i >= 0; i--) {
		if (etable[i].level < level)break;
		if (etable[i].name==name) {
			switch (etable[i].sym)
			{
			case CONST:
				throw exception("Redefine Constant.");
				break;
			case VARIABLE:
				if (s == VARIABLE)
					return;
				else
					throw exception("Redefine Variable to Other Type.");
				break;
			case FUNCTION:
				throw exception("Redefine Function.");
				break;
			default:
				throw exception("Not Supported Symbol.");
				break;
			}
		}
	}
	Entry e = Entry{ name,s,value,level,0 };
	if (s == VARIABLE) {
		e.address = vct++;
	}
	etable[idx++] = e;
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
	Assert(GetSymbol(tarr), CONST);
	NextSymbol(tarr, size);
	ConstDef(tarr, size);
	while (GetSymbol(tarr) == COMMA)
	{
		NextSymbol(tarr, size);
		ConstDef(tarr, size);
	}
	Assert(GetSymbol(tarr), SEMICOLON);
	NextSymbol(tarr, size);
}

void ConstDef(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), IDENTIFIER);
	string name((char*)(tarr->value));
	Assert(NextSymbol(tarr, size), EQ);
	Assert(NextSymbol(tarr, size), NUMBER);
	int value(*(int*)(tarr->value));
	Maintain(name, CONST, value);
	NextSymbol(tarr, size);
}

void VarDec(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), VARIABLE);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	string name((char*)(tarr->value));
	Maintain(name, VARIABLE);
	while (NextSymbol(tarr, size) == COMMA)
	{
		Assert(NextSymbol(tarr, size), IDENTIFIER);
		name = string((char*)(tarr->value));
		Maintain(name, VARIABLE);
	}
	Assert(GetSymbol(tarr), SEMICOLON);
	NextSymbol(tarr, size);
}

void FuncDec(Token *& tarr, int & size)
{
	FuncHead(tarr, size);
	int v = vct;
	level++;
	vct = 3;
	if (level > 3) {
		throw exception("Stack Overflow.");
	}
	SubProgram(tarr, size);
	level--;
	vct = v;
	Assert(GetSymbol(tarr), SEMICOLON);
	NextSymbol(tarr, size);
	if (GetSymbol(tarr) == FUNCTION) {
		FuncDec(tarr, size);
	}
}

void FuncHead(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), FUNCTION);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	string name((char*)(tarr->value));
	Maintain(name, FUNCTION);
	Assert(NextSymbol(tarr, size), SEMICOLON);
	NextSymbol(tarr, size);
}

void Statement(Token *& tarr, int & size)
{
	SYMBOLS sym = GetSymbol(tarr);
	if (sym == IDENTIFIER) {
		AssignStm(tarr, size);
	}
	else if (sym == IF)
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
	Assert(GetSymbol(tarr), IDENTIFIER);
	Assert(NextSymbol(tarr, size), ASSIGN);
	NextSymbol(tarr, size);
	Expression(tarr, size);
}

void CondStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), IF);
	NextSymbol(tarr, size);
	Cond(tarr, size);
	Assert(GetSymbol(tarr), THEN);
	NextSymbol(tarr, size);
	Statement(tarr, size);
}

void WhileStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), WHILE);
	NextSymbol(tarr, size);
	Cond(tarr, size);
	Assert(GetSymbol(tarr), DO);
	NextSymbol(tarr, size);
	Statement(tarr, size);
}

void CallStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), CALL);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	NextSymbol(tarr, size);
}

void ReadStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), READ);
	Assert(NextSymbol(tarr, size), BRACKETSTART);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	while (NextSymbol(tarr, size) == COMMA) {
		Assert(NextSymbol(tarr, size), IDENTIFIER);
	}
	Assert(GetSymbol(tarr), BRACKETEND);
	NextSymbol(tarr, size);
}

void WriteStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), WRITE);
	Assert(NextSymbol(tarr, size), BRACKETSTART);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	while (NextSymbol(tarr, size) == COMMA) {
		Assert(NextSymbol(tarr, size), IDENTIFIER);
	}
	Assert(GetSymbol(tarr), BRACKETEND);
	NextSymbol(tarr, size);
}

void CompStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), BEGIN);
	NextSymbol(tarr, size);
	Statement(tarr, size);
	while (GetSymbol(tarr) == SEMICOLON) {
		NextSymbol(tarr, size);
		Statement(tarr, size);
	}
	Assert(GetSymbol(tarr), END);
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
		AssertCond(IsRelOpt(GetSymbol(tarr)));
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
	if (sym == IDENTIFIER || sym == NUMBER) {
		NextSymbol(tarr, size);
	}
	else Assert(sym, BRACKETSTART) {
		NextSymbol(tarr, size);
		Expression(tarr, size);
		Assert(GetSymbol(tarr), BRACKETEND);
		NextSymbol(tarr, size);
	}
}
