# Fryte-Compiler
A compiler held together by fairy-dust for the Fryte interpreter.

# Currently supported
Variables of types string, char, int, bool and auto (if a default value is given, not really a type). 8-bit maximum, excluding strings, at the moment.

Math involving the operators +, -, *, / and %. Math can be made up of other variables or numbers.

Assignment operators +=, -=, *=, /= and %=.

Escape sequences in strings, supports \n, \t, \v, \b, \r, \f and \a. C++ equivalents. 

Console output

Console input

Conditional IF, ELSE, FOR and WHILE statements with support for the operators ==, !=, >, <, >= and <=, and the logical operators OR (|) and AND (&)

Gotos <- Have fun

Declaring/initialising/setting/using variables.

Functions with arguments and return statements which can return values

Statically sized arrays of all data types

# Sample program

```
string getName()
{
    string name
    Console.print("\nWhat is your name? ")
    Console.scan(name)
    return name
}

int getAge()
{
    int age
    Console.print("\nWhat is your age? ")
    Console.scan(age)
    return age
}

void entry()
{
    Console.print("\nHello ", getName(), ", you are ", getAge(), " years old!")
}
```

# Another sample program

```
string key = "password1"
string entered
int attemptsRemaining = 3

while((entered != key) & (attemptsRemaining > 0)) 
{
    Console.print("Enter the password: ")
    Console.scan(entered)
    if(entered == key)
    {
        Console.print("Password was correct!")
    }
    else
    {
        Console.print("Password incorrect. Attempts remaining: ", attemptsRemaining - 1, "\n")
        attemptsRemaining -= 1
    }
}
```

# Yet another sample program

```
for(int a = 0, a < 10, a += 1)
{
    if((a % 2) == (0))
    {
        Console.print(a, " is even!\n")
    }
    else
    {
        Console.print(a, " is odd!\n")
    }
}
```
