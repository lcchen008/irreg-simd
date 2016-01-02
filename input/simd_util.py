f = open('soc-pokec-tuple.txt')

line = f.readline()
[q, w] = line.split()
w = int(w)
print w
line = f.readline()
x = 0
padding = 0
c = 0
while line:
	[a, a1, a2] = line.split()
	if x==a:
		c = c + 1
	else:
		c = c % 16
		if c != 0:
			c = 16 - c
		padding = padding + c 
		x = a
		c = 0
	line = f.readline()
	if line=='0':
		break

print 1.0 * w / (w + padding)