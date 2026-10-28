// Compile on your PC, from the SemesterProject folder:
//   g++ -o run_on_robot Semesterprojekt_3/run_on_robot.cpp
// Run:
//   ./run_on_robot

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// Change this line when the Pi gets a new address.
const char *sshID = "pi@172.20.10.4";
const char *LocalFile = "tests/50cm_accurate_test.cpp";
const char *remoteFile = "/home/pi/code_ws/src/rb3_package_cpp/src/rb3_node_cpp.cpp";
// A socket file on your PC. The first ssh keeps the login open here, so the later scp and ssh reuse it and do not ask for the password again.
const char *kSocket = "/tmp/tb3-run-on-robot.sock";

// Shell commands that run on the Pi. bash -s reads this text on standard input.
const char *kRemoteScript = R"REMOTE(
  echo "Stopping leftover motor and program processes..."
  pkill -f '[t]urtlebot3_bringup robot.launch.py' || true
  pkill -f '[t]urtlebot3_node/turtlebot3_ros' || true
  pkill -f '[r]b3_package_cpp/rb3_node_cpp' || true
  sleep 1

  set -e
  source /opt/ros/jazzy/setup.bash
  source /home/pi/turtlebot3_ws/install/setup.bash
  source /home/pi/rb3_ws/install/setup.bash
  export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
  export ROS_DOMAIN_ID=30
  export LDS_MODEL=LDS-02
  export TURTLEBOT3_MODEL=burger
  #export PYTHONUNBUFFERED=1

  echo "Building rb3_package_cpp..."
  cd /home/pi/code_ws
  #colcon build --symlink-install --packages-select rb3_package_cpp
  source /home/pi/code_ws/install/setup.bash

  # Background jobs get their own process group, so cleanup can stop the launch
  # and every process it started.
  set -m
  set +e

  LAUNCH_PID=
  cleanup() {
    trap - EXIT INT TERM HUP
    if [ -n "$LAUNCH_PID" ]; then
      kill -- -"$LAUNCH_PID" 2>/dev/null || kill "$LAUNCH_PID" 2>/dev/null || true
    fi
    pkill -f '[t]urtlebot3_bringup robot.launch.py' 2>/dev/null || true
    pkill -f '[t]urtlebot3_node/turtlebot3_ros' 2>/dev/null || true
    pkill -f '[r]b3_package_cpp/rb3_node_cpp' 2>/dev/null || true
  }
  trap cleanup EXIT INT TERM HUP

  echo "Starting motors..."
  : > /tmp/tb3_motor.log
  ros2 launch turtlebot3_bringup robot.launch.py > /tmp/tb3_motor.log 2>&1 &
  LAUNCH_PID=$!

  ready=0
  for _ in 1 2 3 4 5 6 7 8 9 10; do
    if grep -q 'diff_drive_controller]: Run!' /tmp/tb3_motor.log; then
      ready=1
      break
    fi
    if ! kill -0 "$LAUNCH_PID" 2>/dev/null; then
      break
    fi
    sleep 1
  done

  if [ "$ready" -ne 1 ]; then
    echo "Motor startup failed. Log from the Pi:" >&2
    cat /tmp/tb3_motor.log >&2
    exit 1
  fi

  echo "Motors are running. Starting your program. Ctrl+C stops both."
  ros2 run rb3_package_cpp rb3_node_cpp
  exit $?
)REMOTE";

// ssh started by run_process. The Ctrl+C handler may kill only this pid.
volatile sig_atomic_t g_child = -1;

void on_stop(int) {
  if (g_child > 0) {
    kill(g_child, SIGTERM);
  }
}

int run_process(const std::vector<std::string> &args, const std::string *input, bool quiet) {
  int pipefd[2] = {-1, -1};
  if (input != nullptr && pipe(pipefd) != 0) {
    std::perror("pipe");
    return 1;
  }

  const pid_t pid = fork();
  if (pid < 0) {
    std::perror("fork");
    return 1;
  }

  if (pid == 0) {
    if (quiet) {
      int devnull = open("/dev/null", O_WRONLY);
      if (devnull >= 0) {
        dup2(devnull, STDERR_FILENO);
        close(devnull);
      }
    }
    if (input != nullptr) {
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
    }
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (const std::string &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    execvp(argv[0], argv.data());
    std::perror(argv[0]);
    _exit(127);
  }

  g_child = pid;

  if (input != nullptr) {
    close(pipefd[0]);
    const char *data = input->data();
    std::size_t left = input->size();
    while (left > 0) {
      const ssize_t wrote = write(pipefd[1], data, left);
      if (wrote < 0) {
        std::perror("write");
        break;
      }
      data += wrote;
      left -= static_cast<std::size_t>(wrote);
    }
    close(pipefd[1]);
  }

  int status = 0;
  while (waitpid(pid, &status, 0) < 0) {
    if (errno != EINTR) {
      std::perror("waitpid");
      return 1;
    }
  }
  g_child = -1;
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return 1;
}

void close_login() {
  run_process({"ssh", "-S", kSocket, "-O", "exit", sshID}, nullptr, true);
  unlink(kSocket);
}

// Ctrl+C stops the local ssh, but the motors were started in the background on
// the Pi and can keep running. This second command uses the same login to stop them.
void stop_robot() {
  const char *cmd =
      "pkill -f '[t]urtlebot3_bringup robot.launch.py' || true; "
      "pkill -f '[t]urtlebot3_node/turtlebot3_ros' || true; "
      "pkill -f '[r]b3_package_cpp/rb3_node_cpp' || true";
  run_process({"ssh", "-S", kSocket, sshID, "bash", "-c", cmd}, nullptr, true);
}

int main() {
  // std::ifstream source(LocalFile);
  // if (!source) {
  //   std::cerr << "Could not open " << LocalFile
  //             << ". Run this program from the SemesterProject folder.\n";
  //   return 1;
  // }
  // source.close();

  struct sigaction action = {};
  action.sa_handler = on_stop;
  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGTERM, &action, nullptr);

  // Drop a login left behind by a previous Ctrl+C, then open one new login.
  close_login();
  std::cout << "Connecting to " << sshID << ". Type the Pi password once.\n";
  const int login = run_process(
      {"ssh", "-M", "-S", kSocket, "-o", "ControlPersist=yes", "-o",
       "StrictHostKeyChecking=accept-new", "-fN", sshID},
      nullptr, false);
  if (login != 0) {
    std::cerr << "Login failed.\n";
    return login;
  }

  // std::cout << "Copying " << LocalFile << "\n";
  // const int copied = run_process(
  //     {"scp", "-o", std::string("ControlPath=") + kSocket, LocalFile,
  //      std::string(sshID) + ":" + remoteFile},
  //     nullptr, false);
  // if (copied != 0) {
  //   std::cerr << "Copy failed.\n";
  //   close_login();
  //   return copied;
  // }

  // No -tt here. With -tt, ssh switches the local terminal to raw mode and Ctrl+C
  // is not delivered as a signal, so this program never gets a chance to stop the Pi.
  const std::string script = kRemoteScript;
  const int ran = run_process(
      {"ssh", "-S", kSocket, sshID, "bash", "-s"}, &script, false);
  stop_robot();
  close_login();
  return ran;
}
