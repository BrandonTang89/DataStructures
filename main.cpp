int global_array[100] = {-1};

int main(const int argc, char **argv) {
    return global_array[argc + 100]; // global buffer overflow
}
