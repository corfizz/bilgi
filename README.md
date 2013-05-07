bilgi
=====

A lightweight flashcards program

bilgi's development has only just started. It is intended to be a program to help you memorise things, 
taking input from a delimited text file and giving the user complete control over which fields are 
displayed to them, in what order and for how long, simulating a physical flashcard deck.

## Usage
bilgi [OPTIONS] -a ACTIONS FILE

## Actions list
To get bilgi to do anything at all, you need to supply a list of actions. This is a list of field numbers delimited however you like, which will be displayed in the order given. The special characters 'w' and 't' will pause the program. 'w' pauses the program until ENTER is pressed, and 't' pauses the program for a specific number of seconds. The default is 3 seconds; it can be changed with the -t option.

Example:
$ bilgi -a "1 w 2" mywords

Prints the first field, then waits for ENTER to be pressed, then prints the second field of the first line of the file mywords. Note that "1 w 2", "1w2", "1,w,2" and others are all equivalent. The only characters that bilgi parses in the actions listing are numbers, 'w' and 't'.

## Options

-a ACTIONS		list the actions to perform on the card; only obligatory option

-b				put the card to the back after acting

-c				continue to the next card

-d [CHAR]		specify the delimiter; the default is a tab character

-h				show usage

-i				move card to a new position interactively after actions are complete

-l				disable logging

-m NUM[%]		move card to position NUM after acting, or NUM% of the way through the file

-n NUM-[NUM]	use cards in the given range

-p				use the last card seen, via the log file

-r				use a random card

-s				shuffle the deck prior to acting

-t NUM			specify the sleep timeout in seconds

-x REGEXP		only operate on lines matching REGEXP

Moving a card works like this: if you enter NUM, the card will now be at position NUM in the deck. If you enter +NUM, it will be placed NUM cards further down the deck. If you enter -NUM, it will be moved up. The special character 'b' stands for the end of the deck. Moving cards around is 'destructive': it replaces the input file immediately to reflect the new order of the lines.

## Utility value
You don't have to use bilgi as a flashcard program. You could just use it as a utility for manipulating delimited data, including CSV and TSV files. bilgi could then be considered a more powerful alternative to cut, or a more limited alternative to sed/awk.
