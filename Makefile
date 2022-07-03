CXX := g++
CXXFLAGS :=  

INC := include
BIN := bin
SRC := src
OBJ := obj
LIB := lib

SDL_INC := i686-w64-mingw32/include/SDL2
SDL_LIB := i686-w64-mingw32/lib
ENGINE_SRC := GameEngine/src
ENGINE_LIB := GameEngine/lib
ENGINE_INC := GameEngine/include

INCLUDE_PATHS := -I$(INC)/$(ENGINE_SRC) \
	-I$(INC)/$(ENGINE_INC)/SDL2-2.0.12/$(SDL_INC) \
	-I$(INC)/$(ENGINE_INC)/SDL2_image-2.0.5/$(SDL_INC) \
	-I$(INC)/$(ENGINE_INC)/SDL2_ttf-2.0.15/$(SDL_INC)
LIBRARY_PATHS := -L$(INC)/$(ENGINE_LIB) \
	-L$(INC)/$(ENGINE_INC)/SDL2-2.0.12/$(SDL_LIB) \
	-L$(INC)/$(ENGINE_INC)/SDL2_image-2.0.5/$(SDL_LIB) \
	-L$(INC)/$(ENGINE_INC)/SDL2_ttf-2.0.15/$(SDL_LIB)
LINKER_FLAGS := -lGameEngine -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

DEPFLAGS := -MM $(INCLUDE_PATHS)

# Get all header file dependencies relative to ./ and convert to .cpp
SOURCES = $(shell realpath --relative-to ./ $(patsubst %.h,%.cpp,$(filter $(SRC)/%.h, $(shell $(CXX) $(DEPFLAGS) $1))))
# Filter out missing .cpp files
EXIST = $(filter $(foreach file,$(call SOURCES,$1),$(wildcard $(file))),$(call SOURCES,$1))
# Compute .d and .h dependencies
DEPS = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$1) $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(call EXIST,$1))
OBJS = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$1) $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(call EXIST,$1))

.PHONY: all clean

all: IdleWizard
	@$(BIN)/IdleWizard

IdleWizard: $(call OBJS,$(SRC)/IdleWizard.cpp)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@ $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)
-include $(call DEPS,$(SRC)/IdleWizard.cpp)

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@ $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)

clean:
	@find $(OBJ) -type f \( -name "*.o" -o -name "*.d" \) -delete

test: test.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)
