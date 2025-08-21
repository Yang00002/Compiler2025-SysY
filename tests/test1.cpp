#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_set>
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

// 定义一个函数来解析字符串并提取时间信息
std::optional<std::vector<int>> extractTime(const std::string &input) {
  std::regex pattern(R"(TOTAL:\s*(\d+)H-(\d+)M-(\d+)S-(\d+)us)");
  std::smatch match;

  if (std::regex_search(input, match, pattern) && match.size() > 4) {
    std::vector<int> timeValues;
    for (size_t i = 1; i < match.size(); ++i) {
      timeValues.push_back(std::stoi(match[i].str()));
    }
    return timeValues;
  }

  return std::nullopt;
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
  string shown = filePath;
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

int timeCmp(vector<int> &a, vector<int> &b) {
  long ta = a[0] * 3600000000 + a[1] * 60000000 + a[2] * 1000000 + a[3];
  long tb = b[0] * 3600000000 + b[1] * 60000000 + b[2] * 1000000 + b[3];
  long av = (ta + tb) >> 1;
  long dif = abs(ta - tb);
  if (av == 0)
    return 0;
  if (dif * 20 < av)
    return 0;
  if (ta < tb)
    return -1;
  if (ta > tb)
    return 1;
  return 0;
}

long long timeOf(vector<int> a) {
  long long ta = a[0] * 3600000000 + a[1] * 60000000 + a[2] * 1000000 + a[3];
  return ta;
}

int strictTimeCmp(vector<int> &a, vector<int> &b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] > b[i])
      return 1;
    if (a[i] < b[i])
      return -1;
  }
  return 0;
}

string time2Str(vector<int> &a) {
  return to_string(a[0]) + "H-" + to_string(a[1]) + "M-" + to_string(a[2]) +
         "S-" + to_string(a[3]) + "us";
}

vector<int> allTime = {0, 0, 0, 0};
vector<int> bestAllTime = {0, 0, 0, 0};

void updateAllTime(vector<int> &get) {
  allTime[3] += get[3];
  if (allTime[3] >= 1000000) {
    allTime[2] += allTime[3] / 1000000;
    allTime[3] %= 1000000;
  }
  allTime[2] += get[2];
  if (allTime[2] >= 60) {
    allTime[1] += allTime[2] / 60;
    allTime[2] %= 60;
  }
  allTime[1] += get[1];
  if (allTime[1] >= 60) {
    allTime[0] += allTime[1] / 60;
    allTime[1] %= 60;
  }
  allTime[0] += get[0];
}
void updateBestAllTime(vector<int> &get) {
  bestAllTime[3] += get[3];
  if (bestAllTime[3] >= 1000000) {
    bestAllTime[2] += bestAllTime[3] / 1000000;
    bestAllTime[3] %= 1000000;
  }
  bestAllTime[2] += get[2];
  if (bestAllTime[2] >= 60) {
    bestAllTime[1] += bestAllTime[2] / 60;
    bestAllTime[2] %= 60;
  }
  bestAllTime[1] += get[1];
  if (bestAllTime[1] >= 60) {
    bestAllTime[0] += bestAllTime[1] / 60;
    bestAllTime[1] %= 60;
  }
  bestAllTime[0] += get[0];
}
void showTime(vector<int> &get, string name, bool toExample) {
  string exampleTimePath = "./build/example/" + name + ".time";
  string customBestTimePath = "./build/custom/" + name + ".time";
  string customMidTimePath = "./build/custom/" + name + ".timid";
  string exampleTimeString =
      filesystem::exists(exampleTimePath) ? readFile(exampleTimePath) : "";
  string customBestTimeString = filesystem::exists(customBestTimePath)
                                    ? readFile(customBestTimePath)
                                    : "";
  auto exampleTimeOp = extractTime(exampleTimeString);
  auto customBestTimeOp = extractTime(customBestTimeString);
  string ret;
  if (exampleTimeOp) {
    int cmp = timeCmp(get, *exampleTimeOp);
    if (cmp < 0)
      ret += green(time2Str(get));
    else if (cmp > 0)
      ret += red(time2Str(get));
    else
      ret += yellow(time2Str(get));
    ret += " [ gcc ";
    ret += time2Str(*exampleTimeOp);
    ret += " ]";
  } else
    ret += time2Str(get);
  if (customBestTimeOp) {
    ret += " [ best ";
    int cmp = timeCmp(get, *customBestTimeOp);
    if (cmp < 0)
      ret += green(time2Str(*customBestTimeOp));
    else if (cmp > 0)
      ret += red(time2Str(*customBestTimeOp));
    else
      ret += yellow(time2Str(*customBestTimeOp));
    ret += "]";
    updateBestAllTime(*customBestTimeOp);
  } else {
    auto dv = vector{100, 0, 0, 0};
    updateBestAllTime(dv);
  }
  if (toExample)
    writeFile("TOTAL: " + time2Str(get), exampleTimePath);
  else {
    writeFile("TOTAL: " + time2Str(get), customMidTimePath);
    updateAllTime(get);
  }
  cout << ret;
}

void validate(const string &commonPathName, list<string> &fileList,
              bool shouldReturn, string shell) {
  makeDir("build/example/" + commonPathName);
  makeDir("build/custom/" + commonPathName);
  fileList.remove_if([](const string &v) -> bool { return !validateFile(v); });
  fileList.sort();
  if (filesystem::exists("build/example_diff/" + commonPathName))
    filesystem::remove_all("build/example_diff/" + commonPathName);
  for (auto &path : fileList) {
    string pathWithoutType = lastDotLeft(path);
    string name = last2LineRight(pathWithoutType);
    string input = pathWithoutType + ".in";
    string output = pathWithoutType + ".out";
    bool exp = shell == "compileTest.sh";
    string command = exp ? ("./build/example/" + name + ".out")
                         : ("./build/custom/" + name + ".out");
    bool haveIntput = filesystem::exists(input);
    if (haveIntput) {
      command += " < " + input;
    }
    if (shell != "compileTest.sh") {
      bool valid = validateFile(path, shell, false);
      if (!valid)
        continue;
    }
    auto out = runCommand(
        "qemu-aarch64 -L /usr/aarch64-linux-gnu -cpu cortex-a53 " + command,
        INT_MAX, shouldReturn);
    string compOut = out.first;
    string compMessage = out.second;
    optional<vector<int>> time = extractTime(compMessage);

    string defOut = readFile(output);
    compOut = normalizeString(compOut);
    defOut = normalizeString(defOut);
    if (compOut == defOut) {
      cout << path << green(" OK ");
      if (time) {
        showTime(*time, name, exp);
      }
      cout << endl;
    } else {
      cout << path << red(" 输出不一致") << endl;
      string pdif = "build/example_diff/" + name;
      makeDir(pdif);
      writeFile(compOut, pdif + "/comp_out.out");
      if (haveIntput)
        filesystem::copy_file(input, pdif + "/" + lastLineRight(input),
                              filesystem::copy_options::overwrite_existing);
      filesystem::copy_file(output, pdif + "/" + lastLineRight(output),
                            filesystem::copy_options::overwrite_existing);
      filesystem::copy_file(path, pdif + "/" + lastLineRight(path),
                            filesystem::copy_options::overwrite_existing);
    }
  }

  int bestBetter = strictTimeCmp(bestAllTime, allTime);
  string ret;
  ret += " [ best ";
  if (bestBetter > 0)
    ret += green(time2Str(allTime));
  else if (bestBetter < 0)
    ret += red(time2Str(allTime));
  else
    ret += yellow(time2Str(allTime));
  ret += "] / ";
  ret += time2Str(bestAllTime);
  cout << ret << '\n';
  if (bestBetter > 0) {
    auto fs =
        getTargetSuffixFilePaths("./build/custom/" + commonPathName, "time");
    for (auto i : fs)
      filesystem::remove(i);
    fs = getTargetSuffixFilePaths("./build/custom/" + commonPathName, "timid");

    for (auto i : fs)
      filesystem::rename(i, lastDotLeft(i) + ".time");
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

struct ele {
  long long dif;
  vector<int> t1;
  vector<int> t2;
  string name;
};

string formatTimeStr(long long str) {
  string s = to_string(str);
  while (s.size() != 10) {
    s = " " + s;
  }
  return s;
}

string formatTimeStr(double str) {
  string s = to_string(str);
  while (s.size() != 11) {
    s = " " + s;
  }
  return s;
}

string fastRate(long long l, long long r) {
  auto dif = ((r - l) * 1000) / r;
  string str;
  if (dif > 0)
    str = to_string(dif / 10) + "." + to_string(dif % 10) + "%";
  else {
    dif = -dif;
    str = "-" + to_string(dif / 10) + "." + to_string(dif % 10) + "%";
  }
  while (str.size() != 10) {
    str = " " + str;
  }
  return str;
}

int main(int argc, char *argv[]) {
  string path1 = "build/result/";
  string path2 = "build/result/";
  path1 += argv[1];
  path2 += argv[2];
  auto fils1 = getTargetSuffixFilePaths(path1, ".time");
  auto fils2 = getTargetSuffixFilePaths(path2, ".time");
  set<string> f2c;
  for (auto i : fils2)
    f2c.emplace(i);
  set<string> pths;
  for (auto i : fils1)
    pths.emplace(i);
  fils1.clear();
  for (auto i : pths)
    fils1.emplace_back(i);
  vector<ele> v;
  for (auto i : fils1) {
    auto j = string("build/result/") + argv[2] + "/" + lastLineRight(i);
    if (!f2c.count(j))
      continue;
    auto t1 = *extractTime(readFile(i));
    auto t2 = *extractTime(readFile(j));
    auto lt1 = timeOf(t1);
    auto lt2 = timeOf(t2);
    auto dif = lt1 - lt2;
    auto frac = (((lt2 - lt1) * 20) / lt2) != 0;
    if (frac)
      v.emplace_back(ele{lt2 - lt1, t1, t2,
                         "tests/arm/" + lastDotLeft(lastLineRight(i)) + ".sy"});
  }
  for (auto &i : v) {
    if (string(argv[argc - 1]) == "0")
      cout << formatTimeStr(i.dif / 1000000.0) << "\n";
    else if (string(argv[argc - 1]) == "1")
      cout << formatTimeStr(timeOf(i.t1) / 1000000.0) << "\n";
    else if (string(argv[argc - 1]) == "2")
      cout << formatTimeStr(timeOf(i.t2) / 1000000.0) << "\n";
    else if (string(argv[argc - 1]) == "3")
      cout << fastRate(timeOf(i.t1), timeOf(i.t2)) << "\n";
    else if (string(argv[argc - 1]) == "4")
      cout << i.name << "\n";
    else if (string(argv[argc - 1]) == "5")
      cout << lastDotLeft(lastLineRight(i.name)) << "\n";
    else
      cout << formatTimeStr(i.dif / 1000000.0) << "\t" << formatTimeStr(timeOf(i.t1) / 1000000.0)
           << "\t" << formatTimeStr(timeOf(i.t2) / 1000000.0) << "\t"
           << fastRate(timeOf(i.t1), timeOf(i.t2)) << "\t" << i.name << "\n";
  }
}