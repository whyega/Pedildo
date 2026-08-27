#include "Plugin.hpp"

#include <plugin.h>

#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

std::vector<std::string> ParseArguments(const char* arguments) {
  std::istringstream input{arguments ? arguments : ""};
  std::vector<std::string> tokens;

  for (std::string token; input >> token;) {
    for (auto& character : token) {
      character = static_cast<char>(
          std::tolower(static_cast<unsigned char>(character)));
    }
    tokens.push_back(std::move(token));
  }
  return tokens;
}

}  // namespace

void Plugin::OnAttach(void* module) {
  (void)module;

  constexpr std::array<std::uintptr_t, 2> renderCalls{0x5E77FC,
                                                      0x5E780A};
  for (std::size_t i = 0; i < renderPedHooks_.size(); ++i) {
    auto& hook = renderPedHooks_[i];
    hook.set_dest(renderCalls[i]);
    hook.before.connect([this](const auto&, CPed*& ped) {
      commands_.TryRegister(&Plugin::SelectDildo);
      remote_.Process();

      if (!ped) {
        return false;
      }

      return !skeleton_.Render(*ped) &&
             !remote_.ShouldSuppress(ped);
    });
    hook.install();
  }

  plugin::Events::initGameEvent += [this] {
    skeleton_.Initialize();
    remote_.Initialize();
  };

  plugin::Events::gameProcessEvent += [this] {
    remote_.Process();
  };

  plugin::Events::shutdownRwEvent += [this] {
    remote_.Shutdown();
    skeleton_.Shutdown();
  };
}

void Plugin::OnDetach() {
  for (auto& hook : renderPedHooks_) {
    hook.remove();
  }
  remote_.Shutdown();
  skeleton_.Shutdown();
}

void __cdecl Plugin::SelectDildo(const char* arguments) {
  auto& plugin = GetInstance();
  const auto tokens = ParseArguments(arguments);

  if (tokens.size() == 2 && tokens[0] == "all") {
    if (tokens[1] == "on") {
    plugin.remote_.SetEnabled(true);
    } else if (tokens[1] == "off") {
    plugin.remote_.SetEnabled(false);
    } else if (tokens[1].size() == 1 && tokens[1][0] >= '1' &&
               tokens[1][0] <= '3') {
      const auto number = static_cast<unsigned int>(tokens[1][0] - '0');
      if (plugin.remote_.SetModel(number)) {
        plugin.remote_.SetEnabled(true);
      }
    }
    return;
  }

  if (tokens.size() == 1 && tokens[0].size() == 1 &&
      tokens[0][0] >= '1' && tokens[0][0] <= '3') {
    plugin.skeleton_.SelectModel(
        static_cast<unsigned int>(tokens[0][0] - '0'));
  }
}
