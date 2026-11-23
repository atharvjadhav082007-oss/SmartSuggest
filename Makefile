CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -O2
LDFLAGS = 

# Detect Windows OS for linking winsock
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
    TARGET_APP = app.exe
    TARGET_TEST = test_runner.exe
else
    TARGET_APP = app
    TARGET_TEST = test_runner
endif

COMMON_SRCS = src/Dictionary.cpp src/TrieNode.cpp src/Trie.cpp src/Levenshtein.cpp src/Autocomplete.cpp src/SpellChecker.cpp src/ParagraphChecker.cpp src/SpellingGame.cpp src/WhatsAppBot.cpp
MAIN_SRCS = $(COMMON_SRCS) src/server.cpp src/main.cpp
TEST_SRCS = $(COMMON_SRCS) src/test_cases.cpp

.PHONY: all clean run test server

all: $(TARGET_APP) $(TARGET_TEST)

$(TARGET_APP): $(MAIN_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_TEST): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test: $(TARGET_TEST)
	./$(TARGET_TEST)

run: $(TARGET_APP)
	./$(TARGET_APP)

server: $(TARGET_APP)
	./$(TARGET_APP) --server

clean:
	rm -f $(TARGET_APP) $(TARGET_TEST) *.o
