# Builds the wait-for graph driver. Run from the project root.
#
#   make           build ./wait_for_graph
#   make samples   build, then run the driver on every tests/testN.txt as run N,
#                  writing output_schedules/out_N.txt and
#                  graphs/wait_for_graph_N.graphml
#   make clean     remove the binary and the object files (the driver's outputs
#                  in output_schedules/ and graphs/ are kept)

CXX       = c++
CXXFLAGS  = -std=c++17 -Wall -Wextra -Iinclude
# Make the compiler note which headers each source includes, so that editing a
# header rebuilds the sources that use it
DEPFLAGS  = -MMD -MP

TARGET    = wait_for_graph
BUILD_DIR = build
SRCS      = $(wildcard src/*.cpp)
OBJS      = $(SRCS:src/%.cpp=$(BUILD_DIR)/%.o)
SAMPLES   = $(wildcard tests/test*.txt)

.PHONY: all samples clean

all: $(TARGET)

# Link the driver from one object file per source file
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# Compile one source file; the build directory only has to exist first
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

# The run number N is taken from the file name tests/testN.txt
samples: $(TARGET)
	@for f in $(SAMPLES); do \
		i=$${f#tests/test}; i=$${i%.txt}; \
		echo "== $$f (run $$i) =="; \
		./$(TARGET) $$i < $$f || exit 1; \
	done

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Header dependencies written by the compiler; absent before the first build
-include $(OBJS:.o=.d)
