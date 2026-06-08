CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.

CORE_SRCS = src/Graph.cpp src/Dijkstra.cpp src/OrderManager.cpp

MOC     := $(shell which moc-qt5 2>/dev/null || which moc 2>/dev/null)
QTFLAGS := $(shell pkg-config --cflags Qt5Widgets)
QTLIBS  := $(shell pkg-config --libs Qt5Widgets)

GUI_SRCS = main_gui.cpp gui/MainWindow.cpp $(CORE_SRCS)

moc_MainWindow.cpp: gui/MainWindow.h
	$(MOC) $(QTFLAGS) $< -o $@

qt-gui: moc_MainWindow.cpp $(GUI_SRCS)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) -fPIC -o delivery_qt $^ $(QTLIBS)

run-qt: qt-gui
	./delivery_qt

clean:
	rm -f delivery delivery_qt moc_*.cpp *.o

.PHONY: qt-gui run-qt clean
