#pragma once

struct CmagTarget;

class TargetSelection {
public:
    explicit TargetSelection() = default;

    void select(CmagTarget *target) { selection = target; }
    const CmagTarget *getSelection() const { return selection; }
    CmagTarget *getMutableSelection() { return selection; }
    bool isSelected(const CmagTarget &target) const { return selection == &target; }

private:
    CmagTarget *selection = nullptr;
};
