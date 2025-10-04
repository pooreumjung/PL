#  Recursive-Descent Interpreter (C++ / Python)

숭실대학교 컴퓨터학부 / 2025년 프로그래밍언어 수업 김철홍 교수님의 구현 과제입니다.
본 과제에서는 EBNF 문법을 기반으로 한 Recursive-Descent Parser 인터프리터를 구현하였습니다.
총 두 개의 구현 과제로 구성되어 있으며, 각각 Python과 C++로 작성되었습니다.

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
