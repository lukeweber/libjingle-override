
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This class sets up the environment for running the native tests inside an
// android application. It outputs (to logcat) markers identifying the
// START/END/CRASH of the test suite, FAILURE/SUCCESS of individual tests etc.
// These markers are read by the test runner script to generate test results.
// It injects an event listener in gtest to detect various test stages and
// installs signal handlers to detect crashes.

#include <android/log.h>
#include <signal.h>
#include <stdio.h>
#include <jni.h>

#include "talk/base/flags.h"
#include "talk/base/fileutils.h"
#include "talk/base/unixfilesystem.h"
#include "talk/base/gunit.h"
#include "talk/base/logging.h"
#include "talk/base/pathutils.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

// The main function of the program to be wrapped as a test apk.
extern int main(int argc, char** argv);

namespace {

// The list of signals which are considered to be crashes.
const int kExceptionSignals[] = {
  SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, -1
};

const char kCrashedMarker[] = "[ CRASHED      ]\n";

struct sigaction g_old_sa[NSIG];

// This function runs in a compromised context. It should not allocate memory.
void SignalHandler(int sig, siginfo_t *info, void *reserved)
{
  // Output the crash marker.
  write(STDOUT_FILENO, kCrashedMarker, sizeof(kCrashedMarker) -1);
  __android_log_write(ANDROID_LOG_ERROR, "webrtcjingle", kCrashedMarker);
  g_old_sa[sig].sa_sigaction(sig, info, reserved);
}

void InstallHandlers() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sa.sa_sigaction = SignalHandler;
  sa.sa_flags = SA_SIGINFO;

  for (unsigned int i = 0; kExceptionSignals[i] != -1; ++i) {
    sigaction(kExceptionSignals[i], &sa, &g_old_sa[kExceptionSignals[i]]);
  }
}

#define OVERRIDE
// As we are the native side of an Android app, we don't have any 'console', so
// gtest's standard output goes nowhere.
// Instead, we inject an "EventListener" in gtest and then we print the results
// using LOG, which goes to adb logcat.
class AndroidLogPrinter : public ::testing::EmptyTestEventListener {
 public:
  void Init(int* argc, char** argv);

  // EmptyTestEventListener
  virtual void OnTestProgramStart(
	  const ::testing::UnitTest& unit_test); OVERRIDE;
  virtual void OnTestStart(const ::testing::TestInfo& test_info) OVERRIDE;
  virtual void OnTestPartResult(
      const ::testing::TestPartResult& test_part_result) OVERRIDE;
  virtual void OnTestEnd(const ::testing::TestInfo& test_info) OVERRIDE;
  virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test) OVERRIDE;
};

void AndroidLogPrinter::Init(int* argc, char** argv) {
  // InitGoogleTest must be called befure we add ourselves as a listener.
  ::testing::InitGoogleTest(argc, argv);
  ::testing::TestEventListeners& listeners =
      ::testing::UnitTest::GetInstance()->listeners();
  // Adds a listener to the end.  Google Test takes the ownership.
  listeners.Append(this);
  LOGI("AndroidLogPrinter::Init argc=%d", *argc);
}

void AndroidLogPrinter::OnTestProgramStart(
    const ::testing::UnitTest& unit_test) {
  std::stringstream msg ;
  msg << "[ START      ] "<< unit_test.test_to_run_count();
  LOG(LS_ERROR) << msg.str();
}

void AndroidLogPrinter::OnTestStart(const ::testing::TestInfo& test_info) {
  std::stringstream msg;
  msg << "[ RUN      ] " << test_info.test_case_name() << "." << test_info.name();
  LOG(LS_ERROR) << msg.str();
}

void AndroidLogPrinter::OnTestPartResult(
    const ::testing::TestPartResult& test_part_result) {
  std::stringstream msg;
  msg << (test_part_result.failed() ? "*** Failure" : "Success")
      << " in " << test_part_result.file_name()
      << test_part_result.line_number()
      << test_part_result.summary();
  LOG(LS_ERROR) << msg.str();
}

void AndroidLogPrinter::OnTestEnd(const ::testing::TestInfo& test_info) {
  std::stringstream msg;
  msg << (test_info.result()->Failed() ? "[  FAILED  ]" : "[       OK ]")
      << " " << test_info.test_case_name() << "." << test_info.name();
  LOG(LS_ERROR) << msg;
}

void AndroidLogPrinter::OnTestProgramEnd(
    const ::testing::UnitTest& unit_test) {
  std::stringstream msg;
  msg << "[ END      ] " << unit_test.successful_test_count();
  LOG(LS_ERROR) << msg;
}

}  // namespace


talk_base::Pathname GetTalkDirectory() {
  talk_base::Pathname path (std::string("/sdcard/talk"));
  return path;
}



// This method is called on a separate java thread so that we won't trigger
// an ANR.
extern "C" {
  JNIEXPORT void JNICALL Java_org_chromium_native_1test_ChromeNativeTestActivity_nativeRunTests
  (JNIEnv *, jobject, jstring, jobject);
}

JNIEXPORT void JNICALL Java_org_chromium_native_1test_ChromeNativeTestActivity_nativeRunTests(
    JNIEnv* env,
    jobject obj,
    jstring jfiles_dir,
    jobject app_context) {

  InstallHandlers();

  const char* nativeDir = env->GetStringUTFChars(jfiles_dir, NULL);
  // A few options, such "--gtest_list_tests", will just use printf directly
  // and won't use the "AndroidLogPrinter". Redirect stdout to a known file.
  //  std::string out_dir(nativeDir);
  std::string out_dir = "/sdcard/talk";
  std::string stdout_path = out_dir + "/stdout.txt";
  freopen(stdout_path.c_str(), "w", stdout);

  std::vector<char *> argv;
  argv.push_back((char*) "ChromeTestActivity");
  std::string  out_xml = "--gtest_output=xml:" + out_dir;
  argv.push_back((char*) out_xml.c_str());
  argv.push_back((char*) "--gtest_color=no");
  int argc = argv.size();

  // This object is owned by gtest.
  AndroidLogPrinter* log = new AndroidLogPrinter();
  log->Init(&argc, &argv[0]);

  talk_base::Filesystem::SetOrganizationName("google");
  talk_base::Filesystem::SetApplicationName("unittest");
  talk_base::UnixFilesystem::SetAppTempFolder(out_dir);
  talk_base::UnixFilesystem::SetAppDataFolder(out_dir);

  main(argc, &argv[0]);
}


int main(int argc, char** argv) {

  talk_base::LogMessage::LogTimestamps();
  talk_base::LogMessage::ConfigureLogging("debug", "/sdcard/talk/unittest.log");

  int res = RUN_ALL_TESTS();

  // clean up logging so we don't appear to leak memory.
  talk_base::LogMessage::ConfigureLogging("", "");
  return res;
}
