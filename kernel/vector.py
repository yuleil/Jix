#!/usr/bin/python 

print '.globl alltraps\n'

l = range(10, 15) + [1, 17,]

for i in range(0, 256):
    print '.globl vector' + str(i)
    print 'vector%d:' % i
    if (i not in l):
        print ' pushl $0'
    print ' pushl $' + str(i)
    print ' jmp alltraps'

print '\n\n# vector table\n'
print '.data'
print '.globl vectors'
print 'vectors:'
for i in range(0, 256):
    print ' .long vector' + str(i)
