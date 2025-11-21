#include "IssueTrackerView.hpp"
#include "IssueTrackerController.hpp"
#include "IssueRepository.hpp"

int main() {
    IssueRepository* repo;
    IssueTrackerController controller(repo);
    IssueTrackerView view(&controller);
    view.run();

    delete repo;
    return 0;
}
