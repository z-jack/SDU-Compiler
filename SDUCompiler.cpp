// SDUCompiler.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#pragma region DataStruct

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

char** AsmList = new char*[8]{ "LIT", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "OPR" };

enum OPERATOR {
	EXIT,
	PLUS,
	SUBTRACT,
	MULTIPLE,
	DIVISION,
	EQUAL,
	NOTEQUAL,
	GREATTHAN,
	GREATEQUAL,
	LESSTHAN,
	LESSEQUAL,
	IN,
	OUT
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
	int address;
};

struct AsmStm
{
	ASSEMBLE f;
	int l;
	int a;
};

#pragma endregion

#pragma region Declare

#define Assert(a,b) \
if(a!=b){throw std::exception("Unexpected Symbol.");}\
else
#define AssertCond(a) \
if(!a){throw std::exception("Unexpected Symbol.");}\
else

using namespace std;
template<typename T>
T* Q2List(queue<T>& q, int& size);
template<typename T>
void QStuff(queue<T>& q, int size);
template<typename T>
void QShift(queue<T>& q, int size);
template<typename T>
void QConcat(queue<T>& q1, queue<T>& q2);
void Lexer(string prog, Token*& tarr, int& size);
Token ReadBlock(string::iterator& it, string::iterator& end);
SYMBOLS GetSymbol(Token* t);
SYMBOLS NextSymbol(Token*& tarr, int& size, bool flag);
void Maintain(string name, SYMBOLS s, int value, int ptr);
void LookUp(string name, Entry& e);
void ValidVar(Entry e);
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
void ValidWrite(Entry e);
void WriteStm(Token*& tarr, int& size);
void CompStm(Token*& tarr, int& size);
void EmptyStm(Token*& tarr, int& size);
void Cond(Token*& tarr, int& size);
void Expression(Token*& tarr, int& size);
void Term(Token*& tarr, int& size);
void Factor(Token*& tarr, int& size);
void Simulator(AsmStm* aarr, int size);

int level = 0;
int idx = 0;
int vct = 3;
Entry* etable = new struct Entry[1024];
string* stk = new string[4];
queue<AsmStm> asmq;

#pragma endregion

int main(int argc, char** argv)
{
	if (argc != 2) {
		printf("Usage: SDUCompiler filename");
		return -1;
	}
	ifstream file(argv[1]);
	string prog(static_cast<stringstream const&>(stringstream() << file.rdbuf()).str());
	Token* tarr;
	int size;
	try {
		Lexer(prog, tarr, size);
	}
	catch (exception e) {
		fprintf(stderr, "<Lexer Error> %s\n", e.what());
		return 1;
	}
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
	delete[] etable;
	AsmStm* aarr = Q2List(asmq, size);
	for (int i = 0; i < size; i++) {
		AsmStm a = aarr[i];
		printf("%d: %s, %d, %d\n", i, AsmList[a.f], a.l, a.a);
	}
	printf("===== Running =====\n");
	try {
		Simulator(aarr, size);
	}
	catch (exception e) {
		fprintf(stderr, "<Simulator Error> %s\n", e.what());
		return 2;
	}
	return 0;
}

#pragma region Lexer

void Lexer(string prog, Token*& tarr, int& size) {
	string::iterator it;
	queue<Token> q;
	for (it = prog.begin(); it < prog.end(); it++) {
		q.push(ReadBlock(it, prog.end()));
		if (it == prog.end())
			break;
	}
	q.push(Token{ EOS, nullptr });
	tarr = Q2List(q, size);
}

Token ReadBlock(string::iterator& it, string::iterator& end) {
	char ch = *it;
	while ((ch == ' ' || ch == '\n' || ch == '\t') && it < end)
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
	if (ch == '.') {
		return Token{ EOS,nullptr };
	}
	return Token{ EXCEPTION, nullptr };
}

#pragma endregion

#pragma region Parser

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

void Maintain(string name, SYMBOLS s, int value = 0, int ptr = 0)
{
	if (idx >= 1024)
		throw exception("Too many Identifier.");
	if (s != CONST&&s != VARIABLE&&s != FUNCTION)
		throw exception("Not Supported Symbol.");
	for (int i = idx - 1; i >= 0; i--) {
		if (etable[i].level < level)break;
		if (etable[i].name == name) {
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
	Entry e = Entry{ name,s,value,level,ptr };
	if (s == VARIABLE) {
		e.address = vct++;
	}
	etable[idx++] = e;
}

void LookUp(string name, Entry & e)
{
	e = Entry();
	e.sym = EXCEPTION;
	for (int i = 0; i < idx; i++) {
		Entry et = etable[i];
		if (et.level > level) continue;
		if (et.name == name)
			e = et;
		if (et.sym == FUNCTION && et.level < level && et.name != stk[et.level]) {
			int l = et.level;
			while (++i < idx && etable[i].level > l);
		}
	}
}

void ValidVar(Entry e)
{
	if (e.sym == EXCEPTION) {
		throw exception("Undefined Identifier.");
	}
	else if (e.sym == CONST) {
		throw exception("Trying to Assign to Const.");
	}
	else if (e.sym == FUNCTION) {
		throw exception("Trying to Assign to Function.");
	}
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
	AssertCond((size > 0 && GetSymbol(tarr) == EOS) || size <= 0);
}

void SubProgram(Token*& tarr, int& size) {
	queue<AsmStm> tmp;
	bool flag = false;
	if (GetSymbol(tarr) == CONST)
		ConstDec(tarr, size);
	if (GetSymbol(tarr) == VARIABLE)
		VarDec(tarr, size);
	if (GetSymbol(tarr) == FUNCTION) {
		flag = true;
		tmp = asmq;
		asmq = queue<AsmStm>();
		int ts = tmp.size() + 1;
		QStuff(asmq, ts);
		FuncDec(tarr, size);
		tmp.push(AsmStm{ JMP, 0, (int)asmq.size() });
		QShift(asmq, ts);
		QConcat(tmp, asmq);
		asmq = tmp;
	}
	asmq.push(AsmStm{ INT, 0, vct });
	Statement(tarr, size);
	asmq.push(AsmStm{ OPR, 0, EXIT });
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
	stk[level] = name;
	Maintain(name, FUNCTION, 0, asmq.size());
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
	string name((char*)tarr->value);
	Entry e;
	LookUp(name, e);
	ValidVar(e);
	Assert(NextSymbol(tarr, size), ASSIGN);
	NextSymbol(tarr, size);
	Expression(tarr, size);
	asmq.push(AsmStm{ STO, level - e.level, e.address });
}

void CondStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), IF);
	NextSymbol(tarr, size);
	Cond(tarr, size);
	Assert(GetSymbol(tarr), THEN);
	NextSymbol(tarr, size);
	queue<AsmStm> tmp = asmq;
	asmq = queue<AsmStm>();
	int ts = tmp.size() + 1;
	QStuff(asmq, ts);
	Statement(tarr, size);
	tmp.push(AsmStm{ JPC, 0, (int)asmq.size() });
	QShift(asmq, ts);
	QConcat(tmp, asmq);
	asmq = tmp;
}

void WhileStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), WHILE);
	NextSymbol(tarr, size);
	int s = asmq.size();
	Cond(tarr, size);
	Assert(GetSymbol(tarr), DO);
	NextSymbol(tarr, size);
	queue<AsmStm> tmp = asmq;
	asmq = queue<AsmStm>();
	int ts = tmp.size() + 1;
	QStuff(asmq, ts);
	Statement(tarr, size);
	asmq.push(AsmStm{ JMP, 0, s });
	tmp.push(AsmStm{ JPC, 0, (int)asmq.size() });
	QShift(asmq, ts);
	QConcat(tmp, asmq);
	asmq = tmp;
}

void CallStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), CALL);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	string name((char*)tarr->value);
	Entry e;
	LookUp(name, e);
	if (e.sym == EXCEPTION) {
		throw exception("Undefined Identifier.");
	}
	else if (e.sym == CONST) {
		throw exception("Trying to Call Const.");
	}
	else if (e.sym == VARIABLE) {
		throw exception("Trying to Call Variable.");
	}
	asmq.push(AsmStm{ CAL, level - e.level, e.address });
	NextSymbol(tarr, size);
}

void ReadStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), READ);
	Assert(NextSymbol(tarr, size), BRACKETSTART);
	Assert(NextSymbol(tarr, size), IDENTIFIER);
	string name((char*)tarr->value);
	Entry e;
	LookUp(name, e);
	ValidVar(e);
	asmq.push(AsmStm{ OPR, 0, IN });
	asmq.push(AsmStm{ STO,level - e.level,e.address });
	while (NextSymbol(tarr, size) == COMMA) {
		Assert(NextSymbol(tarr, size), IDENTIFIER);
		name = string((char*)tarr->value);
		LookUp(name, e);
		ValidVar(e);
		asmq.push(AsmStm{ OPR, 0, IN });
		asmq.push(AsmStm{ STO,level - e.level,e.address });
	}
	Assert(GetSymbol(tarr), BRACKETEND);
	NextSymbol(tarr, size);
}

void ValidWrite(Entry e)
{
	switch (e.sym)
	{
	case EXCEPTION:
		throw exception("Undefined Identifier.");
		break;
	case VARIABLE:
		asmq.push(AsmStm{ LOD,level - e.level,e.address });
		break;
	case CONST:
		asmq.push(AsmStm{ LIT,0,e.value });
		break;
	case FUNCTION:
		throw exception("Tring to Write Function.");
		break;
	default:
		throw exception("Unknown Exception.");
		break;
	}
}

void WriteStm(Token *& tarr, int & size)
{
	Assert(GetSymbol(tarr), WRITE);
	Assert(NextSymbol(tarr, size), BRACKETSTART);
	NextSymbol(tarr, size);
	Expression(tarr, size);
	asmq.push(AsmStm{ OPR, 0, OUT });
	while (GetSymbol(tarr) == COMMA) {
		NextSymbol(tarr, size);
		Expression(tarr, size);
		asmq.push(AsmStm{ OPR, 0, OUT });
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
		SYMBOLS s = tarr->sym;
		NextSymbol(tarr, size);
		Expression(tarr, size);
		switch (s)
		{
		case EQ:
			asmq.push(AsmStm{ OPR,0,EQUAL });
			break;
		case NE:
			asmq.push(AsmStm{ OPR,0,NOTEQUAL });
			break;
		case LT:
			asmq.push(AsmStm{ OPR,0,LESSTHAN });
			break;
		case LE:
			asmq.push(AsmStm{ OPR,0,LESSEQUAL });
			break;
		case GT:
			asmq.push(AsmStm{ OPR,0,GREATTHAN });
			break;
		case GE:
			asmq.push(AsmStm{ OPR,0,GREATEQUAL });
			break;
		default:
			throw exception("Unknown Exception.");
			break;
		}
	}
}

void Expression(Token *& tarr, int & size)
{
	if (IsAddSub(GetSymbol(tarr))) {
		SYMBOLS s = tarr->sym;
		NextSymbol(tarr, size);
		asmq.push(AsmStm{ LIT,0,0 });
		Term(tarr, size);
		asmq.push(AsmStm{ OPR, 0, s == ADD ? PLUS : SUBTRACT });
	}
	else
	{
		Term(tarr, size);
	}
	while (IsAddSub(GetSymbol(tarr)))
	{
		SYMBOLS s = tarr->sym;
		NextSymbol(tarr, size);
		Term(tarr, size);
		asmq.push(AsmStm{ OPR, 0, s == ADD ? PLUS : SUBTRACT });
	}
}

void Term(Token *& tarr, int & size)
{
	Factor(tarr, size);
	while (IsMulDiv(GetSymbol(tarr)))
	{
		SYMBOLS s = tarr->sym;
		NextSymbol(tarr, size);
		Factor(tarr, size);
		asmq.push(AsmStm{ OPR, 0, s == MUL ? MULTIPLE : DIVISION });
	}
}

void Factor(Token *& tarr, int & size)
{
	SYMBOLS sym = GetSymbol(tarr);
	if (sym == IDENTIFIER || sym == NUMBER) {
		if (sym == IDENTIFIER) {
			string name((char*)tarr->value);
			Entry e;
			LookUp(name, e);
			switch (e.sym)
			{
			case EXCEPTION:
				throw exception("Undefined Identifier.");
				break;
			case CONST:
				asmq.push(AsmStm{ LIT,0,e.value });
				break;
			case VARIABLE:
				asmq.push(AsmStm{ LOD,level - e.level,e.address });
				break;
			case FUNCTION:
				throw exception("Tring to Calculate Function.");
				break;
			default:
				throw exception("Unknown Exception.");
				break;
			}
		}
		else {
			int i(*(int*)tarr->value);
			asmq.push(AsmStm{ LIT,0,i });
		}
		NextSymbol(tarr, size);
	}
	else Assert(sym, BRACKETSTART) {
		NextSymbol(tarr, size);
		Expression(tarr, size);
		Assert(GetSymbol(tarr), BRACKETEND);
		NextSymbol(tarr, size);
	}
}

template<typename T>
T * Q2List(queue<T>& q, int& size)
{
	T* arr = new T[q.size()];
	size = q.size();
	for (int i = 0; i < size; i++) {
		arr[i] = q.front();
		q.pop();
	}
	return arr;
}

template<typename T>
void QStuff(queue<T>& q, int size)
{
	for (int i = 0; i < size; i++) {
		q.push(T());
	}
}

template<typename T>
void QShift(queue<T>& q, int size)
{
	if (q.size() < size)
		size = q.size();
	for (int i = 0; i < size; i++) {
		q.pop();
	}
}

template<typename T>
void QConcat(queue<T>& q1, queue<T>& q2)
{
	while (!q2.empty())
	{
		q1.push(q2.front());
		q2.pop();
	}
}

#pragma endregion

#pragma region Simulator

void Simulator(AsmStm * aarr, int size)
{
	stack<int> s;
	s.push(0);
	int* memory = new int[1024];
	int ptr = 0;
	int tail = -1;
	memory[0] = 0;
	memory[1] = 0;
	memory[2] = -1;

	for (int i = 0; i < size;) {
		AsmStm a = aarr[i];
		int tpr, v, x, y;
		string ch;
		switch (a.f)
		{
		case LIT:
			s.push(a.a);
			break;
		case LOD:
			tpr = ptr;
			for (int i = 0; i < a.l; i++) {
				tpr = memory[tpr];
			}
			s.push(memory[tpr + a.a]);
			break;
		case STO:
			v = s.top();
			s.pop();
			tpr = ptr;
			for (int i = 0; i < a.l; i++) {
				tpr = memory[tpr];
			}
			memory[tpr + a.a] = v;
			break;
		case CAL:
			if (tail >= 1021)
				throw exception("Out of Memory.");
			tpr = ptr;
			for (int i = 0; i < a.l; i++) {
				tpr = memory[tpr];
			}
			memory[tail + 1] = tpr;
			memory[tail + 2] = 0;
			memory[tail + 3] = i + 1;
			s.push(ptr);
			ptr = tail + 1;
			i = a.a;
			continue;
			break;
		case INT:
			memory[ptr + 1] += a.a;
			tail += a.a;
			if (tail >= 1024)
				throw exception("Out of Memory.");
			break;
		case JMP:
			i = a.a;
			continue;
			break;
		case JPC:
			x = s.top();
			s.pop();
			if (!x) {
				i = a.a;
				continue;
			}
			break;
		case OPR:
			switch (a.a)
			{
			case EXIT:
				printf("\n<Simulator Log> pop block:\n");
				printf("<Simulator Log> dynamic link: %d\n<Simulator Log> return ptr: %d\n<Simulator Log> static link: %d\n<Simulator Log> variable value:", s.top(), memory[ptr + 2], memory[ptr]);
				for (int x = ptr + 3; x < tail; x++) {
					printf("%d\t", memory[x]);
				}
				printf("\n");
				tail -= memory[ptr + 1];
				i = memory[ptr + 2];
				ptr = s.top();
				s.pop();
				if (i < 0)
					goto finish;
				else
					continue;
				break;
			case PLUS:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x + y);
				break;
			case SUBTRACT:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x - y);
				break;
			case MULTIPLE:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x * y);
				break;
			case DIVISION:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				if (y == 0)
					throw exception("Devided by 0.");
				s.push(x / y);
				break;
			case EQUAL:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x == y ? 1 : 0);
				break;
			case NOTEQUAL:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x == y ? 0 : 1);
				break;
			case GREATTHAN:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x > y ? 1 : 0);
				break;
			case GREATEQUAL:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x >= y ? 1 : 0);
				break;
			case LESSTHAN:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x < y ? 1 : 0);
				break;
			case LESSEQUAL:
				y = s.top();
				s.pop();
				x = s.top();
				s.pop();
				s.push(x <= y ? 1 : 0);
				break;
			case IN:
				while (1) {
					printf(">>> ");
					ch = "";
					cin >> ch;
					try {
						x = stoi(ch);
						s.push(x);
						break;
					}
					catch (exception) {
						fprintf(stderr, "<Simulator Warning> Invalid Input.\n");
					}
				}
				break;
			case OUT:
				x = s.top();
				s.pop();
				printf("%d\n", x);
				break;
			default:
				throw exception("Unexpected Operation.");
				break;
			}
			break;
		default:
			throw exception("Unexpected Operation.");
			break;
		}
		if (++i >= size) {
			throw exception("Access Denied.");
		}
	}
finish:
	delete[] memory;
}

#pragma endregion
