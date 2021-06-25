import os

ansfile_loc = os.path.join(os.getcwd(), "test_env", "testdata", "test.ans")
output_loc = os.path.join(os.getcwd(), "output.txt")
ansfile = open(ansfile_loc, "r")
outputfile = open(output_loc, "r")

waInfoFile = open(os.path.join(os.getcwd(), "JudgeInfo.txt"), "w")
ac = True

output_dict = dict()
for line in outputfile:
    idx = line.index(':')
    output_dict[int(line[:idx])] = str(line[idx+2:-1])

wrong_qid = []

for line in ansfile:
    idx = line.index(':')
    qid = int(line[:idx])
    if qid in output_dict:
        ans = str(line[idx+2:-1])
        if (len(output_dict[qid]) == 0 and len(ans) == 0):
            continue
        elif (len(output_dict[qid]) == 0 and len(ans) != 0):
            ac = False
            waInfoFile.write("WA: QID:{}. Your ans is blank:\n".format(qid))
            waInfoFile.write("\tYour Ans: blank\n")
            waInfoFile.write("\tTure Ans: {}...\n".format(ans[0: min(10, len(ans))]))
            wrong_qid.append(qid)
            continue
        elif (len(output_dict[qid]) != 0 and len(ans) == 0):
            ac = False
            waInfoFile.write("WA: QID:{}. Ture ans is blank:\n".format(qid))
            waInfoFile.write("\tYour Ans: {}...\n".format(output_dict[qid][0: min(10, len(output_dict[qid]))]))
            waInfoFile.write("\tTure Ans: blank\n")
            wrong_qid.append(qid)
            continue

        index = next((i for i in range(min(len(output_dict[qid]), len(ans))) if output_dict[qid][i]!=ans[i]), None)
        if index is not None:
            ac = False
            wrong_qid.append(qid)
            print("WA: QID:{}. Your ans does not match from &here&:".format(qid))
            print("\tYour Ans:{}&{}&{}".
                format(output_dict[qid][max(index-5, 0): index], output_dict[qid][index], output_dict[qid][index+1: min(index+10, len(output_dict[qid]))]))
            print("\tTure Ans:{}&{}&{}".
                format(ans[max(index-5, 0): index], ans[index], ans[index+1: min(index+10, len(ans))]))
            waInfoFile.write("WA: QID:{}. Your ans does not match from &here& (show [&-5:&+10]):\n".format(qid))
            waInfoFile.write("\tYour Ans: ...{}&{}&{}...\n".
                format(output_dict[qid][max(index-5, 0): index], output_dict[qid][index], output_dict[qid][index+1: min(index+10, len(output_dict[qid]))]))
            waInfoFile.write("\tTure Ans: ...{}&{}&{}...\n".
                format(ans[max(index-5, 0): index], ans[index], ans[index+1: min(index+10, len(ans))]))

if (ac):
    print("Accept!")
    waInfoFile.write("Accept!")
print("judge information above wrote into JudgeInfo.txt")
print("wrong qid's as follows:")
print(wrong_qid)
ansfile.close()
outputfile.close()