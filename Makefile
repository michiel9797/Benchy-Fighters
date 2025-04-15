OBJS	= action_list.cc network_layer.cc engine.cc main.cc
SOURCE	= action_list.o network_layer.o engine.o main.o
HEADER	=
OUT	= BenchyFighters
CC	= c++
FLAGS	= -Wall -Weffc++ -std=c++17 -O2 -Iinclude

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

action_list.o: action_list.cc
	$(CC) $(FLAGS) action_list.cc

network_layer.o: network_layer.cc
	$(CC) $(FLAGS) network_layer.cc

engine.o: engine.cc
	$(CC) $(FLAGS) engine.cc

main.o: main.cc
	$(CC) $(FLAGS) main.cc

clean:
	rm -f $(SOURCE) $(OUT)