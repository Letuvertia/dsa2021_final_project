import os
import numpy as np
from scipy.stats import chisquare

def toNumber(c):
    if (ord('A') <= ord(c) <= ord('Z')):
        return ord(c) - ord('A')
    if (ord('a') <= ord(c) <= ord('z')):
        return ord(c) - ord('a')
    if (ord('0') <= ord(c) <= ord('9')):
        return ord(c) - ord('0') + 26

def hash_djb2(s):
    hash = 5381
    for c in s:
        hash = (( hash << 5) + hash) + toNumber(c)
    return hash & 0xFFFFFFFF

def hash_RS(s):
    b = 378551
    a = 63689
    hash = 0
    for c in s:
        hash = hash * a + toNumber(c)
        a *= b
    return hash & 0xFFFFFFFF

def hash_SDBM(s):
    hash = 0
    for c in s:
        hash = (hash << 6) + (hash << 16) - hash + toNumber(c)
    return hash & 0xFFFFFFFF

def hash_JS(s):
    hash = 1315423911
    for c in s:
        hash = hash ^ ((hash << 5) + toNumber(c) + (hash >> 2)) 
    return hash & 0xFFFFFFFF

def hash_BKDR(s):
    seed = 131
    hash = 0
    for c in s:
        hash = hash * seed + toNumber(c)
    return hash & 0xFFFFFFFF

RK_Q = 59650000
RK_D = 36
def hash_RabinKarp(s):
    hash = 0
    for c in s:
        hash = (hash*RK_D + toNumber(c)) % RK_Q
    return hash

n_mails = 10000

for hash_func in [hash_djb2, hash_RS, hash_SDBM, hash_JS, hash_BKDR, hash_RabinKarp]:
#for hash_func in [hash_RabinKarp]:
    HASH_M = 2**15
    file = open(os.path.join(os.getcwd(), "token.txt"), "r")
    great_dict = dict()
    collision_ctr = 0
    token_ctr = 0
    for line in file:
        if line[0] == '&':
            mail_id = int(line.split(' ')[-1])
            if (mail_id % 1000 == 0):
                print("hashing...{}%".format(int(mail_id/n_mails*100)))
            continue
        s = str(line)[:-1]
        s_hash = hash_func(s) % HASH_M
        if s_hash not in great_dict.keys():
            great_dict[s_hash] = [s.lower()]
            token_ctr += 1
        else:
            if (s.lower() not in great_dict[s_hash]):
                #print('{} collide with {}; s_hash = {} and {}'.format(s, great_dict[s_hash], s_hash, [hash_func(str) for str in great_dict[s_hash]]))
                great_dict[s_hash].append(s.lower())
                token_ctr += 1
            #else:
            #    print('{} repeat with {}; s_hash = {} and {}'.format(s, great_dict[s_hash], s_hash, [hash_func(str) for str in great_dict[s_hash]]))
    boxN = [len(great_dict[key]) for key in great_dict.keys()]
    boxN += [0] * (HASH_M - len(boxN))
    assert(len(boxN) == HASH_M)
    boxN_np = np.asarray(boxN)
    print("function: {}; HASH_M: {} std: {}; chi-square: {}; token count: {}".format(hash_func.__name__, HASH_M, np.std(boxN_np), chisquare(boxN_np), token_ctr))
    #print("box occupied: {}; boxN_np: {}".format(len(great_dict), boxN_np))
    file.close()


''' hash2: collision
#for hash_func in [hash_djb2, hash_RS, hash_SDBM, hash_JS, hash_BKDR]:
for hash_func in [hash_RabinKarp]:
    file = open(os.path.join(os.getcwd(), "token.txt"), "r")
    great_dict = dict()
    collision_ctr = 0
    token_ctr = 0
    for line in file:
        if line[0] == '&':
            continue
        s = str(line)[:-1]
        s_hash = hash_func(s)
        if s_hash not in great_dict.keys():
            great_dict[s_hash] = [s.lower()]
            token_ctr += 1
        else:
            if (s.lower() not in great_dict[s_hash]):
                #print('{} collide with {}; s_hash = {} and {}'.format(s, great_dict[s_hash], s_hash, [hash_func(str) for str in great_dict[s_hash]]))
                great_dict[s_hash].append(s.lower())
                collision_ctr += 1
                token_ctr += 1
            #else:
            #    print('{} repeat with {}; s_hash = {} and {}'.format(s, great_dict[s_hash], s_hash, [hash_func(str) for str in great_dict[s_hash]]))
    print("function: {}; collision count: {}; token count: {}".format(hash_func.__name__, collision_ctr, token_ctr))
    file.close()
'''


''' each mail
mails_dict = list()
h2s_dict = dict()
for line in file:
    if line[0] == '&':
        mail_id = int(line.split(' ')[-1])
        print(mail_id)
        if mail_id > 0:
            mails_dict.append(h2s_dict)
            h2s_dict = dict()
            if (mail_id == 35):
                break
    else:
        s = str(line)[:-1]
        s_hash = hash_djb2(s)
        if s_hash not in h2s_dict.keys():
            h2s_dict[s_hash] = s
        else:
            if (s == h2s_dict[s_hash]):
                print('{} repeated'.format(s))
            else:
                print('{} collide with {}!!'.format(s, h2s_dict[s_hash]))
'''


    
    

