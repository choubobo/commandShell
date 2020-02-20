//type in a "fdsg", the return NULL will be executed
//type in set, i will get Null POINTER
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>
using namespace std;

class Shell {
 public:
  int isSet = 0;
  int isEOF = 0;
  //decide whether the command name is set
  void assignIsSet(vector<string> inputList) {
    if (inputList.size() == 0) {
      isSet = 0;
    }
    else if (inputList.size() == 1) {
      isSet = 1;
      cerr << "miss one input argument" << endl;
    }
    else {
      isSet = 1;
    }
  }
  //parse the input command
  vector<string> parse() {
    vector<string> inputList;
    int c;
    queue<char> q;
    string s;
    int loopNum = 0;
    int set = 0;
    while ((c = getchar())) {
      if (c == EOF) {
        isEOF = 1;
        return vector<string>();
      }
      //if command name is set
      if (loopNum == 0 && c == 's') {
        set++;
      }
      if (loopNum == 1 && c == 'e') {
        set++;
      }
      if (loopNum == 2 && c == 't' && set == 2) {
        inputList = parse_setVar();
        assignIsSet(inputList);
        break;
      }

      //first situation
      if (c == '\\') {
        int c1;
        if ((c1 = getchar()) == '\n') {
          break;
        }
        else {
          q.push(c1);
        }
      }

      //second situation
      else if (c == '"') {
        int c2;
        while ((c2 = getchar()) != '"') {
          if (c2 == '\n') {
            cerr << "The input path does not have correspoding qutation mark " << endl;
            return vector<string>();
          }
          else if (c2 == '\\') {
            int c3 = getchar();
            q.push(c3);
          }
          else {
            q.push(c2);
          }
        }
        while (!q.empty()) {
          s.push_back(q.front());
          q.pop();
        }
      }

      //third situation
      else if (c == ' ' || c == '\n') {
        while (!q.empty()) {
          s.push_back(q.front());
          q.pop();
        }
      }

      //fourth situation
      else {
        q.push(c);
      }
      if (q.empty() && s.length() != 0) {
        inputList.push_back(s);
        s.clear();
      }

      if (c == '\n' && loopNum == 0) {
        return vector<string>();
      }
      else if (c == '\n') {
        break;
      }
      loopNum++;
    }
    return inputList;
  }

  //split environment variable according to :
  vector<string> splitEnv(vector<string> ECE551PATH) {
    string delimiter = ":";
    size_t pos = 0;
    vector<string> envVec;
    for (size_t i = 0; i < ECE551PATH.size(); i++) {
      while ((pos = ECE551PATH[i].find(delimiter)) != string::npos) {
        envVec.push_back(ECE551PATH[i].substr(0, pos));
        ECE551PATH[i].erase(0, pos + 1);
      }
      envVec.push_back(ECE551PATH[i]);
    }
    return envVec;
  }

  //search specific envrioment by add "/" and filename to see if there is
  string searchEnv(vector<string> strArray, vector<string> ECE551PATH) {
    vector<string> envPath = splitEnv(ECE551PATH);
    string command = strArray[0];
    int isAbs = isAbsolute(strArray);

    // if input is absolute path
    if (isAbs == 0) {
      ifstream ifs;
      ifs.open(strArray[0]);
      if (!ifs.fail()) {
        ifs.close();
        return strArray[0];
      }
      ifs.close();
    }
    //input is relative path
    else {
      for (unsigned i = 0; i < envPath.size(); i++) {
        envPath[i].push_back('/');
        envPath[i].append(command);
        ifstream ifs;
        ifs.open(envPath[i]);
        if (!ifs.fail()) {
          ifs.close();
          return envPath[i];
        }
        ifs.clear();
        ifs.close();
      }
    }
    return "invalidCommand";
  }

  int isAbsolute(vector<string> strArray) {
    string s1 = strArray[0];
    if (s1.front() == '/' || s1[0] == '.' && s1[1] == '/' ||
        s1[0] == '.' && s1[1] == '.' && s1[2] == '/') {
      return 0;
    }
    else {
      return 1;
    }
  }

  void run(vector<string> strArray, vector<string> ECE551PATH) {
    int cpid = fork();
    int wstatus;
    //error in forking
    if (cpid == -1) {
      perror("fork");
      return;
    }

    //code executed by child
    if (cpid == 0) {
      vector<char *> vec;

      //  transform string to char *
      for (unsigned i = 0; i < strArray.size(); ++i) {
        char * arg = const_cast<char *>(strArray[i].c_str());
        vec.push_back(arg);
      }

      char * argv[vec.size()];
      for (unsigned i = 0; i < strArray.size(); ++i) {
        argv[i] = vec[i];
      }

      argv[vec.size()] = NULL;

      //when the $ going to use is not in the map
      // if (notFound == 1) {
      //   notFound = 0;
      //   cout << "notFound in run: " << notFound << endl;
      //   exit(EXIT_FAILURE);
      // }

      if (strArray[0] == "." || strArray[0] == "..") {
        cout << "Command " << strArray[0] << " not found" << endl;
        exit(EXIT_FAILURE);
      }
      else if (strArray[0] != "cd" && isSet == 0 && strArray[0] != "export" &&
               strArray[0] != "rev") {
        string searchRes = searchEnv(strArray, ECE551PATH);
        argv[0] = const_cast<char *>(searchRes.c_str());
        if (searchRes == "invalidCommand") {
          cout << "Command " << strArray[0] << " not found" << endl;
          exit(EXIT_FAILURE);
        }
        else {
          char * path[ECE551PATH.size() + 1];
          path[ECE551PATH.size()] = NULL;
          for (size_t i = 0; i < ECE551PATH.size(); i++) {
            path[i] = const_cast<char *>(ECE551PATH[i].c_str());
          }
          execve(argv[0], argv, path);
        }
      }
    }

    //Code Executed by parent
    else {
      do {
        pid_t w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
        if (w == -1) {
          perror("waitpid");
          return;
        }
        //   cout << "WIFEXITED(wstatus): " << WIFEXITED(wstatus) << endl;
        if (WIFEXITED(wstatus)) {
          if (WEXITSTATUS(wstatus) == 0) {
            printf("Program was successful\n");
          }
          else {
            printf("Program failed with code %d\n", WEXITSTATUS(wstatus));
          }
        }
        else if (WIFSIGNALED(wstatus)) {
          printf("Terminated by signal %d\n", WTERMSIG(wstatus));
        }

      } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    }
  }

  //parse function for set command
  vector<string> parse_setVar() {
    vector<string> vec;
    queue<char> q;
    string s;
    int c = 0;
    int i = 0;
    int loopNum = 0;
    // cout << "getchar() is " << getchar() << endl;
    while ((c = getchar()) != EOF) {
      // cout << "current c is " << c << endl;
      if (c == '\n' && loopNum == 0) {
        isSet = 0;
        cerr << "set command need two argument" << endl;
        break;
      }
      if (!q.empty() && c == ' ' && i == 0) {
        while (!q.empty()) {
          s.push_back(q.front());
          q.pop();
        }
        vec.push_back(s);

        s.clear();
        i++;
      }
      else if (!q.empty() && c == '\n') {
        //      cout << "else if is executed" << endl;

        while (!q.empty()) {
          s.push_back(q.front());
          q.pop();
        }
        vec.push_back(s);
        s.clear();
        i++;
        break;
      }
      else if (c == ' ' && i == 0) {
        continue;
      }
      else {
        q.push(c);
      }
      loopNum++;
    }
    return vec;
  }
};
