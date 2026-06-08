CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SFMLLIBS = -lsfml-graphics -lsfml-window -lsfml-system

CORE_SRCS = src/Graph.cpp src/Dijkstra.cpp src/TopoSort.cpp \
            src/OrderManager.cpp src/FileManager.cpp

all: cli gui

cli: main.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) -o delivery $^

gui: main_gui.cpp gui/GuiApp.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) -o delivery_gui $^ $(SFMLLIBS)

clean:
	rm -f delivery delivery_gui

run-cli: cli
	./delivery

run-gui: gui
	./delivery_gui
