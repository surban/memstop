memstop.so: memstop.c Makefile
	gcc -Wall -s -Os -shared -fPIC -o memstop.so memstop.c

install: memstop.so
	install -m 755 memstop.so /usr/local/lib

clean:
	rm -f memstop.so
