#!/usr/bin/env python3
import glob

filenames = glob.glob('./data/time/*.txt')
index = []
data2 = []
i = 0
for file in filenames: 
    with open(file, 'r') as f:
        datas = f.readlines()
    for j in range(0, len(datas)):
        s = datas[j].split()
        if i == 0:
            index.append(int(s[0]))
            data2.append(int(s[1]))
        else:
            data2[j] += int(s[1])
    #break
    i = 1
for j in range(0, len(data2)):
    data2[j] = data2[j] / 10
f = open('./data/final.txt', 'w')
for i in range(0, len(data2)):
    f.write('%i %i\n' % (index[i],data2[i]))