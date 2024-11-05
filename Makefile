include ./Makefile.inc

SHARED_SOURCES=$(wildcard src/shared/*.c)
SERVER_SOURCES=$(wildcard src/server/*.c)
CLIENT_SOURCES=$(wildcard src/client/*.c)

OUTPUT_FOLDER=./bin
SERVER_OUTPUT_FILE=$(OUTPUT_FOLDER)/server
CLIENT_OUTPUT_FILE=$(OUTPUT_FOLDER)/client

all: server client

server:
	mkdir -p $(OUTPUT_FOLDER)
	$(COMPILER) $(COMPILERFLAGS) $(SHARED_SOURCES) $(SERVER_SOURCES) -o $(SERVER_OUTPUT_FILE)

client:
	mkdir -p $(OUTPUT_FOLDER)
	$(COMPILER) $(COMPILERFLAGS) $(SHARED_SOURCES) $(CLIENT_SOURCES) -o $(CLIENT_OUTPUT_FILE)

clean:
	rm -rf $(OUTPUT_FOLDER)

.PHONY: all server client clean
