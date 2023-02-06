LDLIBS += -lpcap

all: signal-strength

mac.o : mac.h mac.cpp

signal-strength : signal_strength.o mac.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f signal-strength *.o
