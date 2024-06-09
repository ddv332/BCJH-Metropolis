#ifndef STATE_RECORDER_HPP
#define STATE_RECORDER_HPP
#include <fstream>
#include <iostream>
#include "States.hpp"
#include <vector>
#include "Chef.hpp"
#include "Recipe.hpp"

class StatesSerializer {
    CList *cl;
    RList *rl;

  public:
    StatesSerializer(CList *cl, RList *rl) {
        this->cl = cl;
        this->rl = rl;
    }

    static void serialize(std::ofstream &file, States *state) {
        int chefs[NUM_CHEFS];
        Tool tools[NUM_CHEFS];
        int recipe[DISH_PER_CHEF * NUM_CHEFS];
        for (int i = 0; i < NUM_CHEFS; i++) {
            chefs[i] = state->getChef(i).id;
            tools[i] = state->getTool(i);
        }
        for (int i = 0; i < NUM_CHEFS * DISH_PER_CHEF; i++) {
            recipe[i] = state->recipe[i] == NULL ? -1 : state->recipe[i]->id;
        }
        file.write((char *)chefs, sizeof(int) * NUM_CHEFS);
        file.write((char *)tools, sizeof(Tool) * NUM_CHEFS);
        file.write((char *)recipe, sizeof(int) * NUM_CHEFS * DISH_PER_CHEF);
    }
    States *deserialize(std::ifstream &file) {
        States *state = new States();
        int chefs[NUM_CHEFS];
        Tool tools[NUM_CHEFS];
        int recipe[DISH_PER_CHEF * NUM_CHEFS];
        file.read((char *)chefs, sizeof(int) * NUM_CHEFS);
        file.read((char *)tools, sizeof(Tool) * NUM_CHEFS);
        file.read((char *)recipe, sizeof(int) * NUM_CHEFS * DISH_PER_CHEF);
        if (file.fail()) {
            delete state;
            return NULL;
        }
        for (int i = 0; i < NUM_CHEFS; i++) {
            state->setChef(i, *cl->byId(chefs[i]));
            state->modifyTool(i, tools[i]);
        }
        for (int i = 0; i < NUM_CHEFS * DISH_PER_CHEF; i++) {
            if (recipe[i] != -1) {
                state->recipe[i] = rl->byId(recipe[i]);
            }
        }
        return state;
    }
};
class StatesRecorder {
    const int stateSize = sizeof(States);
    std::ofstream file;
    std::vector<States *> states;
    States **states_ptr;
    StatesSerializer serializer;
    std::size_t id;
    bool id_written = false;
    std::string filename;

    void writeHeader() {
        file = std::ofstream(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Warning: could not create states recorder" << filename
                      << std::endl;
        } else {
            file.write((char *)&id, sizeof(std::size_t));
        }
    }

  public:
    StatesRecorder(std::string filename, std::size_t id, CList *cl, RList *rl)
        : serializer(cl, rl) {
        std::ifstream inputFile(filename, std::ios::binary);

        // States state;
        // inputFile.read((char *)&state, stateSize);

        if (inputFile.is_open()) {
            std::size_t file_id;
            inputFile.read((char *)&file_id, sizeof(std::size_t));
            if (file_id == id) {
                try {
                    while (true) {
                        States *state = serializer.deserialize(inputFile);
                        if (state == NULL) {
                            break;
                        }
                        states.push_back(state);
                    }
                    std::cout << "从存档点" << id << "恢复" << states.size()
                              << "条记录。若要重新开始，请删除同一目录下states."
                                 "bin文件。"
                              << std::endl;
                } catch (std::exception &e) {
                    std::cerr << "Previous checkpoint found, but file is "
                                 "corropted. Error : "
                              << e.what() << std::endl;
                }
            }
        }
        inputFile.close();
        this->id = id;
        this->filename = filename;
    }
    ~StatesRecorder() {
        if (file.is_open()) {
            file.close();
        }
        for (auto &state : states) {
            delete state;
        }
        delete[] states_ptr;
    }

    States **get_states(size_t i) {
        states_ptr = new States *[i];
        if (i > states.size()) {
            for (size_t j = 0; j < states.size(); j++) {
                states_ptr[j] = states[j];
            }
            for (size_t j = states.size(); j < i; j++) {
                states_ptr[j] = NULL;
            }
        } else {
            for (size_t j = 0; j < i; j++) {
                states_ptr[j] = states[j];
            }
        }
        return states_ptr;
    }
    void add_state(States *states) {
        if (!id_written) {
            writeHeader();
            id_written = true;
        }
        if (file.is_open()) {
            serializer.serialize(file, states);
        }
    }
};

#endif