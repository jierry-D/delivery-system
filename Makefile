CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.

CORE_SRCS = src/Graph.cpp src/Dijkstra.cpp src/TopoSort.cpp \
            src/OrderManager.cpp src/FileManager.cpp

# ── CLI ──────────────────────────────────────────────────
cli: main.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) -o delivery $^

run-cli: cli
	./delivery

# ── Qt GUI ───────────────────────────────────────────────
MOC     := $(shell which moc-qt5 2>/dev/null || which moc 2>/dev/null)
QTFLAGS := $(shell pkg-config --cflags Qt5Widgets)
QTLIBS  := $(shell pkg-config --libs Qt5Widgets)

GUI_SRCS = main_gui.cpp gui/MainWindow.cpp gui/GraphWidget.cpp $(CORE_SRCS)

moc_MainWindow.cpp: gui/MainWindow.h
	$(MOC) $(QTFLAGS) $< -o $@

moc_GraphWidget.cpp: gui/GraphWidget.h
	$(MOC) $(QTFLAGS) $< -o $@

qt-gui: moc_MainWindow.cpp moc_GraphWidget.cpp $(GUI_SRCS)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) -fPIC -o delivery_qt $^ $(QTLIBS)

run-qt: qt-gui
	DISPLAY=:1 ./delivery_qt

# ── 清理 ─────────────────────────────────────────────────
clean:
	rm -f delivery delivery_qt moc_*.cpp *.o

.PHONY: cli qt-gui run-cli run-qt clean
