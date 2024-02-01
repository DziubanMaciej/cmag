#pragma once

#include "cmag_core/utils/error.h"

class TabChange {
public:
    enum TabSelection {
        Auto,
        TargetGraph,
        ListDir,
        TargetFolder,
        Summary,
    };

    void change(TabSelection newSelection) {
        if (selection != TabSelection::Auto) {
            LOG_WARNING("Multiple tab selection in one frame detected. Only last one will be honored");
        }

        selection = newSelection;
    }

    TabSelection fetch() {
        const TabSelection result = selection;
        selection = TabSelection::Auto;
        return result;
    }

private:
    TabSelection selection = TabSelection::Auto;
};