#目标：依赖
#(TAB)执行
#if（目标被调用）
#	if（依赖 == true）
	#then  执行
all:s c

s:server.c
	gcc -o server server.c server_fun.c -lpthread -g

c:client.c
	gcc -o client client.c client_fun.c game.c -lpthread -g

clean:
	rm -rf *.o *.i *.s server client
