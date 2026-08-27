#pragma once

class Commands final {
 public:
  using Handler = void(__cdecl*)(const char*);

  bool TryRegister(Handler handler);

 private:
  void* registeredInput_{};
};
