#!/usr/bin/python 

f = open('boot/bootblock', 'r')
s = f.read(510)
f = open('boot/boot', 'w')
f.write(s + '\0' * (512 - 2 - len(s)) + '\x55\xaa')
f.close()


