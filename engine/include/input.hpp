#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <mutex>

class InputStates {
public:
  enum Value { RELEASED, HELD, JUST_RELEASED, JUST_PRESSED };

  InputStates() = default;
  constexpr InputStates(Value state) : value(state) {}

  constexpr bool operator==(InputStates a) const { return value == a.value; }
  constexpr bool operator!=(InputStates a) const { return value != a.value; }

  constexpr bool IsHeld() const { return value == HELD; }
  constexpr bool IsReleased() const { return value == RELEASED; }
  constexpr bool IsJustPressed() const { return value == JUST_PRESSED; }
  constexpr bool IsJustReleased() const { return value == JUST_RELEASED; }
  constexpr bool IsDown() const { return IsHeld() || IsJustPressed(); }
  constexpr bool IsUp() const { return IsReleased() || IsJustReleased(); }

private:
  Value value;
};

class InputManager {
private:
  std::map<unsigned int, InputStates> key_map;
  std::mutex mtx; // thread safety since we are using singleton pattern

  // store game input mapping here
  float axis_vertical_movement = 0.0f;   // (W || ←) to (S || →)
  float axis_horizontal_movement = 0.0f; // (A || ↑) to (D || ↓)

  bool use_text_input = false;

  const char *text_input_buffer = "";

public:
  static void Update(const uint8_t *key_state, const int num_keys);

  // useful for debugging, prefer mapping values for actual game input
  static InputStates GetKey(SDL_Scancode key);

  // create getters for game input mapping here

  // returns a normalized vector for movement
  static glm::vec2 GetVectorMovement();

  static float GetAxisHorizontalMovement();

  static bool GetTriggerJump();

  static void ToggleTextInput();

  static const char *GetTextInputBuffer();

  static void SetTextInputBuffer(const char *text);

  static bool IsTextInputActive();
};