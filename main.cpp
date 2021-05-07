/**
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
    /* first - state 'from', second - state 'to', third - transition */
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
        if (getline(input, buffer)) throw logic_error("E0");

        /* Check connectivity of FSA */
        try {
            dfs(states, trans);
        } catch (logic_error) {
            throw logic_error("E2");
        }

        for (size_t i = 0; i < trans.size(); ++i) {
            bool flag = false;

            /* Check for edges with same transitions from state */
            for (size_t j = i + 1; j < trans.size(); ++j) {
                if (trans[i].first == trans[j].first
                        && trans[i].third == trans[j].third) {
                    throw logic_error("E5");
                }
            }

            /* Check for presence of transitions in alphabet */
            for (size_t j = 0; j < alpha.size(); ++j) {
                if (trans[i].third == alpha[j]) flag = true;
            }

            if (!flag) throw logic_error("E3" + trans[i].third);
        }

        /* Check that every state from transition table is present in set of states */
        for (size_t i = 0; i < trans.size(); ++i) {
            if (!belongs(states, trans[i].first)) {
                throw logic_error("E1" + trans[i].first);
            } else if (!belongs(states, trans[i].second)) {
                throw logic_error("E1" + trans[i].second);
            }
        }

        /* Check that every state from accepting set is present in set of states*/
        for (size_t i = 0; i < accepting.size(); ++i) {
            if (!belongs(states, accepting[i])) throw logic_error("E1" + accepting[i]);
        }

        /* Check that initial state is defined*/
        if (initial.empty()) {
            throw logic_error("E4");
        } else if (!belongs(states, initial[0])) throw logic_error("E1" + initial[0]);


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
    } catch (const logic_error& error) {
        string exception_message = error.what(), error_code = exception_message.substr(0, 2), argument;
        output << "Error:\n";
        
        if (error_code == "E0") {
            output << "E0: Input file is malformed";
        }
        if (error_code == "E1") {
            argument = exception_message.substr(2);
            output << "E1: A state " << argument << " is not in set of states";
        }
        if (error_code == "E2") {
            output << "E2: Some states are disjoint";
        } 
        if (error_code == "E3") {
            argument = exception_message.substr(2);
            output << "E3: A transition " << argument << " is not represented in the alphabet";  
        }
        if (error_code == "E4") {
            output << "E4: Initial state is not defined";
        }
        if (error_code == "E5") {
            output << "E5: FSA is nondeterministic";
        }
    }

    output.close();
    input.close();

    return 0;
}

/**
 * @brief Splits string into set of words
 *
 * @param str - input string
 * 
 * @returns set of words
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

/** 
 * @brief Splits string into set of transitions
 *
 * @param str - input string
 * 
 * @returns set of transitions
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

/** 
 * @brief Performs Depth First Search
 *
 * @param states - reference to set of states
 * @param trans  - reference to set of transitions
 * 
 * @throws std::logic_error Thrown if FSA is not a connected graph
 */
void dfs(vector<string>& states, vector<triple>& trans) {
    vector<bool> visited (states.size(), false); 
    for (size_t i = 0; i < states.size(); ++i) {
        visited[i] = true;
        dfsVisit(visited, states[i], states, trans);
        break;
    }
    for (size_t i = 0; i < states.size(); ++i) {
        if (!visited[i]) throw std::logic_error("E2");
    }
}

/**
 * @brief Helper function for DFS
 * 
 * @relatesalso dfs
 *
 * @param visited - reference to set of boolean value
 * @param state   - current state
 * @param states  - reference to set of states
 * @param trans   - reference to set of transitions
 * 
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

/**
 * @brief Determines whether given element belongs to set
 *
 * @param set     - reference to some set
 * @param element - given element
 * 
 * @returns true, if elements belongs to set, false otherwise
 */
bool belongs(vector<string>& set, string element) {
    for (size_t i = 0; i < set.size(); ++i) {
        if (set[i] == element) return true;
    }
    return false;
}
