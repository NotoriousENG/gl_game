#include "input.hpp"

static std::unique_ptr<InputManager> instance =
    std::make_unique<InputManager>();

void InputManager::Update(const uint8_t *key_state, const int num_keys) {
  std::lock_guard<std::mutex> lock(instance->mtx); // thread safety
  for (int i = 0; i < num_keys; i++) {
    // get the name of this key
    const char *key_name =
        SDL_GetKeyName(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(i)));

    // set released, pressed, just_released, just_pressed
    if (key_state[i]) {
      if (instance->key_map[i] == InputStates::RELEASED) {
        instance->key_map[i] = InputStates::JUST_PRESSED;
        // SDL_Log("%s (%i): JUST_PRESSED", key_name, i);
      } else {
        instance->key_map[i] = InputStates::HELD;
        // SDL_Log("%s (%i): HELD", key_name, i);
      }
    } else {
      if (instance->key_map[i] == InputStates::HELD) {
        instance->key_map[i] = InputStates::JUST_RELEASED;
        // SDL_Log("%s (%i): JUST_RELEASED", key_name, i);
      } else {
        instance->key_map[i] = InputStates::RELEASED;
      }
    }
  }

  // update axis values

  // horizontal
  // +1 = ARROW_RIGHT OR D
  // -1 = ARROW_LEFT OR A
  instance->axis_horizontal_movement = 0.0f;
  instance->axis_vertical_movement = 0.0f;

  if (instance->key_map[SDL_SCANCODE_RIGHT].IsDown() ||
      instance->key_map[SDL_SCANCODE_D].IsDown()) {
    instance->axis_horizontal_movement += 1.0f;
  }
  if (instance->key_map[SDL_SCANCODE_LEFT].IsDown() ||
      instance->key_map[SDL_SCANCODE_A].IsDown()) {
    instance->axis_horizontal_movement -= 1.0f;
  }

  // vertical
  // +1 = ARROW_UP OR W
  // -1 = ARROW_DOWN OR S
  if (instance->key_map[SDL_SCANCODE_UP].IsDown() ||
      instance->key_map[SDL_SCANCODE_W].IsDown()) {
    instance->axis_vertical_movement -= 1.0f;
  }
  if (instance->key_map[SDL_SCANCODE_DOWN].IsDown() ||
      instance->key_map[SDL_SCANCODE_S].IsDown()) {
    instance->axis_vertical_movement += 1.0f;
  }
}

InputStates InputManager::GetKey(SDL_Scancode key) {
  std::lock_guard<std::mutex> lock(instance->mtx); // thread safety
  return instance->key_map[key];
}

glm::vec2 InputManager::GetVectorMovement() {
  std::lock_guard<std::mutex> lock(instance->mtx); // thread safety
  glm::vec2 movement = glm::vec2(instance->axis_horizontal_movement,
                                 instance->axis_vertical_movement);
  return glm::length(movement) == 0 ? movement : glm::normalize(movement);
}

float InputManager::GetAxisHorizontalMovement() {
  std::lock_guard<std::mutex> lock(instance->mtx); // thread safety
  return instance->axis_horizontal_movement;
}

bool InputManager::GetTriggerJump() {
  std::lock_guard<std::mutex> lock(instance->mtx); // thread safety
  return instance->key_map[SDL_SCANCODE_SPACE].IsJustPressed() ||
         instance->key_map[SDL_SCANCODE_LALT].IsJustPressed();
}

InputManager *InputManager::GetInstance() { return instance.get(); }

void InputManager::SetInstance(InputManager *new_instance) {
  instance.reset(new_instance);
}
