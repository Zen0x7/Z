---
title: Grammar
description: The complete surface grammar accepted by zc.
navigation:
  title: Grammar
---

# Grammar

This grammar defines the source surface accepted by `zc`. Semantic rules add constraints that the grammar alone cannot express, such as exact operand types, array extents and generic capacity.

```text
program        = statement* ;

statement      = importDeclaration
               | declaration
               | expression ";"
               | labelStatement
               | jumpStatement
               | breakStatement
               | continueStatement
               | structDeclaration
               | interfaceDeclaration
               | enumDeclaration
               | traitDeclaration
               | ifStatement
               | whileStatement
               | doWhileStatement
               | switchStatement
               | forStatement
               | foreachStatement
               | repeatStatement
               | functionDeclaration
               | externDeclaration
               | returnStatement
               | block ;

block          = "{" statement* "}" ;

importDeclaration = "import" "{" identifier ( "," identifier )* "}"
                    "from" stringLiteral ";" ;

builtinType    = "int32" | "uint32" | "int64" | "uint64" | "float" | "double"
               | "boolean" | "string" | "arguments" ;
typeName       = builtinType | identifier typeArguments? ;
typeArguments  = "<" typeName ( "," typeName )* ">" ;
typeParameters = "<" identifier genericConstraint? ( "," identifier genericConstraint? )* ">" ;
genericConstraint = "implements" "{" identifier ( "," identifier )* "}" ;
arrayModifier  = "array" ( "[" integerLiteral "]" )* ;
viewModifier   = "view" "[" "]" typeName? ;
type           = typeName ( "[" integerLiteral "]" )? ;
arrayInitializer = "{" ( expression | arrayInitializer )
                     ( "," ( expression | arrayInitializer ) )* "}" ;
declaration    = "const"? ( arrayModifier typeName | viewModifier | type ) identifier
                 ( "=" ( expression | arrayInitializer ) )? ";" ;

structDeclaration = "struct" identifier typeParameters? inheritance? implements?
                    "{" structMember* "}" ;
inheritance    = ":" identifier ;
implements     = "implements" identifier ( "," identifier )* ;
interfaceDeclaration = "interface" identifier "{" interfaceMember* "}" ;
interfaceMember = declaration | functionSignature ;
traitUses      = "uses" identifier ( "," identifier )* ";" ;
structMember   = traitUses
               | declaration
               | constructorDeclaration
               | functionDeclaration
               | staticFunctionDeclaration ;
constructorDeclaration = "constructor" "(" parameters ")" block ;
staticFunctionDeclaration = "static" functionDeclaration ;

enumDeclaration = "enum" identifier "{" identifier ( "," identifier )* "}" ;
traitDeclaration = "trait" identifier "requires" identifier ( "," identifier )*
                   "{" functionDeclaration* "}" ;
functionSignature = "function" identifier "(" parameters ")" ( ":" type )? ";" ;

functionDeclaration = "function" identifier typeParameters? "(" parameters ")"
                      ( ":" type )? block ;
externDeclaration = "extern" functionSignature "from" stringLiteral ";" ;
parameterMode  = "clone" | "reference" ;
parameter      = "const"? parameterMode ( "array" | type ) identifier ;
parameters     = ( parameter ( "," parameter )* )? ;
returnStatement = "return" expression? ";" ;
labelStatement  = identifier ":" statement ;
jumpStatement   = "jump" identifier ";" ;
breakStatement  = "break" identifier? ";" ;
continueStatement = "continue" identifier? ";" ;
doWhileStatement = "do" block "while" "(" expression ")" ";" ;
switchStatement = "switch" "(" expression ")" "{" caseClause* defaultClause? "}" ;
caseClause      = "case" expression ":" statement* ;
defaultClause   = "default" ":" statement* ;

ifStatement    = "if" "(" expression ")" block
                 ( "else" ( block | ifStatement ) )? ;
whileStatement = "while" "(" expression ")" block ;
forStatement   = "for" "(" ( declaration | expression? ) ";"
                 expression ";" expression ")" block ;
foreachStatement = "foreach" "(" identifier ( "," identifier )* "in"
                    ( identifier | rangeExpression ) ")" block ;
rangeExpression = expression ".." expression ;
repeatStatement = "repeat" "(" expression ")" block ;

expression     = literal
               | identifier typeArguments?
               | "(" expression ")"
               | "!" expression
               | "-" expression
               | expression "&&" expression
               | expression "||" expression
               | expression ( "+" | "-" | "*" | "/" | "%"
                              | "&" | "|" | "^" | "<<" | ">>" ) expression
               | "~" expression
               | expression "?" expression ":" expression
               | matchExpression
               | expression ( "==" | "!=" | "<" | ">" | "<=" | ">=" ) expression
               | identifier "=" expression
               | identifier ( "+=" | "-=" | "*=" | "/=" | "%=" ) expression
               | identifier ( "[" expression "]" )+ "=" expression
               | identifier ( "[" expression "]" )+
               | expression "." identifier
               | identifier typeArguments? "(" ( expression ( "," expression )* )? ")"
               | expression "." identifier "(" ( expression ( "," expression )* )? ")"
               | "length" "(" identifier ")"
               | conversion "(" expression ")" ;

matchExpression = "match" "(" identifier ")" "{" matchArm ( "," matchArm )* "}" ;
matchArm      = ( literal | "default" ) "=>" expression ;
conversion     = "to_int32" | "to_uint32" | "to_int64" | "to_uint64"
               | "to_float" | "to_double" ;
literal        = integerLiteral | floatingLiteral | stringLiteral
               | "true" | "false" ;
```

A `foreach` over a rank-N array takes either N index names or N index names followed by one value name. A `foreach` over `start..end` takes one index name and iterates the exclusive `uint32` range. `else if` is nested `ifStatement` and emits as a native C `else if` chain.

`!`, `-` and `~` bind tighter than bitwise operators; shifts bind tighter than comparisons; `&`, `^` and `|` bind tighter than `&&`, which binds tighter than `||`. Ternary `?:` binds above assignment. Parentheses may group any expression.

Generic applications, including `result_type<T, status_code>`, use `typeArguments`. The compiler checks the arity, the result family and, for user payloads, that `T` implements `dump(): string`.
