#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <map>
#include <vector>
#include <stack>
using namespace std;

// Global Declarations
int charClass;
string lexeme;
char nextChar;
int token;
int nextToken;
string inputBuffer;
int inputIndex = 0;
int printfCount = 0;
bool executing = true; // 실행 제어 플래그 추가

// Character Classes
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

// Token Codes
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

// Variable Tables
map<string, int> varTable;
map<string, bool> declared;
vector<int> v;

// Function Prototypes
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
bool isAllowedTokenInBlock(int token);

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
        executing = true;
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

        while (true)
        {
            declaration();
            if (nextToken != INT_TYPE)
                break;
        }

        while (nextToken != EOF)
            statement();

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

void declaration()
{
    type();
    lex();
    string name;
    var(name);

    lex();
    if (declared.count(name))
        syntax_error();

    declared[name] = true;
    set_variable(name, 0);

    if (nextToken != SEMICOLON)
        syntax_error();
    lex();
}

void statement()
{
    if (nextToken == IDENT)
    {
        string varName;
        var(varName);

        lex();
        if (nextToken != ASSIGN_OP)
            syntax_error();
        lex();
        int val = aexpr();

        if (executing)
            set_variable(varName, val);

        if (nextToken != SEMICOLON)
            syntax_error();
        lex();
    }
    else if (nextToken == PRINT_KW)
    {
        lex();
        int val = aexpr();
        if (nextToken != SEMICOLON)
            syntax_error();

        if (executing)
            v.push_back(val);

        lex();
    }
    else if (nextToken == WHILE_KW)
    {
        lex();
        if (nextToken != LEFT_PAREN)
            syntax_error();

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
        lex();

        int loopStart = inputIndex;
        string loopBuf = inputBuffer;

        while (cond)
        {
            int savedIndex = inputIndex;
            string savedBuf = inputBuffer;

            bool prevExecuting = executing;
            executing = prevExecuting && true;
            parseBlock();
            executing = prevExecuting;

            if (nextToken != RBRACE)
                syntax_error();
            lex();

            inputIndex = loopStart;
            inputBuffer = loopBuf;
            lexeme.clear();
            nextToken = 0;
            charClass = -1;
            nextChar = '\0';
            getChar();
            lex(); // while
            lex(); // (
            cond = bexpr();
            lex(); // )
            lex(); // do
            lex(); // {
        }

        if (nextToken != RBRACE)
            syntax_error();
        lex();
        if (nextToken != SEMICOLON)
            syntax_error();
        lex();
    }
    else if (nextToken == IF_KW)
    {
        lex();
        if (nextToken != LEFT_PAREN)
            syntax_error();
        bool cond = bexpr();
        lex();
        if (nextToken != RIGHT_PAREN)
            syntax_error();
        lex();
        if (nextToken != LBRACE)
            syntax_error();
        lex();

        bool prevExecuting = executing;
        executing = prevExecuting && cond;
        parseBlock();
        executing = prevExecuting;

        if (nextToken != RBRACE)
            syntax_error();
        lex();

        if (nextToken != ELSE_KW)
            syntax_error();
        lex();
        if (nextToken != LBRACE)
            syntax_error();
        lex();

        executing = prevExecuting && !cond;
        parseBlock();
        executing = prevExecuting;

        if (nextToken != RBRACE)
            syntax_error();
        lex();
        if (nextToken != SEMICOLON)
            syntax_error();
        lex();
    }
    else
    {
        syntax_error();
    }
}

// type function => integer
void type()
{
    if (nextToken != INT_TYPE)
        syntax_error();
}

// aexpr function => <term> {( + | - ) <term>}
int aexpr()
{
    int val = term();

    while (nextToken == ADD_OP || nextToken == SUB_OP)
    {
        int op = nextToken;
        lex();
        int rhs = term();
        val = (op == ADD_OP) ? val + rhs : val - rhs;
    }

    return val;
}

// bexpr function => <var> <relop> <var>
bool bexpr()
{
    lex();
    string left;
    var(left);

    lex();
    string op = relop();

    lex();
    string right;
    var(right);

    int l = get_variable_value(left);
    int r = get_variable_value(right);
    return eval_condition(l, op, r);
}

// relop function =>  == | != | < | >
string relop()
{
    if (nextToken == EQ_OP || nextToken == NEQ_OP || nextToken == LT_OP || nextToken == GT_OP)
    {
        string op = lexeme;
        return op;
    }
    syntax_error();
}

// term function => <factor> { * <factor>}
int term()
{
    int val = factor();

    while (nextToken == MULT_OP)
    {
        lex();
        val *= factor();
    }

    return val;
}

// factor function => [ - ] ( <number> | <var> | ‘(’<aexpr>‘)’ )
int factor()
{
    int sign = 1;
    if (nextToken == SUB_OP)
    {
        sign = -1;
        lex();
    }

    int val;

    if (nextToken == INT_LIT)
    {
        val = stoi(lexeme);
        lex();
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
        lex();
        val = aexpr();
        if (nextToken != RIGHT_PAREN)
            syntax_error();
        lex();
    }
    else
        syntax_error();

    return sign * val;
}

// parseBlock function, 반복 블록 실행
void parseBlock()
{
    while (nextToken != RBRACE && nextToken != EOF)
        statement();

    if (nextToken != RBRACE)
        syntax_error();
}

// skipBlock function, 반복 블록 스킵
void skipBlock()
{
    stack<char> braceStack;

    while (nextToken != LBRACE && nextToken != EOF)
        lex();

    if (nextToken == LBRACE)
    {
        braceStack.push('{');
        lex();
    }
    else
        syntax_error();

    while (!braceStack.empty() && nextToken != EOF)
    {
        if (nextToken == LBRACE)
        {
            braceStack.push('{');
        }
        else if (nextToken == RBRACE)
        {
            braceStack.pop();
        }
        else if (nextToken == IF_KW || nextToken == WHILE_KW)
        {
            // 조건 건너뛰기
            lex();
            if (nextToken != LEFT_PAREN)
                syntax_error();

            while (nextToken != RIGHT_PAREN && nextToken != EOF)
                lex();

            lex(); // consume ')'

            if (nextToken == DO_KW)
                lex();

            continue;
        }
        else if (!isAllowedTokenInBlock(nextToken))
        {
            syntax_error();
        }

        lex();
    }

    if (!braceStack.empty())
        syntax_error();
}

// resetLexerState function, inputBuffer와 inputIndex를 초기화
void resetLexerState(const string &buf, int idx)
{
    inputBuffer = buf;
    inputIndex = idx;
    lexeme.clear();
    nextToken = 0;
    charClass = -1;
    nextChar = '\0';
}

// syntax_error function
[[noreturn]] void syntax_error()
{
    throw runtime_error("Syntax Error!");
}

// lex function
int lex()
{
    lexeme.clear();
    getNonBlank();

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

// getChar function
void getChar()
{
    nextChar = (inputIndex >= inputBuffer.length()) ? '\0' : inputBuffer[inputIndex++];
    if (nextChar == '\0')
        charClass = EOF;
    else if (isalpha(nextChar) && islower(nextChar))
        charClass = LETTER;
    else if (isdigit(nextChar))
        charClass = DIGIT;
    else
        charClass = UNKNOWN;
}

// addChar function
void addChar()
{
    if (lexeme.length() <= 98)
        lexeme += nextChar;
}

// getNonBlank function
void getNonBlank()
{
    while (isspace(nextChar))
        getChar();
}

// eval_condition function
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

// set_variable function
void set_variable(const string &name, int value)
{
    if (!declared.count(name))
        syntax_error();
    varTable[name] = value;
}

// get_variable_value function
int get_variable_value(const string &name)
{
    if (!declared.count(name))
        syntax_error();
    return varTable[name];
}

// var function
void var(string &name)
{
    if (nextToken != IDENT || lexeme.length() > 10)
        syntax_error();
    name = lexeme;
}
bool isAllowedTokenInBlock(int token)
{
    switch (token)
    {
    case IDENT:
    case INT_LIT:
    case PRINT_KW:
    case WHILE_KW:
    case DO_KW:
    case IF_KW:
    case ELSE_KW:
    case ASSIGN_OP:
    case ADD_OP:
    case SUB_OP:
    case MULT_OP:
    case EQ_OP:
    case NEQ_OP:
    case LT_OP:
    case GT_OP:
    case LEFT_PAREN:
    case RIGHT_PAREN:
    case LBRACE:
    case RBRACE:
    case SEMICOLON:
        return true;
    default:
        return false;
    }
}
