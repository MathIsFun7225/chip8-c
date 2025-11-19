#ifndef CHIP8_CONFIG_H
#define CHIP8_CONFIG_H

struct chip8_config {
    int target_speed;
    int default_scale;
};

int chip8_load_config(const char *file, struct chip8_config *config);

#endif // CHIP8_CONFIG_H
