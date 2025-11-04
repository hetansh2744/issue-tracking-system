// Minimal source file that allows the pipeline to pass.
#include "IssueTrackerView.h"
#include "IssueRepository.h"
#include "IssueTrackerController.h"

int main() {
    //IssueRepository repo;
    IssueTrackerController controller(nullptr);
    IssueTrackerView view(&controller);
    view.run();
    return 0;
}
