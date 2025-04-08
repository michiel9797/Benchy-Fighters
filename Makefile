OBJS	= action_list.cc engine.cc main.cc
SOURCE	= action_list.o engine.o main.o
HEADER	=
OUT	= BenchyFighters
CC	= c++
FLAGS	= -Wall -Weffc++ -std=c++17 -O2 -Iinclude

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

action_list.o: action_list.cc
	$(CC) $(FLAGS) action_list.cc

engine.o: engine.cc
	$(CC) $(FLAGS) engine.cc

main.o: main.cc
	$(CC) $(FLAGS) main.cc

clean:
	rm -f $(SOURCE) $(OUT)