#pragma once

class CmagProject;
struct CmagTarget;

class TargetSelection {
public:
    explicit TargetSelection(CmagProject &project);

    void select(CmagTarget *target);
    const CmagTarget *getSelection() const { return selection; }
    CmagTarget *getMutableSelection() { return selection; }
    bool isSelected(const CmagTarget &target) const { return selection == &target; }

private:
    CmagProject &project;
    CmagTarget *selection = nullptr;
};
