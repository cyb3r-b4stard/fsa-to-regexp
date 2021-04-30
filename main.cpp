/* 
 * @main
 * 
 * @brief This file converts given FSA to Regular Expression.
 * 
 * @author https://github.com/cyb3r-b4stard (Lomikovskiy Ivan).
 * 
 * @project FSA to RegExp Translator
 * 
 */
#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

struct triple {
    /* first - state 'from', second - state 'to', third - transition*/
    string first, second, third;
    triple(string _first, string _second, string _third) : 
            first(_first), second(_second), third(_third) {}
};

struct tensor {
    vector<vector<vector<string>>> _tensor;
    tensor(size_t size) {
        _tensor = vector<vector<vector<string>>> (size, vector<vector<string>> (size, vector<string> (size, "")));
    }
};

vector<string> preprocess            (string);
vector<triple> preprocessTransitions (string);
void           dfs                   (vector<string>&, vector<triple>&);
void           dfsVisit              (vector<bool>&, string, vector<string>&, vector<triple>&);
bool           belongs               (vector<string>&, string);


int main() {
    string          _states, _alpha, _initial, _accepting, _trans, buffer, answer;
    vector<string>  states, alpha, initial, accepting;
    size_t          initial_index;
    vector<triple>  trans;
    ifstream        input;
    ofstream        output;

    input.open("input.txt");
    output.open("output.txt");

    getline(input, _states);
    getline(input, _alpha);
    getline(input, _initial);
    getline(input, _accepting);
    getline(input, _trans);

    /* Preprocessing input */
    accepting = preprocess(_accepting);
    initial   = preprocess(_initial);
    states    = preprocess(_states);
    alpha     = preprocess(_alpha);
    trans     = preprocessTransitions(_trans);

    tensor regexp (states.size());
    vector<vector<string>> regexp_initial (states.size(), vector<string> (states.size(), ""));

    try {
        /* Check for excess lines in input file */
        if (getline(input, buffer)) {
            output << "Error:\nE0: Input file is malformed";
            throw logic_error("E0");
        }

        /* Check connectivity of FSA */
        try {
            dfs(states, trans);
        } catch (logic_error) {
            output << "Error:\nE2: Some states are disjoint";
            throw logic_error("E2");
        }

        for (size_t i = 0; i < trans.size(); ++i) {
            bool flag = false;

            /* Check for edges with same transitions from state */
            for (size_t j = i + 1; j < trans.size(); ++j) {
                if (trans[i].first == trans[j].first
                        && trans[i].third == trans[j].third) {
                    output << "Error:\nE5: FSA is nondeterministic";
                    throw logic_error("E5");
                }
            }

            /* Check for presence of transitions in alphabet */
            for (size_t j = 0; j < alpha.size(); ++j) {
                if (trans[i].third == alpha[j]) flag = true;
            }

            if (!flag) {
                output << "Error:\nE3: A transition " 
                    << trans[i].third 
                    << " is not represented in the alphabet";
                throw logic_error("E3");
            }
        }

        /* Check that every state from transition table is present in set of states */
        for (size_t i = 0; i < trans.size(); ++i) {
            if (!belongs(states, trans[i].first)) {
                output << "Error:\nE1: A state "
                    << trans[i].first
                    << " is not in set of states";
                throw logic_error("E1");
            } else if (!belongs(states, trans[i].second)) {
                output << "Error:\nE1: A state "
                    << trans[i].second
                    << " is not in set of states";
                throw logic_error("E1");
            }
        }

        /* Check that every state from accepting set is present in set of states*/
        for (size_t i = 0; i < accepting.size(); ++i) {
            if (!belongs(states, accepting[i])) {
                output << "Error:\nE1: A state "
                    << accepting[i]
                    << " is not in set of states";
                throw logic_error("E1");
            }
        }

        /* Check that initial state is defined*/
        if (initial.empty()) {
            output << "Error:\nE4: Initial state is not defined";
        } else {
            if (!belongs(states, initial[0])) {
                output << "Error:\nE1: A state " 
                    << initial[0] 
                    << " is not in set of states";
                throw logic_error("E1");
            }
        }


        /* Set up initial values of RegExp for k = -1 */
        for (size_t i = 0; i < trans.size(); ++i) {
            size_t from, to;
            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == trans[i].first) from = j;
            }
            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == trans[i].second) to = j;
            }
            if (!regexp_initial[from][to].empty()) regexp_initial[from][to] += "|" + trans[i].third;
            else                                   regexp_initial[from][to]  = trans[i].third;
        }
        for (size_t i = 0; i < states.size(); ++i) {
            if (regexp_initial[i][i].empty()) regexp_initial[i][i]  = "eps";
            else                              regexp_initial[i][i] += "|eps";

            for (size_t j = 0; j < states.size(); ++j) {
                if (i != j && regexp_initial[i][j].empty()) regexp_initial[i][j] = "{}";
            }
        }

        /* Calculate RegExp */
        for (size_t k = 0; k < states.size(); ++k) {
            for (size_t i = 0; i < states.size(); ++i) {
                for (size_t j = 0; j < states.size(); ++j) {
                    if (k > 0)
                        regexp._tensor[i][j][k] = "(" + regexp._tensor[i][k][k-1] + ")(" 
                                                  + regexp._tensor[k][k][k-1] + ")*(" 
                                                  + regexp._tensor[k][j][k-1] + ")|(" 
                                                  + regexp._tensor[i][j][k-1] + ")";
                    else
                        regexp._tensor[i][j][k] = "(" + regexp_initial[i][k] + ")(" 
                                                  + regexp_initial[k][k] + ")*(" 
                                                  + regexp_initial[k][j] + ")|(" 
                                                  + regexp_initial[i][j] + ")";
                }
            }
        }

        /* Find index of initial state in set of states*/
        for (size_t i = 0; i < states.size(); ++i) {
            if (states[i] == initial[0]) {
                initial_index = i;
                break;
            }
        }

        /* Translate FSA to Regular Expression */
        for (size_t i = 0; i < accepting.size(); ++i) {
            size_t index;
            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == accepting[i]) index = j;
            }
            if (answer.empty()) answer  = regexp._tensor[initial_index][index][states.size()-1];
            else                answer += "|" + regexp._tensor[initial_index][index][states.size()-1];
        }

        if (answer.empty()) answer = "{}";

        output << answer;
    } catch (logic_error) {}

    output.close();
    input.close();

    return 0;
}

/* 
 * Splits string into set of words
 *
 * @args str - input string
 * 
 * @return set of words
 */
vector<string> preprocess(string str) {
    vector<string> processed;
    string         buffer;
    bool           inside = false;

    for (size_t i = 0; i < str.size() - 1; ++i) {
        if (inside) {
            if (str[i] != ',') buffer.push_back(str[i]);
            else {
                processed.push_back(buffer);
                buffer.clear();
            }
        }
        if (str[i] == '[') {
            inside = true;
        }
    }
    if (!buffer.empty()) processed.push_back(buffer);

    return processed;
}

/* 
 * Splits string into set of transitions
 *
 * @args str - input string
 * 
 * @return set of transitions
 */
vector<triple> preprocessTransitions(string str) {
    vector<string> trans_split = preprocess(str);
    vector<triple> processed;
    string         from, to, weight, buffer;
    bool           first_word = true, second_word = false;

    for (string transition : trans_split) {
        for (size_t i = 0; i < transition.size(); ++i) {
            if (transition[i] == '>' & first_word & !second_word) {
                from = buffer;
                first_word = false;
                buffer.clear();
                continue;
            } else if (transition[i] == '>' & !first_word & !second_word) {
                weight = buffer;
                second_word = true;
                buffer.clear();
                continue;
            }
            buffer.push_back(transition[i]);
        }
        to = buffer;
        first_word = true;
        second_word = false;
        buffer.clear();
        processed.push_back(triple(from, to, weight));
    }
    return processed;
}

/* 
 * Performs Depth First Search
 *
 * @args states - reference to set of states
 *       trans  - reference to set of transitions
 * 
 */
void dfs(vector<string>& states, vector<triple>& trans) {
    vector<bool> visited (states.size(), false); 
    for (size_t i = 0; i < states.size(); ++i) {
        visited[i] = true;
        dfsVisit(visited, states[i], states, trans);
        break;
    }
    for (size_t i = 0; i < states.size(); ++i) {
        if (!visited[i]) throw logic_error("E2");
    }
}

/*
 * Helper function for DFS
 *
 * @args visited - reference to set of boolean value
 *       state   - current state
 *       states  - reference to set of states
 *       trans   - reference to set of transitions
 */
void dfsVisit(vector<bool>& visited, string state, vector<string>& states, vector<triple>& trans) {
    for (size_t i = 0; i < trans.size(); ++i) {
        if (trans[i].first == state) {
            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == trans[i].second && !visited[j]) {
                    visited[j] = true;
                    dfsVisit(visited, states[j], states, trans);
                }
            }
        }
    }
}

/*
 * Determines whether given element belongs to set
 *
 * @args element - given element
 *       set     - reference to some set
 * 
 * @return true, if elements belongs to set, false otherwise
 */
bool belongs(vector<string>& set, string element) {
    for (size_t i = 0; i < set.size(); ++i) {
        if (set[i] == element) return true;
    }
    return false;
}