import sys

# Global Declarations
charClass = -1
lexeme = ""
nextChar = ''
token = 0
nextToken = 0
inputBuffer = ""
inputIndex = 0
printfCount = 0

# Character Classes
EOF = -1
LETTER = 0
DIGIT = 1
UNKNOWN = 99

# Token Codes
INT_LIT = 10
IDENT = 11
ASSIGN_OP = 20
ADD_OP = 21
SUB_OP = 22
MULT_OP = 23
LEFT_PAREN = 24
RIGHT_PAREN = 25
SEMICOLON = 26
PRINT_KW = 27
WHILE_KW = 28
DO_KW = 29
IF_KW = 30
ELSE_KW = 31
LBRACE = 32
RBRACE = 33
EQ_OP = 34
NEQ_OP = 35
LT_OP = 36
GT_OP = 37
INT_TYPE = 38

# Variable Tables
varTable = {}
declared = {}
v = []

# Loop context to manage nested loops
loopStack = []
class LoopContext:
    def __init__(self, condStart, condBuf, blockStart, blockBuf):
        self.condStart = condStart
        self.condBuf = condBuf
        self.blockStart = blockStart
        self.blockBuf = blockBuf

# getChar function
def getChar():
    global nextChar, charClass, inputIndex, inputBuffer
    nextChar = '\0' if inputIndex >= len(inputBuffer) else inputBuffer[inputIndex]
    inputIndex += 1
    if nextChar == '\0':
        charClass = EOF
    elif nextChar.islower():
        charClass = LETTER
    elif nextChar.isdigit():
        charClass = DIGIT
    else:
        charClass = UNKNOWN

# addChar function
def addChar():
    global lexeme, nextChar
    if len(lexeme) <= 98:
        lexeme += nextChar

# getNonBlank function
def getNonBlank():
    while nextChar.isspace():
        getChar()

# lex function
def lex():
    global charClass, lexeme, nextChar, nextToken
    lexeme = ""
    getNonBlank()

    if charClass == LETTER:
        addChar()
        getChar()
        while charClass in [LETTER, DIGIT]:
            addChar()
            getChar()
        if lexeme == "print":
            nextToken = PRINT_KW
        elif lexeme == "while":
            nextToken = WHILE_KW
        elif lexeme == "do":
            nextToken = DO_KW
        elif lexeme == "if":
            nextToken = IF_KW
        elif lexeme == "else":
            nextToken = ELSE_KW
        elif lexeme == "integer":
            nextToken = INT_TYPE
        else:
            nextToken = IDENT
    elif charClass == DIGIT:
        addChar()
        getChar()
        while charClass == DIGIT:
            addChar()
            getChar()
        nextToken = INT_LIT
    elif charClass == UNKNOWN:
        if nextChar == '+':
            addChar(); getChar(); nextToken = ADD_OP
        elif nextChar == '-':
            addChar(); getChar(); nextToken = SUB_OP
        elif nextChar == '*':
            addChar(); getChar(); nextToken = MULT_OP
        elif nextChar == '=':
            addChar(); getChar()
            if nextChar == '=':
                addChar(); getChar(); nextToken = EQ_OP
            else:
                nextToken = ASSIGN_OP
        elif nextChar == '!':
            addChar(); getChar()
            if nextChar == '=':
                addChar(); getChar(); nextToken = NEQ_OP
            else:
                syntax_error()
        elif nextChar == '<':
            addChar(); getChar(); nextToken = LT_OP
        elif nextChar == '>':
            addChar(); getChar(); nextToken = GT_OP
        elif nextChar == ';':
            addChar(); getChar(); nextToken = SEMICOLON
        elif nextChar == '(':
            addChar(); getChar(); nextToken = LEFT_PAREN
        elif nextChar == ')':
            addChar(); getChar(); nextToken = RIGHT_PAREN
        elif nextChar == '{':
            addChar(); getChar(); nextToken = LBRACE
        elif nextChar == '}':
            addChar(); getChar(); nextToken = RBRACE
        else:
            syntax_error()
    elif charClass == EOF:
        nextToken = EOF
    else:
        syntax_error()
    return nextToken

# Error handling
def syntax_error():
    raise RuntimeError("Syntax Error!")

# program function => {<declaration>} {<statement>}
def program():
    global v, varTable, declared, inputIndex, lexeme, nextToken, charClass, nextChar
    try:
        getChar()
        lex()
        while nextToken == INT_TYPE:
            declaration()
        while nextToken != EOF:
            statement()
        if v:
            print(" ".join(map(str, v)))
            v.clear()
    except RuntimeError as e:
        print(e)

# declaration function => <type> <var> ;
def declaration():
    type_()
    lex()
    name = var()
    lex()
    if name in declared:
        syntax_error()
    declared[name] = True
    set_variable(name, 0)
    if nextToken != SEMICOLON:
        syntax_error()
    lex()

# type function => integer
def type_():
    if nextToken != INT_TYPE:
        syntax_error()

# statement function
def statement():
    global loopStack
    if nextToken == IDENT:
        varName = var()
        lex()
        if nextToken != ASSIGN_OP:
            syntax_error()
        lex()
        val = aexpr()
        set_variable(varName, val)
        if nextToken != SEMICOLON:
            syntax_error()
        lex()
    elif nextToken == PRINT_KW:
        lex()
        val = aexpr()
        if nextToken != SEMICOLON:
            syntax_error()
        v.append(val)
        lex()
    elif nextToken == WHILE_KW:
        whileStart = inputIndex - len(lexeme) - 1
        whileBuf = inputBuffer
        lex()
        if nextToken != LEFT_PAREN:
            syntax_error()
        condStart = whileStart
        condBuf = inputBuffer
        cond = bexpr()
        lex()
        if nextToken != RIGHT_PAREN:
            syntax_error()
        lex()
        if nextToken != DO_KW:
            syntax_error()
        lex()
        if nextToken != LBRACE:
            syntax_error()
        blockStart = inputIndex
        blockBuf = inputBuffer
        loopStack.append(LoopContext(condStart, condBuf, blockStart, blockBuf))
        if not cond:
            skipBlock()
            loopStack.pop()
        else:
            while loopStack:
                ctx = loopStack[-1]
                resetLexerState(ctx.blockBuf, ctx.blockStart)
                getChar()
                lex()
                parseBlock()
                if nextToken != RBRACE:
                    syntax_error()
                lex()
                resetLexerState(ctx.condBuf, ctx.condStart)
                getChar()
                lex()
                if nextToken != WHILE_KW:
                    syntax_error()
                lex()
                if nextToken != LEFT_PAREN:
                    syntax_error()
                cond = bexpr()
                if not cond:
                    skipBlock()
                    if nextToken != SEMICOLON:
                        syntax_error()
                    loopStack.pop()
                    break
        if nextToken != SEMICOLON:
            syntax_error()
        lex()
    elif nextToken == IF_KW:
        lex()
        if nextToken != LEFT_PAREN:
            syntax_error()
        cond = bexpr()
        if cond:
            lex()
            if nextToken != RIGHT_PAREN:
                syntax_error()
            lex()
            if nextToken != LBRACE:
                syntax_error()
            lex()
            parseBlock()
            if nextToken != RBRACE:
                syntax_error()
            lex()
            skipBlock()
        else:
            skipBlock()
            if nextToken != ELSE_KW:
                syntax_error()
            lex()
            if nextToken != LBRACE:
                syntax_error()
            lex()
            parseBlock()
            if nextToken != RBRACE:
                syntax_error()
            lex()
        if nextToken != SEMICOLON:
            syntax_error()
        lex()
    else:
        syntax_error()

# aexpr function => <term> {( + | - ) <term>}
def aexpr():
    val = term()
    while nextToken in (ADD_OP, SUB_OP):
        op = nextToken
        lex()
        rhs = term()
        val = val + rhs if op == ADD_OP else val - rhs
    return val

# bexpr function => <var> <relop> <var>
def bexpr():
    lex()
    left = var()
    lex()
    op = relop()
    lex()
    right = var()
    l = get_variable_value(left)
    r = get_variable_value(right)
    return eval_condition(l, op, r)

# relop function =>  == | != | < | >
def relop():
    if nextToken in (EQ_OP, NEQ_OP, LT_OP, GT_OP):
        return lexeme
    syntax_error()

# term function => <factor> { * <factor>}
def term():
    val = factor()
    while nextToken == MULT_OP:
        lex()
        val *= factor()
    return val

#  factor function => [ - ] ( <number> | <var> | ‘(’<aexpr>‘)’ )
def factor():
    sign = 1
    if nextToken == SUB_OP:
        sign = -1
        lex()
    if nextToken == INT_LIT:
        val = int(lexeme)
        lex()
    elif nextToken == IDENT:
        name = var()
        lex()
        val = get_variable_value(name)
    elif nextToken == LEFT_PAREN:
        lex()
        val = aexpr()
        if nextToken != RIGHT_PAREN:
            syntax_error()
        lex()
    else:
        syntax_error()
    return sign * val

# parseBlock function, 반복 블록 실행
def parseBlock():
    while nextToken != RBRACE and nextToken != EOF:
        statement()
    if nextToken != RBRACE:
        syntax_error()

# skipBlock function, 반복 블록 스킵
def skipBlock():
    braceStack = []
    while nextToken != LBRACE and nextToken != EOF:
        lex()
    if nextToken == LBRACE:
        braceStack.append('{')
        lex()
    else:
        syntax_error()
    while braceStack and nextToken != EOF:
        if nextToken == LBRACE:
            braceStack.append('{')
        elif nextToken == RBRACE:
            braceStack.pop()
        elif nextToken in (IF_KW, WHILE_KW):
            lex()
            if nextToken != LEFT_PAREN:
                syntax_error()
            while nextToken != RIGHT_PAREN and nextToken != EOF:
                lex()
            lex()
            if nextToken == DO_KW:
                lex()
            continue
        lex()
    if braceStack:
        syntax_error()

# resetLexerState function, inputBuffer와 inputIndex를 초기화
def resetLexerState(buf, idx):
    global inputBuffer, inputIndex, lexeme, nextToken, charClass, nextChar
    inputBuffer = buf
    inputIndex = idx
    lexeme = ""
    nextToken = 0
    charClass = -1
    nextChar = '\0'

# set_variable function
def set_variable(name, value):
    if name not in declared:
        syntax_error()
    varTable[name] = value

# get_variable_value function
def get_variable_value(name):
    if name not in declared:
        syntax_error()
    return varTable[name]

# var function
def var():
    if nextToken != IDENT or len(lexeme) > 10:
        syntax_error()
    return lexeme

# eval_condition function
def eval_condition(l, op, r):
    if op == "==":
        return l == r
    elif op == "!=":
        return l != r
    elif op == "<":
        return l < r
    elif op == ">":
        return l > r
    syntax_error()

# main
if __name__ == "__main__":
    while True:
        try:
            line = input(">> ")
            if not line:
                break

            inputBuffer = line
            inputIndex = 0
            lexeme = ""
            nextToken = 0
            charClass = -1
            nextChar = '\0'
            printfCount = 0

            varTable.clear()
            declared.clear()
            v.clear()
            program()
        except EOFError:
            break
