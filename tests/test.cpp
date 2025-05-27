#include <climits>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

enum evaluate_result { SUCCESS, FAIL, PASS };

std::string firstSpaceLeft(const std::string &str) {
  size_t spacePos = str.find(' ');

  if (spacePos == std::string::npos) {
    return str;
  } else {
    return str.substr(0, spacePos);
  }
}

std::pair<std::string, std::string>
runCommand(std::string command, int limit = 1, bool shouldReturn = false) {
  int outpipe[2];
  int errpipe[2];
  if (pipe(outpipe) != 0 || pipe(errpipe) != 0) {
    cerr << "pipe() 调用失败!" << endl;
    exit(-1);
  }

  pid_t pid = fork();
  if (pid == -1) {
    cerr << "fork() 调用失败!" << endl;
    exit(-1);
  }

  if (pid == 0) {
    // 子进程
    close(outpipe[0]);
    close(errpipe[0]);
    dup2(outpipe[1], STDOUT_FILENO);
    dup2(errpipe[1], STDERR_FILENO);
    execlp("sh", "sh", "-c", command.c_str(), (char *)nullptr);
    perror("execlp");
    exit(0);
  } else {
    // 父进程
    close(outpipe[1]);
    close(errpipe[1]);

    char buffer[1024];
    std::string stdout_result;
    std::string stderr_result;

    int pre = limit;
    int bytes;
    while (limit > 0 && (bytes = read(outpipe[0], buffer, 1023)) > 0) {
      buffer[bytes] = 0;
      stdout_result += buffer;
      limit--;
    }
    close(outpipe[0]);

    limit = pre;

    while (limit > 0 && (bytes = read(errpipe[0], buffer, 1023)) > 0) {
      buffer[bytes] = 0;
      stderr_result += buffer;
      limit--;
    }
    close(errpipe[0]);

    int status;
    waitpid(pid, &status, 0);

    if (shouldReturn) {
      int exitCode = WEXITSTATUS(status);
      if (!stdout_result.empty() &&
          stdout_result[stdout_result.size() - 1] != '\n')
        stdout_result += '\n';
      stdout_result += std::to_string(exitCode);
    }

    return {stdout_result, stderr_result};
  }
}

std::string readFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    cerr << "无法打开文件 " + filename << endl;
    exit(-1);
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

void writeFile(const std::string &content, const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    cerr << "无法打开文件 " + filename << endl;
    exit(-1);
  }

  file << content;
  file.close();
}

bool endsWith(const string &str, const string &suffix) {
  if (str.length() >= suffix.length()) {
    return str.compare(str.length() - suffix.length(), suffix.length(),
                       suffix) == 0;
  }
  return false;
}

string yellow(const string &str) { return "\033[33;1m" + str + "\033[0m"; }

string red(const string &str) { return "\033[31;1m" + str + "\033[0m"; }

string green(const string &str) { return "\033[32;1m" + str + "\033[0m"; }

string lastDotLeft(const string &str) {
  size_t lastDotPos = str.find_last_of('.');
  if (lastDotPos == string::npos) {
    return str;
  } else {
    return str.substr(0, lastDotPos);
  }
}

string lastLineRight(const string &str) {
  size_t lastSlashPos = str.find_last_of('/');
  if (lastSlashPos == string::npos) {
    return str;
  } else {
    return str.substr(lastSlashPos + 1);
  }
}

string last2LineRight(const string &str) {
  size_t lastSlashPos = str.find_last_of('/');
  if (lastSlashPos == string::npos) {
    return str;
  }

  size_t secondLastSlashPos = str.rfind('/', lastSlashPos - 1);
  if (secondLastSlashPos == string::npos) {
    return str.substr(lastSlashPos + 1);
  }

  return str.substr(secondLastSlashPos + 1);
}

string dirName(const string &str) {
  if (str.empty())
    return {};
  if (str[str.size() - 1] == '/') {
    string s = {str.begin(), str.begin() + str.size() - 1};
    return lastLineRight(s);
  }
  return last2LineRight(str);
}

list<string> getTargetSuffixFilePaths(const string &directoryPath,
                                      const string &suffix) {
  if (!filesystem::exists(directoryPath) ||
      !filesystem::is_directory(directoryPath)) {
    cerr << "指定路径不存在或不是一个目录！" << endl;
    exit(-1);
  }

  list<string> l;

  for (const auto &entry : filesystem::directory_iterator(directoryPath)) {
    string path = entry.path().string();
    if (filesystem::is_regular_file(entry.status())) {
      if (endsWith(path, suffix))
        l.emplace_back(path);
    }
  }
  return l;
}

void makeDir(string dirPath) {
  if (filesystem::exists(dirPath))
    return;
  if (!filesystem::create_directories(dirPath)) {
    cerr << "目录 " + dirPath + " 创建失败！" << endl;
    exit(-1);
  }
}

void prepareLib() {
  if (!filesystem::exists("./build/lib/sylib.a") ||
      !filesystem::exists("./build/lib/sylib++.a")) {
    string ret = runCommand("./tests/compileLib.sh").second;
    if (!ret.empty()) {
      cerr << "依赖库编译失败:\n" << ret << endl;
      exit(-1);
    }
  }
  if (!filesystem::exists("./build/lib/sylib.a") ||
      !filesystem::exists("./build/lib/sylib++.a")) {
    cerr << "依赖库编译失败" << endl;
    exit(-1);
  }
}

bool validateFile(const string &filePath, string shell = "compileTest.sh",
                  bool example = true) {
  if (example)
    cout << "checking " + filePath << endl;
  string shown = last2LineRight(filePath);
  if (!filesystem::exists(filePath)) {
    cerr << shown + red(" 文件不存在") << endl;
    return false;
  }
  string namePath = lastDotLeft(filePath);
  string outPath = namePath + ".out";
  string inPath = namePath + ".in";
  if (!filesystem::exists(outPath)) {
    cerr << shown + red(" out 文件不存在") << endl;
    return false;
  }
  string name = last2LineRight(namePath);
  string compileOutPut =
      (example ? "./build/example/" : "./build/custom/") + name + ".out";
  if (example && filesystem::exists(compileOutPut))
    return true;
  string res =
      runCommand("./tests/" + shell + " " + filePath + " " + compileOutPut,
                 example ? 1 : INT_MAX)
          .second;
  if (!res.empty() || !filesystem::exists(compileOutPut)) {
    cout << shown + red(" 编译失败") << endl;
    cerr << res << endl;
    return false;
  }
  return true;
}

std::string normalizeString(const std::string &input) {
  std::string result;
  for (char c : input) {
    if (c == '\r')
      continue;
    if (c == '\n') {
      result += '\n';
    } else {
      result += c;
    }
  }
  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;
}

void validate(const string &commonPathName, list<string> &fileList,
              bool shouldReturn, string shell) {
  makeDir("build/example/" + commonPathName);
  makeDir("build/custom/" + commonPathName);
  fileList.remove_if([](const string &v) -> bool { return !validateFile(v); });
  if (filesystem::exists("build/example_diff/" + commonPathName))
    filesystem::remove_all("build/example_diff/" + commonPathName);
  for (auto &path : fileList) {
    string pathWithoutType = lastDotLeft(path);
    string name = last2LineRight(pathWithoutType);
    string input = pathWithoutType + ".in";
    string output = pathWithoutType + ".out";
    string command = "./build/example/" + name + ".out";
    bool haveIntput = filesystem::exists(input);
    if (haveIntput) {
      command += " < " + input;
    }
    if (shell != "compileTest.sh") {
      bool valid = validateFile(path, shell, false);
      if (!valid)
        continue;
    }
    string compOut = runCommand(command, INT_MAX, shouldReturn).first;
    string defOut = readFile(output);
    compOut = normalizeString(compOut);
    defOut = normalizeString(defOut);
    if (compOut == defOut) {
      cout << last2LineRight(path) << green(" OK") << endl;
    } else {
      cout << last2LineRight(path) << red(" 输出不一致") << endl;
      string pdif = "build/example_diff/" + name;
      makeDir(pdif);
      writeFile(compOut, pdif + "/g++out.out");
      writeFile(defOut, pdif + "/given.out");
      if (haveIntput)
        filesystem::copy_file(input, pdif + "/" + lastLineRight(input),
                              filesystem::copy_options::overwrite_existing);
      filesystem::copy_file(output, pdif + "/" + lastLineRight(output),
                            filesystem::copy_options::overwrite_existing);
      filesystem::copy_file(path, pdif + "/" + lastLineRight(path),
                            filesystem::copy_options::overwrite_existing);
    }
  }
}

void refresh() {
  if (filesystem::exists("build/lib"))
    filesystem::remove_all("build/lib");
  if (filesystem::exists("build/example_diff"))
    filesystem::remove_all("build/example_diff");
  if (filesystem::exists("build/example"))
    filesystem::remove_all("build/example");
  if (filesystem::exists("build/custom"))
    filesystem::remove_all("build/custom");
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 4) {
    cerr
        << R"(参数错误，如下列出参数，以 EBNF 范式表示，每行一个，顺序不能颠倒。
(functional|arm|自定义测试目录)[/case.sy]  对应功能和性能测试样例集，附加路径可以评测某一样例
[脚本名称.sh]  用于将测试用 .sy 文件构建成可以运行的 .out 文件的脚本，见 tests/compileTest.sh，它是默认的例子
[disableReturn]  该选项禁止脚本在编译得到的输出后面自动附加返回值，默认会附加返回值

如果你不按照以上参数，而是只加一个 -r 参数，那脚本会刷新所有缓存。

评测脚本将先检查 build/lib 是否有对应 sylib.a sylib++.a 静态链接库(C 和 C++ 库)，如果没有会重新生成。如果你想添加新的库，必须要按照 compileLib.sh 手动编译，因为检查库是否缺失是硬编码进脚本的。
随后它根据第一个参数，决定该选取哪一个测试样例。只给出目录，它将使用其中所有测试样例，它们以 .sy 结尾；否则使用指定的样例。
只有当样例合法，它才会进行测试，否则将跳过样例并给出警告。样例合法是指：
- 同一目录有对应的 .out 输出文件
- 如果同一目录还有对应的 .in 文件，它将将其作为输入，否则没有输入。如果程序要求输入而没有输入，脚本不能保证正常运行。
- 样例可以被以 C++ 编译成 .out 可执行程序，它将被存储在 build/example/样例路径 下。
你可以自己在 testcases 下定义新的目录，但请和 arm, functional 同级，请把目录添加进 .gitignore
当有样例后，脚本将调用指定的脚本(默认是 compileTest.sh)编译样例，作为你自己生成的代码进行测试。脚本应该存储在 tests 目录下，你不用也无法指定其目录。但所有代码都在项目根目录执行。
编译结果存储在 build/custom 目录下，和合法性检查时不一样，每次都会重新编译。注意，如果你使用默认脚本，还是放在 example 目录，并且除非编译好的文件删除不会重新编译。编译结果存储在 build/custom 目录下，和合法性检查时不一样，每次都会重新编译。注意，如果你使用默认脚本，还是放在 example 目录，并且除非编译好的文件删除不会重新编译。
随后脚本运行编译输出，得到的输出结果和 .out 比较。
如果指定脚本给输出后面附加返回值，它会将返回值加在输出后面，如果输出最后一个字符不是换行符，那它会起一行。
输出最后是否有换行符，使用哪种换行符，不影响评测结果。
如果一切正确，将会对指定样例显示 OK，否则显示编译错误，或输出不一致。
如果编译错误，会显示标准错误流 cerr 中内容，不会显示 cout，请不要输出到 cout。
如果输出不一致，会将相关的文件复制到 build/example_diff/测试样例 目录，该目录在每次评测都会删除，请不要将脚本生成的结果放到这里。)"
        << endl;
    exit(-1);
  }
  string path = argv[1];
  if (path == "-r") {
    refresh();
    return 0;
  }
  string doc = lastLineRight(path);
  path = "tests/" + path;
  if (!filesystem::exists(path)) {
    cerr << path + red("\t文件夹不存在") << endl;
    exit(-1);
  }
  bool shouldReturn = true;
  string shell = "compileTest.sh";
  if (argc > 2) {
    string arg = argv[2];
    if (arg == "disableReturn")
      shouldReturn = false;
    else {
      shell = arg;
      if (!filesystem::exists("tests/" + shell)) {
        cerr << "评测脚本 " << shell << " 不存在" << endl;
        exit(-1);
      }
      if (argc > 3 && (arg = argv[3]) == "disableReturn")
        shouldReturn = false;
    }
  }
  prepareLib();
  list<string> cases;
  if (doc != argv[1]) {
    if (!filesystem::exists(path)) {
      cerr << path << red(" 样例不存在") << endl;
      exit(-1);
    }
    cases.emplace_back(path);
  } else
    cases = getTargetSuffixFilePaths(path, ".sy");
  validate(argv[1], cases, shouldReturn, shell);
}