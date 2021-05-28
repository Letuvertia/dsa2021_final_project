# dsa2021_final_project
This is the final project repo of Team 44 (峻瑋好帥) for DSA 2021 @ NTU CSIE.

### Team Members
- 資工一 劉峻瑋 b09902009
- 資工一 王秀軒 b09902033
- 資工一 洪郁凱 b09902040

## Include Structure
+-- main.c
|   +-- expression_match.h
|   |   +-- hashTable.h
|   +-- find_similar.h
|   +-- group_analyze.h
|   +-- api.h

## Some Notes (from Jun)

由於`main.c`這邊是我寫的，怕你們之後debug會有看不懂的困擾，這邊解釋一下。
我自己習慣寫debug，我都叫他verbose(碎碎念)，所以如果你們看到那個就代表可以直接忽略。
我上面有一個`#define DEBUG`，你把他設0他就不會一直print東西了。

## test env

The testdata was generated with the 10000 mails.
There are 10000 queries for you to check your implementations before submitting your code to the DSA Judge.

### Usage (for LINUX/MAC)

1. replace main.c by your own solution
2. run command "make run" to get your score of these 10000 queries

### Usage (for Windows)

1. replace main.c by your own solution
2. open your terminal (cmd or powershell) in this directory and run the commands below to compile the source codes
    - `gcc main.c -o main -O3 -std=c11 -w`
    - `g++ validator/validator.cpp -o validator/validator -O3`
3. run the command `main < testdata\test.in | validator\validator` under cmd, or `cmd.exe /c "main < testdata/test.in | validator\validator"` under powershell to get your score

### Notes and Hints

- You'll need gcc and g++ to compile the main.c and validator.cpp
- For Windows user, you may need to put your gcc/g++ compiler path into the "PATH" environment variable to run compilations successfully, also, you are highly suggested to work on CSIE workstations or WSL (Windows Subsystem for Linux), which are linux platforms
- Printing to standard error is a good way to debug.
- If you found your operating system did not support clock_gettime system call, which causes compilation error. You can modify the api.h so that you can test your code without time limitation.
- You can dump the input queries by output the contents of query array.
- The answers of each query is stored in testdata/test.ans

