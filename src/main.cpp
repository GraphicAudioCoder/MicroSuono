#include <iostream>
#include <thread>
#include <chrono>
#include "miniaudio.h"

int main() {
  std::cout << "MicroSuono starting ..." << std::endl;

  ma_result result;
  ma_engine engine;

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    std::cerr << "Failed to initialize audio engine." << std::endl;
    return -1;
  }

  ma_sound sound;
  result = ma_sound_init_from_file(&engine, "../resources/test.wav", 0, NULL, NULL, &sound);
  if (result != MA_SUCCESS) {
    std::cerr << "Failed to load sound file." << std::endl;
    ma_engine_uninit(&engine);
    return -1;
  }

  std::cout << "Playing sound..." << std::endl;
  ma_sound_start(&sound);

  // Aspetta che il suono finisca
  while (ma_sound_is_playing(&sound)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "Sound finished playing." << std::endl;

  ma_sound_uninit(&sound);
  ma_engine_uninit(&engine);

  return 0;
}
