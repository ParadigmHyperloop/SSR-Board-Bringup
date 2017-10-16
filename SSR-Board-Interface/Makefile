ssr: ssr.o i2cfunc.o
	cc ssr.o i2cfunc.o -o ssr

ssr_server: ssr_server.o i2cfunc.o
	cc ssr_server.o i2cfunc.o -o ssr_server

ssr_init: init.o i2cfunc.o
	cc init.o i2cfunc.o -o ssr_init

clean:
	rm -f *.o ssr ssr_init ssr_server

