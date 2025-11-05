#include "IssueTrackerView.hpp"
#include "IssueTrackerController.hpp"
#include "IssueRepository.hpp"

int main() {
    auto repo = createIssueRepository();
    auto controller = std::make_unique<IssueTrackerController>(std::move(repo));
    IssueTrackerView view(controller.get());
    view.run();
    return 0;
}
