#include "selector.hh"
#include "plan.hh"

int Selector::selectFromAgenda( Plan * plan){
    return plan->getCurrentAgenda()->back().first;
};

int Selector::selectTask(Plan * plan){
    return plan->getCurrentContext()->offspring->size() -1;
};

int Selector::selectMethod(Plan * plan){
    return 0;
};

int Selector::selectUnification(Plan * plan) {
    return 0;
};

