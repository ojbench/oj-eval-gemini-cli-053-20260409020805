#pragma once
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>
// You are NOT allowed to add any headers
// without the permission of TAs.
namespace Grammar {
class NFA;
NFA MakeStar(const char &character);
NFA MakePlus(const char &character);
NFA MakeQuestion(const char &character);
NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
NFA Union(const NFA &nfa1, const NFA &nfa2);
NFA MakeSimple(const char &character);
/*!
  \brief This class is used to store the type of the transition.
  \details You can use it like this:
  \code
      TransitionType type = TransitionType::Epsilon;
      TransitionType type2 = TransitionType::a;
      TransitionType type3 = TransitionType::b;
  \endcode
*/
enum class TransitionType { Epsilon, a, b };
struct Transition {
  /*!
    \brief value is used to store which character to match.
  */
  TransitionType type;
  /*!
    \brief This is used to store which node the transition is going to.
  */
  int to;
  Transition(TransitionType type, int to) : type(type), to(to) {}
};
class NFA {
private:
  /*!
    \brief This is used to store the start state of the NFA.
  */
  int start;
  /*!
    \brief This is used to store the end state of the NFA.
  */
  std::unordered_set<int> ends;
  /*!
    \brief This is used to store the transitions of the NFA.
    \details For example, transitions[3] stores all the transitions beginning
    with \details state 3.
  */
  std::vector<std::vector<Transition>> transitions;

public:
  NFA() = default;
  ~NFA() = default;
  /*!
    \brief Get the epsilon closure of a state.
    \param state The state to get the epsilon closure of.
    \return The epsilon closure of the state.
*/
  std::unordered_set<int>
  GetEpsilonClosure(std::unordered_set<int> states) const {
    std::unordered_set<int> closure;
    std::queue<int> queue;
    for (const auto &state : states) {
      if (closure.find(state) != closure.end())
        continue;
      queue.push(state);
      closure.insert(state);
    }
    while (!queue.empty()) {
      int current = queue.front();
      queue.pop();
      for (const auto &transition : transitions[current]) {
        if (transition.type == TransitionType::Epsilon) {
          if (closure.find(transition.to) == closure.end()) {
            queue.push(transition.to);
            closure.insert(transition.to);
          }
        }
      }
    }
    return closure;
  }
  /*!
    \brief Advance the NFA to the next states.
    \param current_states The current states of the NFA.
    \param character The character to match.
    \return The next states of the NFA.
  */
  std::unordered_set<int> Advance(std::unordered_set<int> current_states,
                                  char character) const {
    std::unordered_set<int> closure = GetEpsilonClosure(current_states);
    std::unordered_set<int> next_states;
    TransitionType target_type = (character == 'a') ? TransitionType::a : TransitionType::b;
    for (int state : closure) {
      for (const auto &transition : transitions[state]) {
        if (transition.type == target_type) {
          next_states.insert(transition.to);
        }
      }
    }
    return GetEpsilonClosure(next_states);
  }

  /*!
    \brief Check if a state is accepted.
    \param state The state to check.
    \return true if the state is accepted, false otherwise.
  */
  bool IsAccepted(int state) const { return ends.find(state) != ends.end(); }

  /*!
    \brief Return the start state.
    \return The start state.
  */

  int GetStart() const { return start; }

  friend NFA MakeStar(const char &character);
  friend NFA MakePlus(const char &character);
  friend NFA MakeQuestion(const char &character);
  friend NFA MakeSimple(const char &character);
  friend NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
  friend NFA Union(const NFA &nfa1, const NFA &nfa2);
};
class RegexChecker {
private:
  /*!
    \brief This is used to store the regex string.
  */
  NFA nfa;

public:
  /*!
    \brief To check if the string is accepted by the regex.
    \param str The string to be checked.
    \return true if the string is accepted by the regex, false otherwise.
  */
  bool Check(const std::string &str) const {
    std::unordered_set<int> current_states;
    current_states.insert(nfa.GetStart());
    for (char c : str) {
      current_states = nfa.Advance(current_states, c);
    }
    for (int state : current_states) {
      if (nfa.IsAccepted(state)) {
        return true;
      }
    }
    return false;
  }
  /*!
    \brief This is used to build the NFA from the regex string.
  */
  RegexChecker(const std::string &regex) {
    std::vector<std::string> parts;
    std::string current_part = "";
    for (char c : regex) {
      if (c == '|') {
        parts.push_back(current_part);
        current_part = "";
      } else {
        current_part += c;
      }
    }
    parts.push_back(current_part);

    auto parse_part = [](const std::string &part) -> NFA {
      std::vector<NFA> items;
      for (size_t i = 0; i < part.length(); i++) {
        char c = part[i];
        if (c == 'a' || c == 'b') {
          if (i + 1 < part.length()) {
            char next_c = part[i + 1];
            if (next_c == '+') {
              items.push_back(MakePlus(c));
              i++;
            } else if (next_c == '*') {
              items.push_back(MakeStar(c));
              i++;
            } else if (next_c == '?') {
              items.push_back(MakeQuestion(c));
              i++;
            } else {
              items.push_back(MakeSimple(c));
            }
          } else {
            items.push_back(MakeSimple(c));
          }
        }
      }
      NFA res = items[0];
      for (size_t i = 1; i < items.size(); i++) {
        res = Concatenate(res, items[i]);
      }
      return res;
    };

    if (!parts.empty()) {
      nfa = parse_part(parts[0]);
      for (size_t i = 1; i < parts.size(); i++) {
        nfa = Union(nfa, parse_part(parts[i]));
      }
    }
  }
};

/*!
  \brief Return a NFA to match a* or b*.
  \param character The character to match.
  \details This function will create a NFA that matches a* or b*.
*/
NFA MakeStar(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(0);
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 0});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 0});
  }
  return nfa;
}
/*!
  \brief Return a NFA to match a+ or b+.
  \param character The character to match.
  \details This function will create a NFA that matches a+ or b+.
*/
NFA MakePlus(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.push_back(std::vector<Transition>());
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }
  nfa.transitions[1].push_back({TransitionType::Epsilon, 0});
  return nfa;
}
/*!
  \brief Return a NFA to match a? or b?.
  \param character The character to match.
  \details This function will create a NFA that matches a? or b?.
*/
NFA MakeQuestion(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.push_back(std::vector<Transition>());
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }
  nfa.transitions[0].push_back({TransitionType::Epsilon, 1});
  return nfa;
}

/*!
  \brief Return a NFA to match ...(the first part) ...(the second part).
  \param nfa1 The first NFA to match, for example, it is a.
  \param nfa2 The second NFA to match, for example, it is b.
  \details This function will create a NFA that matches ab.
*/
NFA Concatenate(const NFA &nfa1, const NFA &nfa2) {
  NFA nfa;
  nfa.start = nfa1.start;
  int offset = nfa1.transitions.size();
  nfa.transitions = nfa1.transitions;
  for (const auto &t : nfa2.transitions) {
    std::vector<Transition> new_t;
    for (const auto &tr : t) {
      new_t.push_back({tr.type, tr.to + offset});
    }
    nfa.transitions.push_back(new_t);
  }
  for (int end_state : nfa1.ends) {
    nfa.transitions[end_state].push_back({TransitionType::Epsilon, nfa2.start + offset});
  }
  for (int end_state : nfa2.ends) {
    nfa.ends.insert(end_state + offset);
  }
  return nfa;
}
/*!
  \brief Return a NFA to match ... | ...
  \param nfa1 The first NFA to match, for example, it is a.
  \param nfa2 The second NFA to match, for example, it is b.
  \details This function will create a NFA that matches a|b.
*/
NFA Union(const NFA &nfa1, const NFA &nfa2) {
  NFA nfa;
  nfa.start = 0;
  nfa.transitions.push_back(std::vector<Transition>());
  int offset1 = 1;
  for (const auto &t : nfa1.transitions) {
    std::vector<Transition> new_t;
    for (const auto &tr : t) {
      new_t.push_back({tr.type, tr.to + offset1});
    }
    nfa.transitions.push_back(new_t);
  }
  int offset2 = 1 + nfa1.transitions.size();
  for (const auto &t : nfa2.transitions) {
    std::vector<Transition> new_t;
    for (const auto &tr : t) {
      new_t.push_back({tr.type, tr.to + offset2});
    }
    nfa.transitions.push_back(new_t);
  }
  nfa.transitions[0].push_back({TransitionType::Epsilon, nfa1.start + offset1});
  nfa.transitions[0].push_back({TransitionType::Epsilon, nfa2.start + offset2});
  for (int end_state : nfa1.ends) {
    nfa.ends.insert(end_state + offset1);
  }
  for (int end_state : nfa2.ends) {
    nfa.ends.insert(end_state + offset2);
  }
  return nfa;
}

/*!
  \brief Return a NFA to match a, or return a NFA to match b.
  \param character The character to match.
  \return The required NFA.
*/
NFA MakeSimple(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.push_back(std::vector<Transition>());
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }
  return nfa;
}
} // namespace Grammar
