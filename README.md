# dsa2021_final_project
This is the final project repo of Team 44 (峻瑋好帥) for DSA 2021 @ NTU CSIE.

### Team Members
- 資工一 劉峻瑋 b09902009
- 資工一 王秀軒 b09902033
- 資工一 洪郁凱 b09902040

## Include Structure
```bash
├── main.c
    ├── expression_match.h
    │   ├── hashTable.h
    ├── find_similar.h
    ├── group_analyze.h
    └── api.h
```

## About DEBUG mode (To 西西 & 洪)

我在`main.c`裡寫了一個debug mode，關於怎麼用我寫一點註解在code裏。要打開的話把code一開始的`#define DEBUG`設成1就好。  
他可以做以下的事情  
1. 他可以讓你指定要測qid為多少的query。
2. 他可以讓你指定要測某一類型的query。
3. 他可以幫你把每一個query的答案output進`output.txt`，然後我寫了一個`checkAnswer.py`，他可以幫你確認哪一個query有錯，與從哪裡開始錯，錯誤資訊會output成`JudgeInfo.txt`。


## To-do List (So far)

1. group_analyze可以做path compression
2. 檢查group_analyze的initialization是否有和其他兩個函數的initialization做重複的事
3. 目前分成`.h`會比一個1000多行的`main.c`容易debug。但這樣每次要繳交要一直複製合併很麻煩。要寫一個可以自動把五份`.h`merge進main.c，可以直接拿去丟DSA Judge的(用Python or C, whatever)，方法目前我想的很簡單，只要根據`Include Structure`，把最底層在放在檔案前面，這樣merge應該就沒問題了(Jun: 這個我可以寫)。



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

