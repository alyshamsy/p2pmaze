
####################################################################################################
##
## CONFIGURATION
##
####################################################################################################

## Compilation parameters

CC = gcc
CXX = g++
CXXFLAGS := `sdl-config --cflags` -I./src -Wall -g
## Other CXXFLAGS: (also defined in src/Settings.h)
## -D__DISABLE_RATE_MONITOR__	<== don't monitor trasnfer: faster transfer but no statistics
## -D__COMPRESSED_MESSAGES__	<== update messages from peer to clients are compressed with zlib
## -D__MESSAGE_BUFFER__		<== each thread stores messages in a private buffer and periodicaly
## 				transfers them to the main queue
## -D__PEER_PLAYER_RATIO__	<== each player has a coefficient depending on its speed (works well
##				only if the peer has one regular update thread)
## -D__USE_3DS_GRAPHICS__	<== use 3ds models instead of vrml for client GUI
LDFLAGS := `sdl-config --libs` -lSDL_net
## -lz is also needed when compiling with -D__COMPRESSED_MESSAGES__
LDFLAGS_CLIENT = $(LDFLAGS) -lGL -lGLU
LDFLAGS_PEER = $(LDFLAGS)
LDFLAGS_SUPERPEER = $(LDFLAGS)
LDFLAGS_PEER   = $(LDFLAGS)
LDFLAGS_SUPERPEER = $(LDFLAGS)

## Source and binary files

BUILD_DIR = build
SOURCE_DIR = src

SHARED_SOURCES = $(wildcard $(SOURCE_DIR)/*.cpp) \
	$(wildcard $(SOURCE_DIR)/utils/*.cpp)
GAME_SOURCES = $(wildcard $(SOURCE_DIR)/game/*.cpp)
COMM_SOURCES = $(wildcard $(SOURCE_DIR)/comm/*.cpp)
GRAPHICS_SOURCES = $(wildcard $(SOURCE_DIR)/graphics/*.cpp) \
	$(wildcard $(SOURCE_DIR)/graphics/vrml/*.cpp) \
	$(wildcard $(SOURCE_DIR)/graphics/3ds/*.cpp) \
	$(wildcard $(SOURCE_DIR)/graphics/texture/*.cpp) \
	$(wildcard $(SOURCE_DIR)/graphics/font/*.cpp)

## PEER
PEER_DIR = $(SOURCE_DIR)/peer
PEER_SRCS := $(wildcard $(PEER_DIR)/*.cpp) \
	$(GAME_SOURCES) \
	$(COMM_SOURCES) \
	$(SHARED_SOURCES)
PEER_OBJS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(PEER_SRCS))

## SUPERPEER
SUPERPEER_DIR = $(SOURCE_DIR)/superpeer
SUPERPEER_SRCS := $(wildcard $(SUPERPEER_DIR)/*.cpp) $(SHARED_SOURCES) $(GAME_SOURCES) $(COMM_SOURCES)
SUPERPEER_OBJS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SUPERPEER_SRCS))

## CLIENT
CLIENT_DIR = $(SOURCE_DIR)/client
CLIENT_SRCS := $(wildcard $(CLIENT_DIR)/*.cpp) \
	$(wildcard $(CLIENT_DIR)/AStar/*.cpp) \
	$(GRAPHICS_SOURCES) \
	$(GAME_SOURCES) \
	$(COMM_SOURCES) \
	$(SHARED_SOURCES)
CLIENT_OBJS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CLIENT_SRCS))

PEER_BINARY_NAME = peer
CLIENT_BINARY_NAME = client
SUPERPEER_BINARY_NAME = superpeer

## Parameters for running

SUPERPEER_CONFIG_FILE = config_default.ini
HOST_NAME := `uname -n`

####################################################################################################
##
## BUILD
##
####################################################################################################

all: client peer superpeer peer superpeer

## Create directories for object files
$(BUILD_DIR):
	@echo "Creating directories for the object files"
	@mkdir $(BUILD_DIR)
	@mkdir $(BUILD_DIR)/utils
	@mkdir $(BUILD_DIR)/client
	@mkdir $(BUILD_DIR)/client/AStar
	@mkdir $(BUILD_DIR)/comm
	@mkdir $(BUILD_DIR)/game
	@mkdir $(BUILD_DIR)/graphics
	@mkdir $(BUILD_DIR)/graphics/vrml
	@mkdir $(BUILD_DIR)/graphics/3ds
	@mkdir $(BUILD_DIR)/graphics/texture
	@mkdir $(BUILD_DIR)/graphics/font
	@mkdir $(BUILD_DIR)/peer
	@mkdir $(BUILD_DIR)/superpeer

## Build client and peer

graphics: $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(GRAPHICS_SOURCES))
game: $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(GAME_SOURCES))
shared: $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SHARED_SOURCES))
comm: $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(COMM_SOURCES))
client_only: $(CLIENT_OBJS)
peer_only: $(PEER_OBJS)
superpeer_only: $(SUPERPEER_OBJS)

client: $(BUILD_DIR) $(CLIENT_OBJS)
	$(CXX) $(LDFLAGS_CLIENT) -o $(CLIENT_BINARY_NAME) $(CLIENT_OBJS)
peer: $(BUILD_DIR) $(PEER_OBJS)
	$(CXX) $(LDFLAGS_PEER) -o $(PEER_BINARY_NAME) $(PEER_OBJS)
superpeer: $(BUILD_DIR) $(SUPERPEER_OBJS)
	$(CXX) $(LDFLAGS_SUPERPEER) -o $(SUPERPEER_BINARY_NAME) $(SUPERPEER_OBJS)

## Target for generic cpp files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

## Call another makefile to build the game monitor
gamemonitor:
	make -C src/gamemonitor

## Clean
.PHONY: clean
clean:
	-rm -f -R $(BUILD_DIR)
	-rm -f $(PEER_BINARY_NAME) $(CLIENT_BINARY_NAME) $(SUPERPEER_BINARY_NAME) $(SUPERPEER_BINARY_NAME)
	-rm -f core*
	-rm -f /tmp/superpeer.log
clean_all: clean
	-make -C src/gamemonitor clean

####################################################################################################
##
## RUN
##
####################################################################################################

## run
run_superpeer: superpeer
	./$(SUPERPEER_BINARY_NAME) $(SUPERPEER_CONFIG_FILE) 7878 /tmp/superpeer.log
run_peer: run_peer1
run_peer1: peer
	./$(PEER_BINARY_NAME) $(HOST_NAME):7878 $(HOST_NAME):3000
run_peer2: peer
	./$(PEER_BINARY_NAME) $(HOST_NAME):7878 $(HOST_NAME):3001
run_peer3: peer
	./$(PEER_BINARY_NAME) $(HOST_NAME):7878 $(HOST_NAME):3002
run_peer4: peer
	./$(PEER_BINARY_NAME) $(HOST_NAME):7878 $(HOST_NAME):3003

run_10:
	for (( i=1; i<=10; i++ )); do ./$(CLIENT_BINARY_NAME) $(HOST_NAME):3000 > /dev/null & echo -n $(i)" "; done
run_100:
	for (( i=1; i<=100; i++ )); do ./$(CLIENT_BINARY_NAME) $(HOST_NAME):3000 > /dev/null & echo -n $(i)" "; done
run_200:
	for (( i=1; i<=200; i++ )); do ./$(CLIENT_BINARY_NAME) $(HOST_NAME):3000 > /dev/null & echo -n $(i)" "; done
run_400:
	for (( i=1; i<=400; i++ )); do ./$(CLIENT_BINARY_NAME) $(HOST_NAME):3000 > /dev/null & echo -n $(i)" "; done
run_1000:
	for (( i=1; i<=1000; i++ )); do ./$(CLIENT_BINARY_NAME) $(HOST_NAME):3000 > /dev/null & echo -n $(i)" "; done

run_client: client
	./$(CLIENT_BINARY_NAME) $(HOST_NAME):3000
run_client_gui: client
	./$(CLIENT_BINARY_NAME) --gui $(HOST_NAME):3000

## client
## --gui --nogui --size=640x480 --bpp=32 --fullscreen --debug-AI --disconnect_timeout=5
## --console-fps --fps-average-param=0.99
