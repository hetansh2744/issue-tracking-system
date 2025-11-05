#include "IssueTrackerView.hpp"
#include "IssueTrackerController.hpp"
#include "IssueRepository.hpp"

int main() {
    auto repo = createIssueRepository();
    IssueTrackerController controller(repo.get());
    IssueTrackerView view(&controller);
    view.run();
    return 0;
}
