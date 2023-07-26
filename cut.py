


f = open('./time.txt', "r")

tmp=f.readline()
num_of_op=int(tmp)

total_time=0

for i in range(num_of_op):
    tmp=f.readline()
    total_time=total_time+float(tmp)
    
print(total_time)

f.close()
