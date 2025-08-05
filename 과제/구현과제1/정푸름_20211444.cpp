// 스택 기반 반복 구조를 적용한 RD 파서 (중첩 if/while 처리 포함, 개선판)

#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <map>
#include <vector>
#include <stack>
using namespace std;

int charClass;
string lexeme;
char nextChar;
int token;
int nextToken;
string inputBuffer;
int inputIndex = 0;
int printfCount = 0;

#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define LEFT_PAREN 24
#define RIGHT_PAREN 25
#define SEMICOLON 26
#define PRINT_KW 27
#define WHILE_KW 28
#define DO_KW 29
#define IF_KW 30
#define ELSE_KW 31
#define LBRACE 32
#define RBRACE 33
#define EQ_OP 34
#define NEQ_OP 35
#define LT_OP 36
#define GT_OP 37
#define INT_TYPE 38

map<string, int> varTable;
map<string, bool> declared;
vector<int> v;

struct LoopContext
{
    int condStart;
    string condBuf;
    int blockStart;
    string blockBuf;

    LoopContext(int cs, const string &cb, int bs, const string &bb)
        : condStart(cs), condBuf(cb), blockStart(bs), blockBuf(bb) {}
};

stack<LoopContext> loopStack;

void getChar();
void addChar();
void getNonBlank();
int lex();
[[noreturn]] void syntax_error();
void program();
void declaration();
void statement();
void type();
bool bexpr();
string relop();
int aexpr();
int term();
int factor();
void var(string &);
int get_variable_value(const string &);
void set_variable(const string &, int);
void resetLexerState(const string &, int);
void parseBlock();
bool eval_condition(int, const string &, int);
void skipBlock();
int main()
{
    string line;
    while (true)
    {
        cout << ">> ";
        getline(cin, line);
        if (line.empty())
            break;

        inputBuffer = line;
        inputIndex = 0;
        lexeme.clear();
        nextToken = 0;
        charClass = -1;
        nextChar = '\0';
        printfCount = 0;

        varTable.clear();
        declared.clear();
        v.clear();
        program();
    }
    return 0;
}

void program()
{
    try
    {
        getChar();
        lex();
        // declaration 반복
        while (true)
        {
            declaration();
            cout << "현재 nextToken1: " << nextToken << endl;
            if (nextToken != INT_TYPE)
                break;
        }

        // statement 반복 읽기
        while (nextToken != EOF)
        {
            statement();
            cout << "현재 nextToken2: " << nextToken << endl;
        }

        // 출력하기
        if (!v.empty())
        {
            for (int val : v)
                cout << val << " ";
            cout << endl;
            v.clear();
        }
    }
    catch (const runtime_error &e)
    {
        cout << e.what() << endl;
    }
}
void type()
{
    if (nextToken != INT_TYPE)
        syntax_error();
}
void declaration()
{
    // type 읽기
    type();
    lex();
    string name;
    var(name);

    // var 읽기
    lex();
    if (declared.count(name))
        syntax_error(); // 중복 선언 방지

    declared[name] = true;
    set_variable(name, 0); // 초기값 0으로 등록

    // 세미콜론 읽기
    if (nextToken != SEMICOLON)
        syntax_error();
    lex();
}
void statement()
{

    if (nextToken == IDENT)
    {
        cout << "[IDENT] 현재 nextToken: " << nextToken << endl;
        // <var> = <aexpr> ;
        string varName;
        var(varName);

        // = 읽기
        lex();
        if (nextToken != ASSIGN_OP)
            syntax_error();
        lex();             // consume '='
        int val = aexpr(); // <aexpr>
        set_variable(varName, val);

        if (nextToken != SEMICOLON)
            syntax_error();
        lex(); // consume ';'
        cout << "[IDENT] 변수 " << varName << "에 값 " << val << " 할당" << endl;
    }
    else if (nextToken == PRINT_KW)
    {
        cout << "[print] 현재 nextToken: " << nextToken << endl;
        // print <aexpr> ;
        lex();             // consume 'print'
        int val = aexpr(); // <aexpr>
        if (nextToken != SEMICOLON)
            syntax_error();
        v.push_back(val);
        lex(); // consume ';'
    }
    if (nextToken == WHILE_KW)
    {
        cout << "[while] 현재 nextToken: " << nextToken << endl;
        int whileStart = inputIndex - lexeme.length() - 1;
        string whileBuf = inputBuffer;

        lex();
        if (nextToken != LEFT_PAREN)
            syntax_error();

        int condStart = whileStart;
        string condBuf = inputBuffer;
        bool cond = bexpr();

        lex();
        if (nextToken != RIGHT_PAREN)
            syntax_error();

        lex();
        if (nextToken != DO_KW)
            syntax_error();

        lex();
        if (nextToken != LBRACE)
            syntax_error();

        int blockStart = inputIndex;
        string blockBuf = inputBuffer;

        loopStack.push(LoopContext(condStart, condBuf, blockStart, blockBuf));

        while (!loopStack.empty())
        {
            LoopContext ctx = loopStack.top();

            resetLexerState(ctx.blockBuf, ctx.blockStart);
            getChar();
            lex();

            parseBlock();
            if (nextToken != RBRACE)
            {
                cout << "[Error] '}' expected after block, got token: " << nextToken << endl;
                syntax_error();
            }
            lex();

            resetLexerState(ctx.condBuf, ctx.condStart);
            getChar();
            lex();
            if (nextToken != WHILE_KW)
                syntax_error();

            lex();
            if (nextToken != LEFT_PAREN)
                syntax_error();

            cond = bexpr();

            if (!cond)
            {
                lex();
                if (nextToken != RIGHT_PAREN)
                    syntax_error();
                lex();
                if (nextToken != DO_KW)
                    syntax_error();
                lex();
                if (nextToken != LBRACE)
                    syntax_error();
                lex();
                skipBlock();
                if (nextToken != SEMICOLON)
                    syntax_error();
                lex();
                loopStack.pop();
                break;
            }
        }
    }
    else if (nextToken == IF_KW)
    {
        cout << "[if] 현재 nextToken: " << nextToken << endl;
        // ( 읽기
        lex();
        if (nextToken != LEFT_PAREN)
            syntax_error();

        // 조건 읽기
        bool cond = bexpr();

        // ) 읽기
        lex();
        if (nextToken != RIGHT_PAREN)
            syntax_error();

        // { 읽기
        lex();
        if (nextToken != LBRACE)
            syntax_error();

        lex(); // consume '{'
        if (cond)
        {
            parseBlock(); // if 블록 실행
            if (nextToken != RBRACE)
            {
                syntax_error();
            }
            lex(); // consume '}'
        }
        else
        {
            skipBlock(); // if 블록 무시
        }

        if (nextToken != ELSE_KW)
            syntax_error();
        lex(); // consume 'else'

        if (nextToken != LBRACE)
            syntax_error();
        lex(); // consume '{'

        if (cond)
        {
            skipBlock(); // else 블록 무시
        }
        else
        {
            parseBlock(); // else 블록 실행
            if (nextToken != RBRACE)
            {
                syntax_error();
            }
            lex(); // consume '}'
        }

        if (nextToken != SEMICOLON)
            syntax_error();
        lex(); // consume ';'
    }
    else
    {
        syntax_error();
    }
}
bool bexpr()
{
    lex();
    string left;
    var(left); // <var>

    // 연산자 읽기
    lex();
    string op = relop(); // <relop>

    // 우측 피연산자 읽기
    lex();
    string right;
    var(right); // <var>

    int l = get_variable_value(left);
    int r = get_variable_value(right);
    return eval_condition(l, op, r);
}
string relop()
{
    if (nextToken == EQ_OP || nextToken == NEQ_OP || nextToken == LT_OP || nextToken == GT_OP)
    {
        string op = lexeme;
        return op;
    }
    syntax_error();
}
void parseBlock()
{
    while (nextToken != RBRACE && nextToken != EOF)
    {
        statement(); // 각 statement 안에서 ; 까지 consume
    }
}
void skipBlock()
{
    int brace = 1;

    while (brace > 0 && nextToken != EOF)
    {
        if (nextToken == LBRACE)
        {
            brace++;
        }
        else if (nextToken == RBRACE)
        {
            brace--;
        }
        else if (nextToken == IF_KW)
        {
            lex(); // consume 'if'
            if (nextToken != LEFT_PAREN)
                syntax_error();
            while (nextToken != RIGHT_PAREN)
                lex(); // skip condition
            lex();     // consume ')'
            if (nextToken != LBRACE)
                syntax_error();
            lex();       // consume '{'
            skipBlock(); // skip if block
            if (nextToken == ELSE_KW)
            {
                lex();
                if (nextToken != LBRACE)
                    syntax_error();
                lex();
                skipBlock();
            }
            continue;
        }
        else if (nextToken == WHILE_KW)
        {
            lex(); // consume 'while'
            if (nextToken != LEFT_PAREN)
                syntax_error();
            while (nextToken != RIGHT_PAREN)
                lex(); // skip condition
            lex();     // consume ')'
            if (nextToken != DO_KW)
                syntax_error();
            lex(); // consume 'do'
            if (nextToken != LBRACE)
                syntax_error();
            lex();       // consume '{'
            skipBlock(); // skip while block
            continue;
        }

        lex(); // 일반 토큰 consume
    }

    if (brace != 0)
        syntax_error();
}
int aexpr()
{
    int val = term(); // 첫 term 처리

    while (nextToken == ADD_OP || nextToken == SUB_OP)
    {
        int op = nextToken;
        lex();            // consume '+' or '-'
        int rhs = term(); // 다음 term
        val = (op == ADD_OP) ? val + rhs : val - rhs;
    }

    return val;
}
int term()
{
    int val = factor(); // 첫 factor

    while (nextToken == MULT_OP)
    {
        lex();           // consume '*'
        val *= factor(); // 다음 factor
    }

    return val;
}
int factor()
{
    int sign = 1;
    if (nextToken == SUB_OP)
    {
        sign = -1;
        lex(); // consume '-'
    }

    int val;

    if (nextToken == INT_LIT)
    {
        val = stoi(lexeme);
        lex(); // consume number
    }
    else if (nextToken == IDENT)
    {
        string name;
        var(name);
        lex();
        val = get_variable_value(name);
    }
    else if (nextToken == LEFT_PAREN)
    {
        lex();         // consume '('
        val = aexpr(); // 내부 aexpr 처리
        if (nextToken != RIGHT_PAREN)
            syntax_error();
        lex(); // consume ')'
    }
    else
        syntax_error();

    return sign * val;
}
[[noreturn]] void syntax_error()
{
    throw runtime_error("Syntax Error!");
}
void resetLexerState(const string &buf, int idx)
{
    inputBuffer = buf;
    inputIndex = idx;
    lexeme.clear();
    nextToken = 0;
    charClass = -1;
    nextChar = '\0';
}

int lex()
{
    lexeme.clear();
    getNonBlank(); // 공백 무시

    switch (charClass)
    {
    case LETTER:
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT)
        {
            addChar();
            getChar();
        }

        // 키워드인지 확인
        if (lexeme == "print")
            return nextToken = PRINT_KW;
        if (lexeme == "while")
            return nextToken = WHILE_KW;
        if (lexeme == "do")
            return nextToken = DO_KW;
        if (lexeme == "if")
            return nextToken = IF_KW;
        if (lexeme == "else")
            return nextToken = ELSE_KW;
        if (lexeme == "integer")
            return nextToken = INT_TYPE;

        return nextToken = IDENT;

    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT)
        {
            addChar();
            getChar();
        }
        return nextToken = INT_LIT;

    case UNKNOWN:
        switch (nextChar)
        {
        case '+':
            addChar();
            getChar();
            return nextToken = ADD_OP;
        case '-':
            addChar();
            getChar();
            return nextToken = SUB_OP;
        case '*':
            addChar();
            getChar();
            return nextToken = MULT_OP;
        case '=':
            addChar();
            getChar();
            if (nextChar == '=')
            {
                addChar();
                getChar();
                return nextToken = EQ_OP;
            }
            return nextToken = ASSIGN_OP;
        case '!':
            addChar();
            getChar();
            if (nextChar == '=')
            {
                addChar();
                getChar();
                return nextToken = NEQ_OP;
            }
            throw runtime_error("Syntax Error!");
        case '<':
            addChar();
            getChar();
            return nextToken = LT_OP;
        case '>':
            addChar();
            getChar();
            return nextToken = GT_OP;
        case ';':
            addChar();
            getChar();
            return nextToken = SEMICOLON;
        case '(':
            addChar();
            getChar();
            return nextToken = LEFT_PAREN;
        case ')':
            addChar();
            getChar();
            return nextToken = RIGHT_PAREN;
        case '{':
            addChar();
            getChar();
            return nextToken = LBRACE;
        case '}':
            addChar();
            getChar();
            return nextToken = RBRACE;
        default:
            throw runtime_error("Syntax Error!");
        }

    case EOF:
        return nextToken = EOF;

    default:
        throw runtime_error("Syntax Error!");
    }
}

void getChar()
{
    nextChar = (inputIndex >= inputBuffer.length()) ? '\0' : inputBuffer[inputIndex++];
    if (nextChar == '\0')
        charClass = EOF;
    else if (isalpha(nextChar))
        charClass = LETTER;
    else if (isdigit(nextChar))
        charClass = DIGIT;
    else
        charClass = UNKNOWN;
}

void addChar()
{
    if (lexeme.length() <= 98)
        lexeme += nextChar;
}

void getNonBlank()
{
    while (isspace(nextChar))
        getChar();
}

bool eval_condition(int l, const string &op, int r)
{
    if (op == "==")
        return l == r;
    if (op == "!=")
        return l != r;
    if (op == "<")
        return l < r;
    if (op == ">")
        return l > r;
    syntax_error();
}

void set_variable(const string &name, int value)
{
    if (!declared.count(name))
        syntax_error();
    varTable[name] = value;
}

int get_variable_value(const string &name)
{
    if (!declared.count(name))
        syntax_error();
    return varTable[name];
}

void var(string &name)
{
    if (nextToken != IDENT || lexeme.length() > 10)
        syntax_error();
    name = lexeme;
}
