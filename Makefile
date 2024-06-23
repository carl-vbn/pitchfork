CXX := gcc

OBJ_PATH := obj
SRC_PATH := src
INCLUDE_PATH := include

CXXFLAGS := -g -Wall -I$(INCLUDE_PATH)
LDFLAGS :=
LDLIBS := -lyaml

SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
TARGET := pitchfork

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* $(INCLUDE_PATH)/%.h
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<
 
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -f $(TARGET)

.PHONY: all
all: clean $(TARGET)
