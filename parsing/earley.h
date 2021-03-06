#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "mm/library.h"
#include "parser.h"

template< typename SymType, typename LabType >
struct EarleyState {
    size_t initial;
    size_t current;
    SymType type;
    LabType der_lab;
    const std::vector< SymType > *der;
    std::vector< std::pair< size_t, size_t > > children;

    bool operator==(const EarleyState &x) const {
        return this->initial == x.initial && this->current == x.current && this->type == x.type && this->der_lab == x.der_lab && this->der == x.der;
    }

    ParsingTree< SymType, LabType > get_tree(const std::vector< std::vector< EarleyState > > &state) const {
        ParsingTree< SymType, LabType > ret;
        ret.label = this->der_lab;
        ret.type = this->type;
        for (auto &child : this->children) {
            ret.children.push_back(state[child.first][child.second].get_tree(state));
        }
        return ret;
    }
};

// See http://loup-vaillant.fr/tutorials/earley-parsing/recogniser
// FIXME This version does not support empty derivations
template< typename SymType, typename LabType >
class EarleyParser : public Parser< SymType, LabType > {
public:
    EarleyParser(const std::unordered_map<SymType, std::vector<std::pair<LabType, std::vector<SymType> > > > &derivations) :
        derivations(derivations) {
    }

    using Parser< SymType, LabType >::parse;
    ParsingTree< SymType, LabType > parse(typename std::vector<SymType>::const_iterator sent_begin, typename std::vector<SymType>::const_iterator sent_end, SymType type) const {
        /* For every position (plus one end-of-string position) we maintain a list
         * of tuples whose elements are the initial position of that item, the position
         * of the current point and the associated derivation. */
        std::vector< std::vector< EarleyState< SymType, LabType > > > state;
        size_t sent_size = sent_end - sent_begin;
        state.resize(sent_size + 1);

        // The state is initialized with the derivations for the target type
        for (auto &der : this->derivations.at(type)) {
            state[0].push_back({ 0, 0, type, der.first, &der.second, {} });
        }

        // We use vector indices instead of iterators to avoid problems with reallocation
        for (size_t i = 0; i < state.size(); i++) {
            for (size_t j = 0; j < state[i].size(); j++) {

                if (state[i][j].current == state[i][j].der->size()) {
                    // If the item is finished, do the completion
                    for (size_t k = 0; k < state[state[i][j].initial].size(); k++) {
                        EarleyState< SymType, LabType > &cur_state2 = state[state[i][j].initial][k];
                        if (cur_state2.current < cur_state2.der->size() && (*cur_state2.der)[cur_state2.current] == state[i][j].type) {
                            EarleyState< SymType, LabType > new_item = { cur_state2.initial, cur_state2.current+1, cur_state2.type, cur_state2.der_lab, cur_state2.der, cur_state2.children };
                            new_item.children.push_back(std::make_pair(i, j));
                            if (find(state[i].begin(), state[i].end(), new_item) == state[i].end()) {
                                state[i].push_back(new_item);
                            }
                        }
                    }
                } else {
                    // Else search current symbol in derivations
                    EarleyState< SymType, LabType > &cur_state = state[i][j];
                    SymType symbol = (*cur_state.der)[cur_state.current];
                    auto it = this->derivations.find(symbol);
                    if (it != this->derivations.end()) {
                        // Current symbol is in the derivations, therefore non-terminal: prediction phase
                        // Add one new item for every derivation, unless the same item is already present
                        for (auto &new_der : it->second) {
                            EarleyState< SymType, LabType > new_item = { i, 0, symbol, new_der.first, &new_der.second, {} };
                            if (find(state[i].begin(), state[i].end(), new_item) == state[i].end()) {
                                state[i].push_back(new_item);
                            }
                        }
                    } else {
                        // Current symbol it not in the derivations, therefore is terminal: scan phase
                        // If the symbol matches the sentence, promote item to the new bucket
                        if (i < sent_size && symbol == sent_begin[i]) {
                            EarleyState< SymType, LabType > new_item = { cur_state.initial, cur_state.current+1, cur_state.type, cur_state.der_lab, cur_state.der, cur_state.children };
                            if (find(state[i+1].begin(), state[i+1].end(), new_item) == state[i+1].end()) {
                                state[i+1].push_back(new_item);
                            }
                        }
                    }
                }
            }
        }

    #if 0
        // Dump final status for debug
        std::cout << "Earley dump" << std::endl;
        for (size_t i = 0; i < state.size(); i++) {
            std::cout << "== " << i << " ==" << std::endl;
            for (size_t j = 0; j < state[i].size(); j++) {
                EarleyState< SymType, LabType > &cur_state = state[i][j];
                std::cout << cur_state.der_lab << " (" << cur_state.initial << "): " << cur_state.type << " -->";
                for (size_t k = 0; k < cur_state.der->size(); k++) {
                    if (k == cur_state.current) {
                        std::cout << " •";
                    }
                    std::cout << " " << (*cur_state.der)[k];
                }
                if (cur_state.der->size() == cur_state.current) {
                    std::cout << " •";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    #endif

        // Final phase: see if the sentence was accepted
        for (size_t j = 0; j < state[sent_size].size(); j++) {
            EarleyState< SymType, LabType > &cur_state = state[sent_size][j];
            if (cur_state.initial == 0 && cur_state.current == cur_state.der->size() && cur_state.type == type) {
                ParsingTree< SymType, LabType > tree = cur_state.get_tree(state);
                return tree;
            }
        }
        ParsingTree< SymType, LabType > ret;
        ret.label = {};
        return ret;
    }

private:
    const std::unordered_map<SymType, std::vector<std::pair<LabType, std::vector<SymType> > > > &derivations;
};
