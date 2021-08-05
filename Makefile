ROOT_DIR := $(shell git rev-parse --show-toplevel)
BUILD_DIR := $(ROOT_DIR)/build

all: $(BUILD_DIR)
	cd $(BUILD_DIR); make;

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR); cmake $(ROOT_DIR);
	#cd $(BUILD_DIR); CC=/panfs/roc/msisoft/gcc/7.2.0/bin/gcc CXX=/panfs/roc/msisoft/gcc/7.2.0/bin/g++ cmake $(ROOT_DIR);
