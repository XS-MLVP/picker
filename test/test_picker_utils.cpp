#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <picker.hpp>

using namespace std;

static void write_text(const std::string &p, const std::string &t) {
  ofstream o(p, ios::trunc);
  o << t;
}

int main() {
  // trim/ltrim/rtrim and trim with custom pattern
  assert(picker::ltrim(string("  hi")) == "hi");
  assert(picker::rtrim(string("hi  ")) == "hi");
  assert(picker::trim(string("  hi \t")) == "hi");
  assert(picker::trim(string("--x--"), string("-")) == string("x"));

  // strsplit basic
  auto ss = picker::strsplit(string("a b  c"));
  assert(ss.size() == 3 && ss[0] == "a" && ss[1] == "b" && ss[2] == "c");
  // strsplit initializer_list (front/back)
  assert(picker::strsplit(string("x/y.z"), {"/", "."}, true) == string("x"));
  assert(picker::strsplit(string("x/y.z"), {"/", "."}, false) == string("z"));

  // case transforms + first upper
  assert(picker::lower_case("AbC") == string("abc"));
  assert(picker::capitalize_first_letter("abc") == string("Abc"));
  assert(picker::first_uppercase("abc") == string("Abc"));

  // conflict pin name map and fixer
  auto cmap = picker::get_default_conflict_map();
  assert(!cmap.empty());
  auto pin  = picker::fix_conflict_pin_name("dut", cmap, false);
  auto pinC = picker::fix_conflict_pin_name("dut", cmap, true);
  assert(pin.rfind("pin_", 0) == 0);
  assert(pinC.rfind("Pin_", 0) == 0);
  // non-conflict
  assert(picker::fix_conflict_pin_name("DATA", cmap, false) == string("DATA"));

  // replace helpers
  assert(picker::streplace(string("a_b_c"), string("_"), string("-")) == string("a-b-c"));
  assert(picker::streplace(string("a_b_c"), {"a_", "_c"}, string("")) == string("b"));

  // contains helpers and key_as_vector
  vector<int> vi{1, 2, 3};
  assert(picker::contains(vi, 2));
  map<string, int> m{{"x", 1}, {"y", 2}};
  auto keys = picker::key_as_vector(m);
  assert(keys.size() == 2);

  // join string vector
  vector<string> j{"a", "b", "c"};
  assert(picker::join_str_vec(j, ":") == string("a:b:c"));

  // base_name, get_path
  assert(picker::base_name("/tmp/file.txt") == string("file.txt"));
  assert(picker::base_name("/tmp/file.txt", false) == string("file"));
  assert(picker::get_path("/tmp/file.txt").find("/tmp") == 0);

  // path normalize/join
  auto joined = picker::path_join({"/tmp/", "/a/", "b", "c/"});
  assert(joined.find("/tmp/a/b/c") != string::npos);
  auto normed = picker::normal_path(".");
  assert(!normed.empty() && normed[0] == '/');

  // str_start helpers
  assert(picker::str_start_with("hello", "he"));
  assert(picker::str_start_with_digit("1abc"));
  assert(!picker::str_start_with_digit("abc"));

  // env
  ::setenv("TEST_PICKER_ENV", "v123", 1);
  assert(picker::get_env("TEST_PICKER_ENV", "d") == string("v123"));
  assert(picker::get_env("TEST_PICKER_ENV_NONE", "d") == string("d"));

  // extract_name
  assert(picker::extract_name("a.b", '.', 1) == string("a"));
  assert(picker::extract_name("a.b", '.', 0) == string("b"));

  // read/write/params
  const string tf = "test_params.txt";
  write_text(tf, "# c1\nA=1\nB=2 #hi\n\nC=3\n");
  auto params = picker::read_params(tf);
  assert(params.find("A=1") != string::npos);
  assert(params.find("B=2") != string::npos);
  assert(params.find("C=3") != string::npos);

  const string tf2 = "test_io.txt";
  picker::write_file(tf2, string("hello"));
  auto content = picker::read_file(tf2);
  assert(content == string("hello"));
  assert(picker::file_exists(tf2));

  // exec_result
  auto tok = picker::exec_result("echo a b c", 1);
  assert(tok == string("b"));

  // version non-empty
  assert(!picker::version().empty());

  std::cout << "test_picker_utils OK" << std::endl;
  return 0;
}
