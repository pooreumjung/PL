# 📌 Recursive-Descent Interpreter (C++ / Python)

이 저장소는 EBNF 문법 기반의 Recursive-Descent Parser 인터프리터를 구현한 프로젝트입니다.  
두 개의 구현 과제로 구성되어 있으며, 각각 Python과 C++로 작성되었습니다.

---

## 구현과제 1 - 수식 계산 및 변수 출력

### 문법 정의

```ebnf
<program>   → {<statement>}
<statement> → <var> = <expr> ; | print <var> ;
<expr>      → <term> {+ <term> | * <term>}
<term>      → <factor> {- <factor>}
<factor>    → [ - ] ( <number> | <var> | ‘(’<expr>‘)’ )
<number>    → <digit> {<digit>}
<digit>     → 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
<var>       → <alphabet>{<alphabet>}
<alphabet>  → a | b | ... | z
```

### 실행 방법

#### Python

```bash
python3 rd_parser1.py
```

#### C++

```bash
g++ -o rd_parser1 rd_parser1.cpp && ./rd_parser1
```

---

## 구현과제 2 - 변수 선언, 조건문, 반복문 포함

### 문법 정의

```ebnf
<program>     → {<declaration>} {<statement>}
<declaration> → <type> <var> ;
<statement>   → <var> = <aexpr> ;
              | print <aexpr> ;
              | while ( <bexpr> ) do { <statement>* } ;
              | if ( <bexpr> ) { <statement>* } else { <statement>* } ;
<bexpr>       → <var> <relop> <var>
<relop>       → == | != | < | >
<aexpr>       → <term> {( + | - ) <term>}
<term>        → <factor> { * <factor>}
<factor>      → [ - ] ( <number> | <var> | ‘(’<aexpr>‘)’ )
<type>        → integer
<number>      → <digit> {<digit>}
<digit>       → 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
<var>         → <alphabet>{<alphabet>}
<alphabet>    → a | b | ... | z
```

### 실행 방법

#### Python

```bash
python3 rd_parser2.py
```

#### C++

```bash
g++ -o rd_parser2 rd_parser2.cpp && ./rd_parser2
```
