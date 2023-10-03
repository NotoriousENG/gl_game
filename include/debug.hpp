#pragma once
#include<mutex>

class DebugManager {
private:
  std::mutex mtx;
  bool render_colliders = false;

  public:
	  static void ToggleRenderColliders();
	  static bool GetRenderColliders();
};