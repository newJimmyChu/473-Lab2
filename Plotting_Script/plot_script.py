import sys
import matplotlib.pyplot as plt
# rootdir = sys.argv[1]
path = sys.argv[1]
rootdir = sys.argv[1] + '/../'
f1 = open(path,'r')

lines = f1.readlines()

y1 = []

for line in lines:
    y1.append(line)
	# y2.append(string[1])
	# print string[0],' ',string[1]

x = []
count = 1
x.append(1)
x.append(2)
x.append(4)
x.append(8)
x.append(16)
x.append(32)

# for items in y1:
# 	x.append(count)
# 	count += 1
# line 1 points
# plotting the line 1 points
line1, = plt.plot(x, y1, label = "Counter Value")

# line 2 points
# plotting the line 2 points
# plt.plot(x, y2, label = "Line 2")

# naming the x axis
plt.xlabel('x-axis -> Number of Threads')
# naming the y axis
plt.ylabel('y - axis -> Value of Counter')
# giving a title to my graph
plt.title('Performance vs. # Threads')

# show a legend on the plot
# line2, = plt.plot([3,2,1], label="Memory Consumption in KB", linewidth=1)
plt.legend()

# function to show the plot
# plt.show()

plt.savefig('Performance.png')
plt.show()
