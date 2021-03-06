/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "process.h"
#include "base.h"
#include "util.h"
#include "async_func.h"
#include "text_color.h"

#include <pwd.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

static void swap_fd(const string &filename, FILE *fdesc) {
  FILE *f = fopen(filename.c_str(), "a");
  if (f == NULL || dup2(fileno(f), fileno(fdesc)) < 0) {
    if (f) fclose(f);
    _exit(-1);
  }
}

///////////////////////////////////////////////////////////////////////////////

class FileReader {
public:
  FileReader(FilePtr f, string &out) : m_f(f), m_out(out) {}
  void read() { readString(m_f.get(), m_out); }

  static void readString(FILE *f, string &out) {
    size_t nread = 0;
    const unsigned int BUFFER_SIZE = 1024;
    char buf[BUFFER_SIZE];
    while ((nread = fread(buf, 1, BUFFER_SIZE, f)) != 0) {
      out.append(buf, nread);
    }
  }

private:
  FilePtr m_f;
  string &m_out;
};

///////////////////////////////////////////////////////////////////////////////

// Cached process statics
std::string Process::HostName;
std::string Process::CurrentWorkingDirectory;

void Process::InitProcessStatics() {
  HostName = GetHostName();
  CurrentWorkingDirectory = GetCurrentDirectory();
}

bool Process::Exec(const char *path, const char *argv[], const char *in,
                   string &out, string *err /* = NULL */,
                   bool color /* = false */) {

  int fdin = 0; int fdout = 0; int fderr = 0;
  int pid = Exec(path, argv, &fdin, &fdout, &fderr);
  if (pid == 0) return false;

  {
    FilePtr sin(fdopen(fdin, "w"), file_closer());
    if (!sin) return false;
    if (in && *in) {
      fwrite(in, 1, strlen(in), sin.get());
    }
  }

  char buffer[4096];
  if (fcntl(fdout, F_SETFL, O_NONBLOCK)) {
    perror("fcntl failed on fdout");
  }

  if (fcntl(fderr, F_SETFL, O_NONBLOCK)) {
    perror("fcntl failed on fderr");
  }

  while (fdout || fderr) {
    pollfd fds[2];
    int n = 0;
    if (fdout) {
      fds[n].fd = fdout;
      fds[n].events = POLLIN | POLLHUP;
      n++;
    }
    if (fderr) {
      fds[n].fd = fderr;
      fds[n].events = POLLIN | POLLHUP;
      n++;
    }

    n = poll(fds, n, -1);
    if (n < 0) {
      continue;
    }

    n = 0;
    if (fdout) {
      if (fds[n++].revents & (POLLIN | POLLHUP)) {
        int e = read(fdout, buffer, sizeof buffer);
        if (e <= 0) {
          close(fdout);
          fdout = 0;
        } else {
          if (color && Util::s_stdout_color) {
            out.append(Util::s_stdout_color);
            out.append(buffer, e);
            out.append(ANSI_COLOR_END);
          } else {
            out.append(buffer, e);
          }
        }
      }
    }

    if (fderr) {
      if (fds[n++].revents & (POLLIN | POLLHUP)) {
        int e = read(fderr, buffer, sizeof buffer);
        if (e <= 0) {
          close(fderr);
          fderr = 0;
        } else if (err) {
          if (color && Util::s_stdout_color) {
            err->append(Util::s_stderr_color);
            err->append(buffer, e);
            err->append(ANSI_COLOR_END);
          } else {
            err->append(buffer, e);
          }
        }
      }
    }
  }

  int status;
  bool ret = false;
  if (waitpid(pid, &status, 0) != pid) {
    Logger::Error("Failed to wait for `%s'\n", path);
  } else if (WIFEXITED(status)) {
    if (WEXITSTATUS(status) != 0) {
      Logger::Verbose("Status %d running command: `%s'\n",
                      WEXITSTATUS(status), path);
      while (*argv) {
        Logger::Verbose("  arg: `%s'\n", *argv);
        argv++;
      }
    } else {
      ret = true;
    }
  } else {
    Logger::Verbose("Non-normal exit\n");
    if (WIFSIGNALED(status)) {
      Logger::Verbose("  signaled with %d\n", WTERMSIG(status));
    }
  }
  return ret;
}

int Process::Exec(const std::string &cmd, const std::string &outf,
                  const std::string &errf) {
  vector<string> argvs;
  Util::split(' ', cmd.c_str(), argvs);
  if (argvs.empty()) {
    return -1;
  }

  int pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  Util::safe_strerror(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    signal(SIGTSTP,SIG_IGN);

    swap_fd(outf, stdout);
    swap_fd(errf, stderr);

    int count = argvs.size();
    char **argv = (char**)calloc(count + 1, sizeof(char*));
    for (int i = 0; i < count; i++) {
      argv[i] = (char*)argvs[i].c_str();
    }
    argv[count] = NULL;

    execvp(argv[0], argv);
    Logger::Error("Failed to exec `%s'\n", cmd.c_str());
    _exit(-1);
  }
  int status = -1;
  wait(&status);
  return status;
}

int Process::Exec(const char *path, const char *argv[], int *fdin, int *fdout,
                  int *fderr) {
  CPipe pipein, pipeout, pipeerr;
  if (!pipein.open() || !pipeout.open() || !pipeerr.open()) {
    return 0;
  }

  int pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  Util::safe_strerror(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    /**
     * I don't know why, but things work alot better if this process ignores
     * the tstp signal (ctrl-Z). If not, it locks up if you hit ctrl-Z then
     * "bg" the program.
     */
    signal(SIGTSTP,SIG_IGN);

    if (pipein.dupOut2(fileno(stdin)) && pipeout.dupIn2(fileno(stdout)) &&
        pipeerr.dupIn2(fileno(stderr))) {
      pipeout.close(); pipeerr.close(); pipein.close();

      const char *argvnull[2] = {"", NULL};
      execvp(path, const_cast<char**>(argv ? argv : argvnull));
    }
    Logger::Error("Failed to exec `%s'\n", path);
    _exit(-1);
  }
  if (fdout) *fdout = pipeout.detachOut();
  if (fderr) *fderr = pipeerr.detachOut();
  if (fdin)  *fdin  = pipein.detachIn();
  return pid;
}

/**
 * Copied from http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
void Process::Daemonize(const char *stdoutFile /* = "/dev/null" */,
                        const char *stderrFile /* = "/dev/null" */) {
  pid_t pid, sid;

  /* already a daemon */
  if (getppid() == 1) return;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* At this point we are executing as the child process */

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory.  This prevents the current
     directory from being locked; hence not being able to remove it. */
  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  /* Redirect standard files to /dev/null */
  if (!freopen("/dev/null", "r", stdin)) exit(EXIT_FAILURE);
  if (stdoutFile && *stdoutFile) {
    if (!freopen(stdoutFile, "a", stdout)) exit(EXIT_FAILURE);
  } else {
    if (!freopen("/dev/null", "w", stdout)) exit(EXIT_FAILURE);
  }
  if (stderrFile && *stderrFile) {
    if (!freopen(stderrFile, "a", stderr)) exit(EXIT_FAILURE);
  } else {
    if (!freopen("/dev/null", "w", stderr)) exit(EXIT_FAILURE);
  }
}

///////////////////////////////////////////////////////////////////////////////
// /proc/* parsing functions

pid_t Process::GetProcessId(const std::string &cmd,
                            bool matchAll /* = false */) {
  std::vector<pid_t> pids;
  GetProcessId(cmd, pids, matchAll);
  return pids.empty() ? 0 : pids[0];
}

void Process::GetProcessId(const std::string &cmd, std::vector<pid_t> &pids,
                           bool matchAll /* = false */) {
  const char *argv[] = {"", "/proc", "-regex", "/proc/[0-9]+/cmdline", NULL};
  string out;
  Exec("find", argv, NULL, out);

  vector<string> files;
  Util::split('\n', out.c_str(), files, true);

  string ccmd = cmd;
  if (!matchAll) {
    size_t pos = ccmd.find(' ');
    if (pos != string::npos) {
      ccmd = ccmd.substr(0, pos);
    }
    pos = ccmd.rfind('/');
    if (pos != string::npos) {
      ccmd = ccmd.substr(pos + 1);
    }
  } else {
    ccmd += " ";
  }

  for (unsigned int i = 0; i < files.size(); i++) {
    string &filename = files[i];

    FILE * f = fopen(filename.c_str(), "r");
    if (f) {
      string cmdline;
      FileReader::readString(f, cmdline);
      fclose(f);
      string converted;
      if (matchAll) {
        for (unsigned int i = 0; i < cmdline.size(); i++) {
          char ch = cmdline[i];
          converted += ch ? ch : ' ';
        }
      } else {
        converted = cmdline;
        size_t pos = converted.find('\0');
        if (pos != string::npos) {
          converted = converted.substr(0, pos);
        }
        pos = converted.rfind('/');
        if (pos != string::npos) {
          converted = converted.substr(pos + 1);
        }
      }

      if (converted == ccmd && filename.find("/proc/") == 0) {
        long long pid = atoll(filename.c_str() + strlen("/proc/"));
        if (pid) {
          pids.push_back(pid);
        }
      }
    }
  }
}

std::string Process::GetCommandLine(pid_t pid) {
  string name = "/proc/" + boost::lexical_cast<string>((long long)pid) +
    "/cmdline";

  string cmdline;
  FILE * f = fopen(name.c_str(), "r");
  if (f) {
    FileReader::readString(f, cmdline);
    fclose(f);
  }

  string converted;
  for (unsigned int i = 0; i < cmdline.size(); i++) {
    char ch = cmdline[i];
    converted += ch ? ch : ' ';
  }
  return converted;
}

bool Process::CommandStartsWith(pid_t pid, const std::string &cmd) {
  if (!cmd.empty()) {
    std::string cmdline = GetCommandLine(pid);
    if (cmdline.length() >= cmd.length() &&
        cmdline.substr(0, cmd.length()) == cmd) {
      return true;
    }
  }
  return false;
}

bool Process::IsUnderGDB() {
  return CommandStartsWith(GetParentProcessId(), "gdb ");
}

int Process::GetProcessRSS(pid_t pid) {
  string name = "/proc/" + boost::lexical_cast<string>((long long)pid) +
    "/status";

  string status;
  FILE * f = fopen(name.c_str(), "r");
  if (f) {
    FileReader::readString(f, status);
    fclose(f);
  }

  vector<string> lines;
  Util::split('\n', status.c_str(), lines, true);
  for (unsigned int i = 0; i < lines.size(); i++) {
    string &line = lines[i];
    if (line.find("VmRSS:") == 0) {
      for (unsigned int j = strlen("VmRSS:"); j < line.size(); j++) {
        if (line[j] != ' ') {
          long long mem = atoll(line.c_str() + j);
          return mem/1024;
        }
      }
    }
  }

  return 0;
}

int Process::GetCPUCount() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

#ifndef __LP64__
/* For PIC code we need to save the %EBX */
static __inline void do_cpuid(u_int ax, u_int *p) {
  __asm __volatile("pushl %%ebx\n\t"
                   "cpuid\n\t"
                   "movl %%ebx, %1\n\t" // %1 is the register assigned to p[1]
                   "popl %%ebx\n\t"
                   : "=a" (p[0]), "=r" (p[1]), "=c" (p[2]), "=d" (p[3])
                   : "a" (ax));
}
#else
static __inline void do_cpuid(u_int ax, u_int *p) {
  __asm __volatile("cpuid"
                   : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
                   : "0" (ax));
}
#endif

std::string Process::GetCPUModel() {
  uint32_t regs[4];
  do_cpuid(0, regs);

  char cpu_vendor[13];
  //uint32_t cpu_high = regs[0];
  ((uint32_t *)&cpu_vendor)[0] = regs[1];
  ((uint32_t *)&cpu_vendor)[1] = regs[3];
  ((uint32_t *)&cpu_vendor)[2] = regs[2];
  cpu_vendor[12] = '\0';

  //do_cpuid(1, regs);
  //uint32_t cpu_id       = regs[0];
  //uint32_t cpu_procinfo = regs[1];
  //uint32_t cpu_feature  = regs[3];
  //uint32_t cpu_feature2 = regs[2];

  uint32_t cpu_exthigh = 0;
  if (strcmp(cpu_vendor, "GenuineIntel") == 0 ||
      strcmp(cpu_vendor, "AuthenticAMD") == 0) {
    do_cpuid(0x80000000, regs);
    cpu_exthigh = regs[0];
  }

  char cpu_brand[48];
  char *brand = cpu_brand;
  if (cpu_exthigh >= 0x80000004) {
    for (u_int i = 0x80000002; i < 0x80000005; i++) {
      do_cpuid(i, regs);
      memcpy(brand, regs, sizeof(regs));
      brand += sizeof(regs);
    }
  }
  *brand = '\0';
  return cpu_brand;
}

///////////////////////////////////////////////////////////////////////////////

std::string Process::GetAppName() {
  const char* progname = getenv("_");
  if (!progname || !*progname) {
    progname = "unknown program";
  }
  return progname;
}

std::string Process::GetAppVersion() {
#ifdef HPHP_VERSION
#undefine HPHP_VERSION
#endif
#define HPHP_VERSION(v) return #v;
#include "../version"
}

std::string Process::GetHostName() {
  char hostbuf[128];
  gethostname(hostbuf, 127);
  hostbuf[127] = '\0';
  return hostbuf;
}

std::string Process::GetCurrentUser() {
  const char *name = getenv("LOGNAME");
  if (name && *name) {
    return name;
  }
  passwd *pwd = getpwuid(geteuid());
  if (pwd && pwd->pw_name) {
    return pwd->pw_name;
  }
  return "";
}

std::string Process::GetCurrentDirectory() {
  char buf[PATH_MAX];
  memset(buf, 0, PATH_MAX);
  return getcwd(buf, PATH_MAX);
}

std::string Process::GetHomeDirectory() {
  string ret;

  const char *home = getenv("HOME");
  if (home && *home) {
    ret = home;
  } else {
    passwd *pwd = getpwent();
    if (pwd && pwd->pw_dir) {
      ret = pwd->pw_dir;
    }
  }

  if (ret.empty() || ret[ret.size() - 1] != '/') {
    ret += '/';
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
