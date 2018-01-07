fusefs: fusefs.c simpleFS.c helper.c
	gcc fusefs.c simpleFS.c helper.c -o fusefs `pkg-config fuse --cflags --libs` -g
clean: 
	rm fusefs *~
