include Makefile.common

EXENAME=main_$(NID)_deb.exe
SRCDIR=src
BINDIR=bin/$(OS)/debug
EXEDIR=exe



OBJS = $(BINDIR)/main.o


################################################################################



all:$(EXENAME)

$(EXENAME): $(OBJS)
	mkdir -p $(EXEDIR)/
	$(CC) $(OBJS) $(LIBS) $(SYSLIBS) $(EFLAGS) -o $(EXEDIR)/$(EXENAME)


#$(BINDIR)/math/%.o: $(SRCDIR)/math/%.cpp
#	mkdir -p $(BINDIR)/math/
#	$(CC) $(CFLAGS) -c -o $@ $<


$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(BINDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<


################################################################################

clean:
	rm -rf $(BINDIR)
