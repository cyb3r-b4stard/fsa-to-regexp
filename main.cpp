/**
 * @main
 * 
 * @brief This program converts given FSA to Regular Expression.
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

struct triple 
{
    string from, to, transition;

    triple(string _from, string _to, string s_transition) 
        : from(_from), to(_to), transition(s_transition) 
    {}
};

struct tensor 
{
    vector<vector<vector<string>>> matrix;

    tensor(size_t size) 
        : matrix(vector<vector<vector<string>>> (size, vector<vector<string>> (size, vector<string> (size, "")))) 
    {}
};

vector<string> preprocess            (string);
vector<triple> preprocessTransitions (string);
void           dfs                   (vector<string>&, vector<triple>&);
void           dfsVisit              (vector<bool>&, string, vector<string>&, vector<triple>&);
bool           belongs               (vector<string>&, string);


int main() 
{
    vector<string>  states, alpha, initial, accepting;
    vector<triple>  trans;

    string          s_states, s_alpha, s_initial, s_accepting, s_trans, buffer, answer;
    size_t          initial_index;

    ifstream        input  ("input.txt");
    ofstream        output ("output.txt");

    getline(input, s_states);
    getline(input, s_alpha);
    getline(input, s_initial);
    getline(input, s_accepting);
    getline(input, s_trans);

    /* Preprocessing input */
    accepting = preprocess(s_accepting);
    initial   = preprocess(s_initial);
    states    = preprocess(s_states);
    alpha     = preprocess(s_alpha);
    trans     = preprocessTransitions(s_trans);

    tensor regexp (states.size());
    vector<vector<string>> regexp_initial (states.size(), vector<string> (states.size(), ""));

    try {
        /* Check for excess lines in input file */
        if (getline(input, buffer)) 
            throw logic_error("E0");

        /* Check connectivity of FSA */
        try {
            dfs(states, trans);
        } catch (logic_error & error) {
            throw;
        }

        for (size_t i = 0; i < trans.size(); ++i) {
            bool flag = false;

            /* Check for edges with same transitions from state */
            for (size_t j = i + 1; j < trans.size(); ++j) {
                if (trans[i].from == trans[j].from
                        && trans[i].transition == trans[j].transition) {
                    throw logic_error("E5");
                }
            }

            /* Check if transitions are present in the alphabet */
            for (size_t j = 0; j < alpha.size(); ++j) {
                if (trans[i].transition == alpha[j]) 
                    flag = true;
            }

            if (!flag) 
                throw logic_error("E3" + trans[i].transition);
        }

        /* Check that every state from transition table is present in the set of states */
        for (size_t i = 0; i < trans.size(); ++i) {
            if (!belongs(states, trans[i].from)) {
                throw logic_error("E1" + trans[i].from);
            } else if (!belongs(states, trans[i].to)) {
                throw logic_error("E1" + trans[i].to);
            }
        }

        /* Check that every state from accepting set is present in the set of states*/
        for (size_t i = 0; i < accepting.size(); ++i) {
            if (!belongs(states, accepting[i])) 
                throw logic_error("E1" + accepting[i]);
        }

        /* Check that initial state is defined*/
        if (initial.empty()) {
            throw logic_error("E4");
        } else if (!belongs(states, initial[0])) {
            throw logic_error("E1" + initial[0]);
        }


        /* Set up initial values of RegExp for k = -1 */
        for (size_t i = 0; i < trans.size(); ++i) {
            size_t from, to;

            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == trans[i].from) from = j;
            }

            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == trans[i].to) to = j;
            }

            if (!regexp_initial[from][to].empty()) 
                regexp_initial[from][to] += "|" + trans[i].transition;
            else                                   
                regexp_initial[from][to]  = trans[i].transition;
        }

        for (size_t i = 0; i < states.size(); ++i) {
            if (regexp_initial[i][i].empty()) 
                regexp_initial[i][i]  = "eps";
            else
                regexp_initial[i][i] += "|eps";

            for (size_t j = 0; j < states.size(); ++j) {
                if (i != j && regexp_initial[i][j].empty()) 
                    regexp_initial[i][j] = "{}";
            }
        }

        /* Calculate RegExp */
        for (size_t k = 0; k < states.size(); ++k) {
            for (size_t i = 0; i < states.size(); ++i) {
                for (size_t j = 0; j < states.size(); ++j) {
                    if (k > 0)
                        regexp.matrix[i][j][k] = "(" + regexp.matrix[i][k][k-1] + ")(" 
                                                  + regexp.matrix[k][k][k-1] + ")*(" 
                                                  + regexp.matrix[k][j][k-1] + ")|(" 
                                                  + regexp.matrix[i][j][k-1] + ")";
                    else
                        regexp.matrix[i][j][k] = "(" + regexp_initial[i][k] + ")(" 
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
                if (states[j] == accepting[i]) 
                    index = j;
            }

            if (answer.empty()) 
                answer  = regexp.matrix[initial_index][index][states.size() - 1];
            else                
                answer += "|" + regexp.matrix[initial_index][index][states.size() - 1];
        }

        if (answer.empty()) 
            answer = "{}";

        output << answer;
    } catch (const logic_error & error) {
        string exception_message (error.what()), 
               error_code        (exception_message.substr(0, 2)), 
               argument;

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
}

/**
 * @brief Splits string into set of words
 *
 * @param str - input string
 * 
 * @returns set of words
 */
vector<string> preprocess(string str) 
{
    vector<string> processed;
    string         buffer;
    bool           inside (false);

    for (size_t i = 0; i < str.size() - 1; ++i) {
        if (inside) {
            if (str[i] != ',') {
                buffer.push_back(str[i]);
            } else {
                processed.push_back(buffer);
                buffer.clear();
            }
        }
        if (str[i] == '[')
            inside = true;
    }
    if (!buffer.empty()) 
        processed.push_back(buffer);

    return processed;
}

/** 
 * @brief Splits string into set of transitions
 *
 * @param str - input string
 * 
 * @returns set of transitions
 */
vector<triple> preprocessTransitions(string str) 
{
    vector<string> trans_split (preprocess(str));
    vector<triple> processed;

    string         from, to, weight, buffer;

    bool           from_word   (true), 
                   second_word (false);

    for (const string & transition : trans_split) {
        for (size_t i = 0; i < transition.size(); ++i) {
            if (transition[i] == '>' && from_word && !second_word) {
                from = buffer;
                from_word = false;
                buffer.clear();
                continue;
            } else if (transition[i] == '>' && !from_word && !second_word) {
                weight = buffer;
                second_word = true;
                buffer.clear();
                continue;
            }
            buffer.push_back(transition[i]);
        }
        to = buffer;
        from_word = true;
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
 * @throws std::logic_error thrown if FSA is not a connected graph
 */
void dfs(vector<string> & states, vector<triple> & trans) 
{
    vector<bool> visited (states.size(), false); 

    visited[0] = true;
    dfsVisit(visited, states[0], states, trans);
    
    for (size_t i = 0; i < states.size(); ++i) {
        if (!visited[i]) throw logic_error("E2");
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
        if (trans[i].from == state) {
            for (size_t j = 0; j < states.size(); ++j) {
                if (states[j] == trans[i].to && !visited[j]) {
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
bool belongs(vector<string> & set, string element) {
    for (size_t i = 0; i < set.size(); ++i) {
        if (set[i] == element) 
            return true;
    }
    return false;
}
