# Makefile đơn giản hỗ trợ lệnh "make run" (run sẽ clean trước rồi build và chạy)

# Trình biên dịch và flags
CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra

# Các object files
OBJS    := main.o csv_lib.o

# Tên executable sau khi link
TARGET  := main

.PHONY: all clean run

# Mặc định: build executable
all: $(TARGET)

# Link các object thành executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile từng .c thành .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Xóa object và executable
clean:
	rm -f $(OBJS) $(TARGET)

# Chạy: clean → build → execute
run: clean all
	./$(TARGET)
