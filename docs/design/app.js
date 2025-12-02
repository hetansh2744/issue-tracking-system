import { getIssues, addMockIssue } from "./data.js";
import { renderIssues } from "./issue.js";
import { createModal } from "./modal.js";

const issuesListEl = document.getElementById("issues-list");
const addMockBtn = document.getElementById("add-mock");

const modal = createModal();

const renderAll = () => {
  renderIssues({
    container: issuesListEl,
    issues: getIssues(),
    onOpen: (issue) => modal.open(issue)
  });
};

addMockBtn.addEventListener("click", () => {
  addMockIssue();
  renderAll();
});

renderAll();
