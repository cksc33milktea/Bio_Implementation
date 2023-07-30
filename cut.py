import numpy as np
import random
import math
import matplotlib.pyplot as plt

LUT = 4000
cut_stage = 4

f = open('./time.txt', "r")  # 開啟時間檔案

num_of_op = int(f.readline())  # 總共有幾個op

initial_time = []

for i in range(num_of_op):  # load原始執行時間
    initial_time.append(int(f.readline()))

f.close()

stage_time = sum(initial_time)/cut_stage  # 根據有幾個stage，計算stage time

print("sum of execution time and stage time: ", sum(initial_time), stage_time)

cut_point = []

tt = 0
for i in range(num_of_op):  # 算出那些點為cut point

    tt = tt+initial_time[i]
    if (tt > stage_time):
        tt = 0
        cut_point.append(i+1)

cut_point.append(num_of_op)  # 將尾巴append
print("cut point : ", cut_point)

stage_real_time = []

tt = 0
now = 0
for i in range(num_of_op):  # 算出每個stage實際執行時間

    if (i < cut_point[now]):
        tt = tt+initial_time[i]
    elif (i == cut_point[now]):
        now = now+1
        stage_real_time.append(tt)
        tt = initial_time[i]

stage_real_time.append(tt)
print("stage time : ", stage_real_time)
# print("compare : ", sum(initial_time), sum(stage_real_time))


is_conv = []  # 每個op是否為conv
summation = []  # 若為conv，算出複雜度
for i in range(num_of_op):
    is_conv.append(False)
    summation.append(0)

f = open('./conv.txt', "r")

conv_num = int(f.readline())
print("total conv ops : ", conv_num)

for i in range(conv_num):  # 算出conv op複雜度

    op = int(f.readline())
    is_conv[op] = True
    sum1 = 1

    for j in range(3):

        line = f.readline()
        line = line.strip('\n')
        line = line.split()
        if (j == 1):
            sum1 = sum1*int(line[0])*int(line[1])*int(line[2])*int(line[3])
        if (j == 2):
            sum1 = sum1*int(line[2])*int(line[3])
        # print(line)
        # print(len(line))
    summation[op] = int(sum1/1000000)
# print(is_conv)
print("conv op complexity : ", summation)
f.close()

tt = 0
now = 0

LUT_usage = []  # op的LUT使用量
conv_base_time = []  # op使用一單位conv加速器的執行時間
for i in range(num_of_op):
    LUT_usage.append(0)
    conv_base_time.append(int(summation[i]*300))
    
def insert():

    global LUT

    while (True):
        node_num = random.randint(0, num_of_op-1)
        if (is_conv[node_num] and LUT_usage[node_num] < 411):
            # if (is_conv[node_num]):
            break

    # print(num_of_op, node_num)

    complexity = summation[node_num]

    if (LUT-complexity < 0):
        return
    else:
        LUT = LUT-complexity
        LUT_usage[node_num] = LUT_usage[node_num]+1


def remove():

    global LUT

    while (True):
        node_num = random.randint(0, num_of_op-1)
        if (is_conv[node_num]):
            break

    # print(num_of_op, node_num)

    complexity = summation[node_num]

    if (LUT_usage[node_num] == 0):
        return
    else:
        LUT = LUT+complexity
        LUT_usage[node_num] = LUT_usage[node_num]-1
        
def calculate():

    new_time = []
    for i in range(num_of_op):  # 算出新執行時間(假設每個conv使用兩單位加速器)
        if (is_conv[i] and LUT_usage[i] > 0):
            new_time.append(int(conv_base_time[i]/LUT_usage[i]))
        else:
            new_time.append(initial_time[i])

    # print("new execution time : ", new_time)
    new_stage_time = sum(new_time)/cut_stage
    # print("sum of new execution time and stage time: ",
    #    sum(new_time), new_stage_time)

    new_cut_point = []

    tt = 0
    for i in range(num_of_op):

        tt = tt+new_time[i]
        if (tt > new_stage_time):
            tt = 0
            new_cut_point.append(i+1)

    new_cut_point.append(num_of_op)
    # print("new cut point : ", new_cut_point)

    new_stage_time = []
    tt = 0
    now = 0

    for i in range(num_of_op):

        if (i < new_cut_point[now]):
            tt = tt+new_time[i]
        elif (i == new_cut_point[now]):
            now = now+1
            new_stage_time.append(tt)
            tt = new_time[i]
    new_stage_time.append(tt)

    return new_stage_time
    # print("new_stage_time : ", new_stage_time)


minimum = max(stage_real_time)
print("original time period : ", minimum)
optimal_time = []

t0 = 5000  # Initial temperature
tmin = 1  # End of iteration, which means minimum temperature
k = 10  # Iteration in every temperature steps
coolnum = 0.98

t = t0

evetime_distance = []

while True:
    if t <= tmin:
        break
    for times in range(k):

        operation = random.randint(0, 1)

        if (operation == 0):
            remove()
            # print("remove!")
        else:
            insert()
            # print("insert!")

        new_stage_time = calculate()
        diff = max(new_stage_time) - minimum

        if diff < 0:
            minimum = max(new_stage_time)
            optimal_time = new_stage_time
        elif (diff > 0):
            prob = math.exp(-diff/t)

            randnum = random.uniform(0, 1)
            if randnum < prob:
                minimum = max(new_stage_time)
                optimal_time = new_stage_time
            else:
                minimum = minimum
                optimal_time = optimal_time

    evetime_distance.append(minimum)
    t = t * coolnum


print("optimal time period : ", minimum)
print(optimal_time)
print(LUT)
print(LUT_usage)

plt.figure(figsize=(15, 8))
plt.xlabel("Iteration", fontsize=15)
plt.ylabel("Distance", fontsize=15)

plt.plot(evetime_distance, linewidth=2.5,
         label="Everytime smallest distance", color='r')
plt.legend()
plt.show()
