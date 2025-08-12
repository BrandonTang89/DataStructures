#pragma once
class TestingTracker {
public:
    static int constructed;
    static int destructed;
    static int moves;

    TestingTracker() {
        constructed++;
    }

    TestingTracker(const TestingTracker &) {
        constructed++;
    }

    TestingTracker &operator=(const TestingTracker &) {
        constructed++;
        return *this;
    }

    TestingTracker(TestingTracker &&) noexcept {
        moves++;
    }

    TestingTracker &operator=(TestingTracker &&) noexcept {
        moves++;
        return *this;
    }


    ~TestingTracker() {
        destructed++;
    }
};
