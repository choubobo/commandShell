#include "ffosh.hpp"
bool checkCd(char * currdir, string input) {
  string temp = currdir;
  temp += "/" + input;
  char * directory = const_cast<char *>(temp.c_str());
  // cout << "directory is" << directory << endl;
  if (chdir(directory) != 0) {
    return false;
  }
  return true;
}

bool isValidVar(string s) {
  for (string::iterator it = s.begin(); it != s.end(); it++) {
    if (!isalpha(*it) && !isdigit(*it) && (*it) != '_') {
      cerr << "Variable name must be a combination of letters, numbers and underscore"
           << endl;
      return false;
    }
  }
  return true;
}
int main(void) {
  Shell sh;
  int notFound = 0;
  map<string, string> varMap;
  //exit when detect "exit" or EOF
  char * currdir = get_current_dir_name();
  vector<string> ECE551PATH;
  string path = getenv("PATH");
  path.insert(0, "ECE551PATH=");
  ECE551PATH.push_back(path);

  sh.splitEnv(ECE551PATH);

  while (true) {
    cout << "ffosh:" << currdir << "$";
    vector<string> input = sh.parse();

    //if no input or set has no argument
    if (input.size() == 0 && sh.isEOF == 1) {
      sh.isEOF = 0;
      break;
    }
    else if (input.size() == 0) {
      continue;
    }
    if (input[0].compare("exit") == 0) {
      break;
    }

    //when there is only one argument for set
    if (sh.isSet == 1 && input.size() == 1) {
      continue;
    }

    //siutation for cd
    if (input[0] == "cd") {
      if (input.size() == 1) {
        cerr << "Need to specify directory." << endl;
      }
      else if (input.size() > 2) {
        cerr << "Too many arguments" << endl;
      }
      else if (input[1][0] == '/') {
        chdir(const_cast<char *>(input[1].c_str()));
        currdir = get_current_dir_name();
      }
      else if (checkCd(currdir, input[1])) {
        free(currdir);
        currdir = get_current_dir_name();
      }
      else {
        cerr << "Directory does not exist." << endl;
        continue;
      }
    }

    //parse again for input to see if there is $
    if (sh.isSet == 0) {
      for (vector<string>::iterator it = input.begin(); it != input.end(); it++) {
        size_t found = (*it).find('$');

        if (found != std::string::npos) {
          string var;
          string s;
          size_t i = 0;
          while (i < (*it).length()) {
            if ((*it)[i] == '$') {
              while (i != (*it).length()) {
                i++;
                if (isalpha((*it)[i]) || isdigit((*it)[i]) || (*it)[i] == '_') {
                  var.push_back((*it)[i]);
                }

                else if ((*it)[i] == '$') {
                  if (!var.empty()) {
                    map<string, string>::iterator itmap = varMap.find(var);
                    if (itmap == varMap.end()) {
                      notFound = 1;
                      break;
                    }
                    else {
                      s.append(varMap.find(var)->second);
                      var.clear();
                    }
                  }
                  break;
                }

                else {
                  if (!var.empty()) {
                    map<string, string>::iterator itmap = varMap.find(var);
                    if (itmap == varMap.end()) {
                      notFound = 1;
                      break;
                    }
                    else {
                      s.append(varMap.find(var)->second);
                      var.clear();
                    }
                  }
                  s.push_back(((*it)[i]));
                }
              }
            }
            else {
              s.push_back((*it)[i]);
              i++;
            }
            if (notFound == 1)
              break;
          }

          if (!var.empty() && notFound != 1) {
            map<string, string>::iterator itmap = varMap.find(var);
            if (itmap == varMap.end()) {
              break;
            }
            else {
              s.append(varMap.find(var)->second);
              var.clear();
            }
          }
          if (notFound != 1)
            *it = s;
        }
        if (notFound == 1)
          break;
      }
    }
    //if the name behind $ is not in the map
    if (notFound == 1) {
      cerr << "variable name not found." << endl;
      notFound = 0;
      continue;
    }
    //when command name is export
    if (input[0] == "export") {
      if (input.size() == 1) {
        cerr << "the other argument for export is needed" << endl;
        continue;
      }
      else if (varMap.find(input[1]) == varMap.end()) {
        cerr << "the variable is not in the list" << endl;
        continue;
      }
      else {
        ECE551PATH.push_back(input[1] + "=" + varMap.find(input[1])->second);
      }
    }
    //when command name is rev
    if (input[0] == "rev") {
      if (input.size() == 1) {
        cerr << "the other argument for reverse is needed" << endl;
        continue;
      }
      else if (varMap.find(input[1]) == varMap.end()) {
        cerr << "the variable is not in the list" << endl;
        continue;
      }
      else {
        reverse(varMap.find(input[1])->second.begin(),
                varMap.find(input[1])->second.end());
      }
    }
    //when command name is set
    if (sh.isSet == 1) {
      bool isValid = isValidVar(input[0]);
      if (isValid == 0) {
        continue;
      }
      else if (varMap.find(input[0]) != varMap.end()) {
        varMap.erase(input[0]);
        varMap.emplace(input[0], input[1]);
        sh.isSet = 0;
      }
      else {
        varMap.emplace(input[0], input[1]);
        sh.isSet = 0;
      }
    }
    else if (input[0] != "cd" && input[0] != "export" && input[0] != "rev") {
      sh.run(input, ECE551PATH);
    }
  }
  free(currdir);
}
