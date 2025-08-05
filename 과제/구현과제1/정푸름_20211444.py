# Global Declarations
charClass = None
lexeme = ""
nextChar = ''
token = None
nextToken = None
inputBuffer = ""
inputIndex = 0
printfCount = 0

# Character Classes
LETTER = 0
DIGIT = 1
UNKNOWN = 99

# Token codes
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

# Variable table and output values
varTable = {}
v = []

# lexeme에 문자 추가
def addChar():
    global lexeme, nextChar
    if len(lexeme) <= 98:
        lexeme += nextChar

# 다음 문자를 읽고 문자 클래스 판별
def getChar():
    global nextChar, charClass, inputIndex, inputBuffer
    if inputIndex >= len(inputBuffer):
        nextChar = '\0'
    else:
        nextChar = inputBuffer[inputIndex]
        inputIndex += 1

    if nextChar == '\0':
        charClass = -1
    elif nextChar.isalpha() and nextChar.islower():
        charClass = LETTER
    elif nextChar.isdigit():
        charClass = DIGIT
    elif ord(nextChar) >= 128:
        syntax_error()
    else:
        charClass = UNKNOWN

# 공백 문자 건너뛰기
def getNonBlank():
    while nextChar.isspace():
        getChar()

# 어휘 분석기: 입력을 토큰으로 분리
def lex():
    global lexeme, nextToken
    lexeme = ""
    getNonBlank()

    if charClass == LETTER:
        addChar()
        getChar()
        while charClass == LETTER:
            addChar()
            getChar()
        if lexeme == "print":
            nextToken = PRINT_KW
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
        lookup(nextChar)
        getChar()
    elif charClass == -1:
        nextToken = -1
        lexeme = "EOF"
    else:
        nextToken = -1
    return nextToken

# 연산자 및 괄호 토큰 판별
def lookup(ch):
    global nextToken
    if ch == '+': addChar(); nextToken = ADD_OP
    elif ch == '-': addChar(); nextToken = SUB_OP
    elif ch == '*': addChar(); nextToken = MULT_OP
    elif ch == '=': addChar(); nextToken = ASSIGN_OP
    elif ch == '(': addChar(); nextToken = LEFT_PAREN
    elif ch == ')': addChar(); nextToken = RIGHT_PAREN
    elif ch == ';': addChar(); nextToken = SEMICOLON
    else: syntax_error()

# 문법 오류 발생 시 예외 던지기
def syntax_error():
    raise Exception("Syntax Error!")

# 변수 테이블에 값 저장
def set_variable(varName, value):
    varTable[varName] = value

# 변수 테이블에서 값 가져오기
def get_variable_value(varName):
    return varTable.get(varName, 0)

# 프로그램 전체 파싱
def program():
    global nextToken, printfCount
    if nextToken == -1:
        print("Syntax Error!")
        return
    isSatisfied = True
    while nextToken != -1:
        try:
            statement()
            if nextToken == SEMICOLON:
                lex()
        except Exception as e:
            print(e)
            nextToken = -1
            isSatisfied = False
            varTable.clear()
            printfCount = 0
    # isSatisfied가 true(문법적 오류 x)일 때만 결과 출력
    if isSatisfied and v:
        print(" ".join(map(str, v)), end=" ")
    v.clear()

# 대입문 또는 print문 처리
def statement():
    global printfCount
    if nextToken == IDENT:
        varName = ""
        varName = var()
        lex()
        if nextToken != ASSIGN_OP:
            syntax_error()
        lex()
        value = expr()
        set_variable(varName, value)
        if nextToken != SEMICOLON:
            syntax_error()
    elif nextToken == PRINT_KW:
        lex()
        if nextToken != IDENT:
            syntax_error()
        varName = var()
        lex()
        if nextToken != SEMICOLON:
            syntax_error()
        v.append(get_variable_value(varName))
        printfCount += 1
    else:
        syntax_error()

# <expr> -> <term> {+ <term> | * <term>}
def expr():
    value = term()
    while nextToken in (ADD_OP, MULT_OP):
        op = nextToken
        lex()
        rhs = term()
        if op == ADD_OP:
            value += rhs
        else:
            value *= rhs
    return value

# <term> -> <factor> {- <factor>}
def term():
    value = factor()
    while nextToken == SUB_OP:
        lex()
        rhs = factor()
        value -= rhs
    return value

# <factor> → [ - ] ( <number> | <var> | ‘(’<expr>‘)’ )
def factor():
    signCheck = 1
    if nextToken == SUB_OP:
        signCheck = -1
        lex()

    value = 0
    if nextToken == INT_LIT:
        value = number()
        lex()
    elif nextToken == IDENT:
        varName = var()
        value = get_variable_value(varName)
        lex()
    elif nextToken == LEFT_PAREN:
        lex()
        value = expr()
        if nextToken != RIGHT_PAREN:
            syntax_error()
        lex()
    else:
        syntax_error()
    return signCheck * value

# 숫자 리터럴 파싱
def number():
    if len(lexeme) > 10:
        syntax_error()
    if not lexeme.isdigit():
        syntax_error()
    return int(lexeme)

# 변수 이름 유효성 검사 및 반환
def var():
    if nextToken != IDENT:
        syntax_error()
    if len(lexeme) > 10:
        syntax_error()
    if not all(ch.islower() for ch in lexeme):
        syntax_error()
    return lexeme

# 메인 루프: 한 줄 입력 받아 파싱
def main():
    global inputBuffer, inputIndex, nextToken
    while True:
        try:
            print(">> ", end="")
            line = input()
            if not line:
                break
            inputBuffer = line
            inputIndex = 0
            getChar()
            nextToken = 0
            lex()
            program()
            varTable.clear()
            if printfCount > 0:
                print()
        except EOFError:
            break

if __name__ == '__main__':
    main()
