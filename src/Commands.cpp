#include "Commands.hpp"

#include <sampapi/0.3.7-R1/CInput.h>

bool Commands::TryRegister(Handler handler) {
  if (sampapi::GetBase() == 0) {
    return false;
  }

  auto* input = sampapi::v037r1::RefInputBox();
  if (!input) {
    registeredInput_ = nullptr;
    return false;
  }

  if (registeredInput_ == input) {
    return true;
  }

  if (const auto existing = input->GetCommandHandler("dildo")) {
    if (existing == handler) {
      registeredInput_ = input;
      return true;
    }
    return false;
  }

  if (input->m_nCommandCount >= sampapi::v037r1::CInput::MAX_CLIENT_CMDS) {
    return false;
  }

  input->AddCommand("dildo", handler);
  if (input->GetCommandHandler("dildo") != handler) {
    return false;
  }

  registeredInput_ = input;
  return true;
}
