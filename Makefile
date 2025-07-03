memstop.so: memstop.c Makefile
	gcc -s -Os -shared -fPIC -o memstop.so memstop.c

clean:
	rm -f memstop.so
