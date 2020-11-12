ROOT_DIR := $(shell git rev-parse --show-toplevel)
BUILD_DIR := $(ROOT_DIR)/build

all: $(BUILD_DIR)
	cd $(BUILD_DIR); make;

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR); cmake $(ROOT_DIR);