#pragma once

template <typename T, T empty_value = T{}>
struct MovablePrimitive {
    T value = empty_value;
    MovablePrimitive() = default;
    MovablePrimitive(T const &init) : value(init) {}

    MovablePrimitive(MovablePrimitive const &) = default;
    MovablePrimitive(MovablePrimitive &&other) noexcept : value(other.value) {
        other.value = empty_value;
    }

    MovablePrimitive &operator=(MovablePrimitive const &) = default;
    MovablePrimitive &operator=(MovablePrimitive &&other) {
        value = other.value;
        other.value = empty_value;
        return *this;
    }

    T *operator&() {
        return &value;
    }

    const T *operator&() const {
        return &value;
    }

    operator T() const { return value; }
};
