#pragma once

#include <span>
#include <string_view>

class N64;

int main_headless(std::span<std::string_view> args);

void render_screen(const N64& n64);
void handle_frontend_events();
