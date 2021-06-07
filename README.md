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

## About `autoMerger.py`

可以正確Merge的一些條件:
- 他會根據code中的`// Mark?:`做判斷，看到這種開頭盡量不要更改。 
- 函數宣告放在`// Mark1: Functions declarations`下。  
- 函數定義放在`// Mark2: Functions definitions`下。
- 不要用global variables

autoMerger會自動merge好，並且將debug設成0，輸出為`main_merged.c`。  
**Important Note**: merge後的檔案是可以直接submit的，但僅會回答一種query。要merge前去`autoMerger.py`裡改`test_func`參數，設定你要測哪一種query。


## About DEBUG mode (To 西西 & 洪)

我在`main.c`裡寫了一個debug mode，關於怎麼用我寫一點註解在code裏。要打開的話把code一開始的`#define DEBUG`設成1就好。  
他可以做以下的事情  
1. 他可以讓你指定要測qid為多少的query。
2. 他可以讓你指定要測某一類型的query。
3. 他可以幫你把每一個query的答案output進`output.txt`，然後我寫了一個`checkAnswer.py`，他可以幫你確認哪一個query有錯，與從哪裡開始錯，錯誤資訊會output成`JudgeInfo.txt`。


## To-do List (So far)

See [HackMD](https://hackmd.io/@Xr9r_83jRj64P3utQtN3zg/SkcugdM9d).


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

