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
               | "break" ";"
               | "continue" ";"
               | structDeclaration
               | enumDeclaration
               | traitDeclaration
               | ifStatement
               | whileStatement
               | forStatement
               | foreachStatement
               | repeatStatement
               | functionDeclaration
               | returnStatement
               | block ;

block          = "{" statement* "}" ;

importDeclaration = "import" "{" identifier ( "," identifier )* "}"
                    "from" stringLiteral ";" ;

builtinType    = "int32" | "uint32" | "int64" | "uint64" | "float" | "double"
               | "boolean" | "string" | "arguments" ;
typeName       = builtinType | identifier typeArguments? ;
typeArguments  = "<" typeName ( "," typeName )* ">" ;
typeParameters = "<" identifier ( "," identifier )* ">" ;
arrayModifier  = "array" ( "[" integerLiteral "]" )* ;
type           = typeName ( "[" integerLiteral "]" )? ;
arrayInitializer = "{" ( expression | arrayInitializer )
                     ( "," ( expression | arrayInitializer ) )* "}" ;
declaration    = "const"? ( arrayModifier typeName | type ) identifier
                 ( "=" ( expression | arrayInitializer ) )? ";" ;

structDeclaration = "struct" identifier typeParameters? inheritance? traitUses?
                    "{" structMember* "}" ;
inheritance    = ":" identifier ;
traitUses      = "uses" identifier ;
structMember   = declaration
               | constructorDeclaration
               | functionDeclaration
               | staticFunctionDeclaration ;
constructorDeclaration = "constructor" "(" parameters ")" block ;
staticFunctionDeclaration = "static" functionDeclaration ;

enumDeclaration = "enum" identifier "{" identifier ( "," identifier )* "}" ;
traitDeclaration = "trait" identifier "requires" fieldRequirement*
                   "{" functionDeclaration* "}" ;
fieldRequirement = type identifier ;

functionDeclaration = "function" identifier typeParameters? "(" parameters ")"
                      ( ":" type )? block ;
parameterMode  = "clone" | "reference" ;
parameter      = "const"? parameterMode ( "array" | type ) identifier ;
parameters     = ( parameter ( "," parameter )* )? ;
returnStatement = "return" expression? ";" ;

ifStatement    = "if" "(" expression ")" block
                 ( "else" ( block | ifStatement ) )? ;
whileStatement = "while" "(" expression ")" block ;
forStatement   = "for" "(" ( declaration | expression? ) ";"
                 expression ";" expression ")" block ;
foreachStatement = "foreach" "(" identifier ( "," identifier )* "in"
                    identifier ")" block ;
repeatStatement = "repeat" "(" expression ")" block ;

expression     = literal
               | identifier typeArguments?
               | "(" expression ")"
               | "!" expression
               | "-" expression
               | expression "&&" expression
               | expression "||" expression
               | expression ( "+" | "-" | "*" | "/" | "%" ) expression
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

conversion     = "to_int32" | "to_uint32" | "to_int64" | "to_uint64"
               | "to_float" | "to_double" ;
literal        = integerLiteral | floatingLiteral | stringLiteral
               | "true" | "false" ;
```

A `foreach` over a rank-N array takes either N index names or N index names followed by one value name. `else if` is nested `ifStatement` and emits as a native C `else if` chain.

`!` binds tighter than comparisons. `&&` binds tighter than `||`. Assignment has the lowest precedence. Parentheses may group any expression.

Generic applications, including `result_type<T, status_code>`, use `typeArguments`. The compiler checks the arity and the closed result family after parsing.
