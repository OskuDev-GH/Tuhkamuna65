CC = gcc

SRC_DIR = src
BUILD_DIR = build

build: always
	$(CC) $(SRC_DIR)/*.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o $(BUILD_DIR)/tuhkamuna65

always: 