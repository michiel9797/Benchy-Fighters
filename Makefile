SOURCE	 = action_list.cc engine.cc main.cc
OBJS	 = action_list.o network_layer.o engine.o main.o
OUT	 = BenchyFighters
CC	 = c++
FLAGS	 = -Wall -Weffc++ -std=c++17 -O2
INCLUDES = -Iyojimbo/serialize

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) 

%.o: %.cc
	$(CC) $(FLAGS) $(INCLUDES) -c $< -o $@

network_layer.o: yojimbo_layer.cc
	$(CC) $(FLAGS) $(INCLUDES) -c yojimbo_layer.cc -o network_layer.o

clean:
	rm -f $(OBJS) $(OUT)