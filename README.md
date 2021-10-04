# Pseudocode to C++ transcompiler

Transcompiles a dynamically-typed "pseudocode" language, Ptitsa, to C++

<img style="display: block;
  margin-left: auto;
  margin-right: auto;" src="images/icon.jpg"/> 

# Ptitsa specification

## Interesting features a glance:
Automatic iterating variable in loops:
```
names = ["Nick", "Alan", "Tom"]
go over names:
  show name 
  { variable `name` is auto-generated }
```

English-reading boolean expressions:
```
if x is 12 or 20
```

## Basic syntax
New lines separate statements
Comments are written in curly braces, e.g. `{ comment is here }`

## Functions
Functions do not need brackets, e.g. `show "hi!"`, `length [1, 2, 5]`
Functions surrounded in other code will need surrounding brackets, e.g. `if (sum a, b) is 12`

## Data
There are three primitive data types: strings, numbers and booleans.

### String literals
String literals are enclosed in double-quotes, e.g. `"hi there!"`

### Numeric literals
Numeric literals are written as standard, e.g. `12`, `-3.1`

### Boolean literals
`true` and `false` are the two boolean literals

## Boolean expressions
`is` is used for comparison, e.g. `x is 12`
`isnt` is used for negated comparison, e.g. `x isnt 13`
`and`, `or` and `not` are the core logical operators. They allow for shorter statements: e.g. `x is 12 or 13` is the same as `x is 12 or x is 13`.
A consequence of this is that, if `and` or `or` are used, boolean variables must be followed by `is true`, 
E.g. `x is 12 or b is true` is written istead of `x is true or b` for boolean `b`.


## Lists
Lists can contain any of the three primitive data types.
Lists are written in comma-separated square brackets, e.g. `[a, b, c]`. 
If `a` and `b` are numbers, the syntax `a to b` generates a list of numbers ranging from `a` to `b` inclusive.

## Variables
Variables are dynamically typed
Variable declaration or assignment uses `=`, e.g. `x = 12`. 

## `if` statements
The head of an `if` statement is prefaced by `if`
The body of if statements are indented and are written on a new line, e.g.
```
if x is 12
    { do line 1 }
    { do line 2 }
{if statement has ended here}
```

## `for each` loops
`for each` loops iterate over each element in a list. The values in a list are iterated by reference, e.g.
```for each el in list 
	show el
	el = "Bob"
```
has `el` iterating over each element in `list`. For each `el`, `el` is first shown to the command line, then `list` is updated with `el` being set to `"Bob"`.

Coupled with the `a to b` syntax, 
```
for each x in a to b
	{ do line 1 }
```
constructs a `for each` loop with `x` ranging from the number `a` to the number `b` (inclusive).


`go over` is a special kind of for loop. It automatically makes element variable if list name ends in 's', e.g.
```
go over names:
	show name
	name = "Bob" { shows each name, then changes name to be "Bob". }
```
