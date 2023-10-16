#pragma once

class Game {
public:
  Game();
  ~Game();
  int init();
  int update();
  int unload();
  int close();
};