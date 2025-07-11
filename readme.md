## Assumptions

- Maximum number of processes that can be done in background 1024

- The Maximum length of a char or path is 4096

- In foreground process i am displaying (the total command in the prompt) the latest foreground process in the prompt if continues foreground process are done 

- I am displaying the prompt only if the survives more than 2 seconds

## spec-3

- for the case of relative paths only ~/path only works remaining like -/path doesn't work

- if hop dir1 dir2 is the command given then it will run like hop dir1 and hop dir2 as there no input contraint on it

- hop . does no change

## spec-4

- assuming max no.of entries in the directory as 4096

- reveal - prints the home directory if no prev is present

## spec-5

- error commands are also stored in the log

- any command with "log" will not entered into the store.txt file

- command executing because of log execute is not being entered in the file

## spec 10

- assuming no input after the text file after tokening

## spec 11
- in this the hop doest change the directory 



