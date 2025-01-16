/**
 * For students:
 * To complete the assignment, you have only two lines to pay attention to in
 * this file.
 */
#include <fstream>

#include "nlohmann/json.hpp"
#include "rdr/render.h"

using namespace RDR_NAMESPACE_NAME;

static void printHelp(int, char *argv[]) {  // NOLINT
  std::ostringstream oss;
  oss << "RDR171 version 0.1, Copyright (c) ShanghaiTech CS171 TAs\n"
      << "Please DO NOT EVER release the source code containing your "
         "implementations\n"
      << format("Usage: {} [options] <One scene JSON file>\n", argv[0])
      << format("  --help,-h             Print this help text.\n")
      << format(
             "  --quite,-q            Do not output anything during "
             "rendering.\n")
      << format("  --output,-o <path>    Override the default output path.\n")
      << format(
             "  --override  <json>    Override the scene specification with a "
             "single-line json,\n"
             "                        e.g. --override "
             "'{{\"integrator\":{{\"type\":\"path\",\"profile\":\"MIS\"}}}}'"
             "\n");
  print("{}", oss.str());
}

int rdr_main(int argc, char *argv[]) {  // NOLINT: alias of main function
  // You should skip most of this function, since only two lines are related to
  // core implementation.

  bool quiet = false;
  fs::path source_path{};
  std::optional<std::string> output_path{};
  std::optional<std::string> override_json_string{};

  if (argc <= 1) {
    printHelp(argc, argv);
    return 0;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      printHelp(argc, argv);
      return 0;
    } else if (arg == "--quite" || arg == "-q") {
      quiet = true;
    } else if (arg == "--output" || arg == "-o") {
      if (i + 1 < argc) {
        output_path = argv[++i];
      } else {
        print("Missing output path after [ {} ]\n", arg);
        printHelp(argc, argv);
        return 1;
      }
    } else if (arg == "--override") {
      if (i + 1 < argc) {
        override_json_string = argv[++i];
      } else {
        print("Missing JSON override after [ {} ]\n", arg);
        printHelp(argc, argv);
        return 1;
      }
    } else {
      source_path = arg;
    }
  }

  // Init logger first anyway
  InitLogger(true, quiet);

  // Register factory classes
  Factory::doRegisterAllClasses();

  Info_("===    RDR171 Launching    ===");
  Info_("===    HAPPY RENDERING!    ===");

  if (source_path.extension() != ".json") {
    Exception_("Please specify a JSON file as the scene configuration");
    printHelp(argc, argv);
    return 1;
  }

  // Check if the file exists
  if (!fs::exists(source_path)) {
    Exception_("File {} does not exist", source_path.string());
    return 1;
  }

  // Initialize file resolver
  FileResolver::setBasePath(source_path.parent_path());

  Info_("FileResolver has been initialized with base path [ {} ]",
      source_path.parent_path().string());

  // Load config from json file
  std::ifstream fin;
  fin.open(source_path);

  if (!fin.is_open()) {
    Exception_("Can not open the JSON file [ {} ]", source_path.string());
    return 1;
  } else {
    Info_("JSON file loaded from [ {} ]", source_path.string());
  }

  // Parse json object to Config
  nlohmann::json root_json;
  Properties root_properties;
  try {
    fin >> root_json;
    if (override_json_string.has_value())
      root_json.update(
          nlohmann::json::parse(override_json_string.value()), true);
    root_properties = Properties(root_json);
  } catch (nlohmann::json::exception &ex) {
    Exception_("{}", ex.what());
    return 1;
  }

  if (!output_path.has_value())
    output_path = source_path.filename().stem().string() + ".exr";
  Info_("Root Properties initialized with [ JSON ]. Start building scene...");
  ref<RenderInterface> render = make_ref<NativeRender>(root_properties);

  render->initialize();
  render->preprocess();

  /*===---------------------------------------------------------------===*
   // Start rendering
   *===---------------------------------------------------------------===*/

  Info_("Scene built. Start rendering...");
  auto start = std::chrono::steady_clock::now();

  // Maybe here? try to press ctrl and click on the function name to jump
  // around.
  render->render();
  render->exportImageToDisk(output_path.value());

  auto end = std::chrono::steady_clock::now();
  auto time =
      std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  Info_("Render Finished in {}s", time);
  return 0;
}

// The main function is wrapped, so that we can catch exceptions and print them.
// The exception system of this renderer is relatively simple and is not
// recommended to follow.
int main(int argc, char *argv[]) {
  int ret_val = 0;

  try {
    ret_val = rdr_main(argc, argv);
  } catch (const rdr_exception &e) {
    ret_val = 1;
    Error_("Renderer local exception encountered: {}", e.what());
  } catch (const std::exception &e) {
    ret_val = 2;
    Error_("Renderer non-local exception encountered: {}", e.what());
  }

  return ret_val;
}
