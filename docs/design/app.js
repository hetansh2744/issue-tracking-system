import { getIssues, addMockIssue } from "./data.js";
import { renderIssues } from "./issue.js";
import { createModal } from "./modal.js";
import { fetchIssues, fetchComments, fetchActiveDatabase } from "./api.js";

const issuesListEl = document.getElementById("issues-list");
const addMockBtn = document.getElementById("add-mock");
const createIssueBtn = document.getElementById("create-issue-btn");
const statusEl = document.getElementById("load-status");

const modal = createModal();
let cachedIssues = [];
let activeDatabase = undefined;

const setStatus = (message, isError = false) => {
  if (!statusEl) return;
  statusEl.textContent = message;
  statusEl.classList.toggle("error", isError);
  statusEl.classList.add("show");
};

const renderAll = () => {
  renderIssues({
    container: issuesListEl,
    issues: cachedIssues,
    onOpen: (issue) => openIssueDetail(issue),
    onEdit: (issue) => modal.openEdit(issue)
  });
};

const refreshFromApi = async () => {
  try {
    activeDatabase = await fetchActiveDatabase();
  } catch (err) {
    console.error("Failed to load databases, continuing without:", err);
  }
  try {
    cachedIssues = await fetchIssues(activeDatabase);
    setStatus(`Loaded ${cachedIssues.length} issue(s) from API.${activeDatabase ? " Active DB: " + activeDatabase : ""}`);
  } catch (err) {
    console.error("Failed to load issues from API, using local data:", err);
    cachedIssues = getIssues();
    setStatus("API load failed; showing local mock data.", true);
  }
  renderAll();
};

const openIssueDetail = async (issue) => {
  try {
    const comments = await fetchComments(issue.rawId);
    modal.openDetail({ ...issue, comments });
  } catch (err) {
    console.error("Failed to load comments; showing issue without comments:", err);
    modal.openDetail(issue);
  }
};

addMockBtn.addEventListener("click", () => {
  addMockIssue();
  cachedIssues = getIssues();
  renderAll();
  setStatus("Added a mock issue locally.", false);
});

createIssueBtn.addEventListener("click", () => {
  modal.openCreate();
});

refreshFromApi();
