# dynamic prerequisites on system:
# - glfw
# - freeimage

EXEC = orbit_trap

IMGUI_DIR = imgui
GLAD_DIR = glad
SRC_DIR = src
BUILD_DIR = build
$(info $(shell mkdir -p $(BUILD_DIR)))

CXXFLAGS = -g -Wall -Wextra -Wno-unused-parameter
LDLIBS = -lfreeimage

# SOURCES = src/main.cpp src/shader.cpp src/camera.cpp
SOURCES = $(shell find $(SRC_DIR) -name *.cpp) # gets all cpp files in SRC_DIR
CXXFLAGS += -I$(SRC_DIR)

# adds all IMGUI compiler variables
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends

## Using OpenGL loader: glad2
SOURCES += $(GLAD_DIR)/src/gl.c
CXXFLAGS += -I$(GLAD_DIR)/include -DIMGUI_IMPL_OPENGL_LOADER_GLAD2

# obj files names gotton from the $(SOURCES) names (and are gonna be stored in the $(BUILD_DIR))
OBJS := $(addsuffix .o, $(basename $(notdir $(SOURCES))))
OBJS := $(patsubst %,$(BUILD_DIR)/%,$(OBJS))

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM (mainly glfw and opengl)
##---------------------------------------------------------------------

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LDLIBS += -lGL `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LDLIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LDLIBS += -L/usr/local/lib -L/opt/local/lib
	#LDLIBS += -lglfw3
	LDLIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LDLIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

.PHONY: all
all: $(EXEC)
	@echo Build complete for $(ECHO_MESSAGE)

# link everything together
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

# compile and assamble personal source files
$(BUILD_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# compile and assamble imgui source files
$(BUILD_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# compile and assamble glad source files
$(BUILD_DIR)/%.o:$(GLAD_DIR)/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(EXEC) $(BUILD_DIR)
