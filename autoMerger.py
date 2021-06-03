import os
from posixpath import join

# ============== 
# CONFIGURATIONS
# ==============
# enter the path to your judge
# path_to_judge_dir = r"/Users/anthoni/Desktop/dsa/team40/51/main.c"

# enter the function to want to test
# 0/1/2 : expression/find/group
test_func = 0


# ============== 
# MERGE
# ==============
merged_main_loc = os.path.join(os.getcwd(), "main_merged.c")
output_file = open(merged_main_loc, "w")

## write include
header_files = ["#include \"api.h\"",
                "#include <stdio.h>",
                "#include <string.h>",
                "#include <stdlib.h>",
                "#include <stdbool.h>",
                "#include <limits.h>"]
for header in header_files:
    output_file.write("{}\n".format(header))

## write global vars
global_vars = ["int n_mails, n_queries;\n",
               "mail *mails;\n",
               "query *queries;\n"]
for globle_var in global_vars:
    output_file.write("{}".format(globle_var))


include_struct = ["main.c",
                  "hashTable.h",
                  "expression_match.h",
                  "group_analyze.h",
                  "find_similar.h",]
constants = list()
func_declaration = list()
func_definition = list()

mode = ""
test_func_def = True
for file_name in include_struct:
    file = open(os.path.join(os.getcwd(), file_name), "r", encoding="utf-8")
    mode = ""
    func_definition.append("\n\n// =================================\n")
    func_definition.append("// == START of {} \n".format(file_name))
    func_definition.append("// =================================\n\n")
    for line in file:
        indented_line = line
        line = line.strip() + '\n'
        if line[:7] == "#define":
            if line == "#define DEBUG 1\n":
                constants.append("#define DEBUG 0\n") # submission-ready
            else:
                constants.append(indented_line)
            continue
        
        if line[:8] == "// Mark1":
            mode = "function_declare"
            continue

        if line[:8] == "// Mark2":
            mode = "function_define"
            continue
        
        if line[:9] == "// Mark3-":
            if (int(line[9]) == test_func) or int(line[9]) == 4:
                test_func_def = True
            else:
                test_func_def = False
            continue

        if mode == "function_declare":
            func_declaration.append(indented_line)
        elif (mode == "function_define") and test_func_def:
            func_definition.append(indented_line)
    
    func_definition.append("\n\n// =================================\n")
    func_definition.append("// == END of {} \n".format(file_name))
    func_definition.append("// =================================\n\n")

output_file.write("\n\n// =================================\n")
output_file.write("// == CONSTANTS\n")
output_file.write("// =================================\n\n")
for constant in constants:
    output_file.write(constant)

output_file.write("\n\n// =================================\n")
output_file.write("// == Mark1 - Function Declarations\n")
output_file.write("// =================================\n\n")
for line in func_declaration:
    output_file.write(line)

output_file.write("\n\n// =================================\n")
output_file.write("// == Mark1 - Function Definitions\n")
output_file.write("// =================================\n\n")
for line in func_definition:
    output_file.write(line)

output_file.close()
