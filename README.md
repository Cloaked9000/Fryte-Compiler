# Fryte-Compiler
A compiler held together by fairy-dust for the Fryte interpreter.

# Currently supported
Variables of types string, char, int, bool and auto (if a default value is given, not really a type). 8-bit maximum, excluding strings, at the moment.

Math involving the operators +, -, *, / and %. Math can be made up of other variables or numbers.

Assignment operators +=, -=, *=, /= and %=.

Escape sequences in strings, supports \n, \t, \v, \b, \r, \f and \a. C++ equivalents. 

Console output

Console input

If statements with support for the operators ==, !=, >, <, >= and <=.

Gotos <- Have fun

Declaring/initialising/setting/using variables.


# Sample program

string key = "password1"
string entered
int attemptsRemaining = 3

int a = ((10 + 20) + (2))
Console.print ("Result: ", ((10 + 20) + (2)), "\n")

mypass:
Console.print ("Enter the password: ")
Console.scan (entered)
if (entered == key)
{
    Console.print ("Password was correct!")
}
if (entered != key)
{
    Console.print ("Password incorrect. Attempts remaining: ", attemptsRemaining, "\n")
    attemptsRemaining -= 1
    if (attemptsRemaining >= 0)
    {
        goto mypass
    }
}