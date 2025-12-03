import { getIssues, addMockIssue } from "./data.js";
import { renderIssues } from "./issue.js";
import { createModal } from "./modal.js";
import { apiClient, fetchComments } from "./api.js";

const issuesListEl = document.getElementById("issues-list");
const addMockBtn = document.getElementById("add-mock");
const createIssueBtn = document.getElementById("create-issue-btn");
const statusEl = document.getElementById("load-status");

const modal = createModal({
  onIssueUpdated: (updated) => {
    const normalized = updated.rawId ? { ...updated } : apiClient.mapIssue(updated);
    if (!normalized.database) {
      normalized.database = apiClient.getActiveDatabaseName();
    }
    if (normalized.rawId === undefined || normalized.rawId === null) {
      const cleanId = Number((normalized.id || "").toString().replace(/^#/, ""));
      normalized.rawId = Number.isNaN(cleanId) ? normalized.id : cleanId;
      normalized.id = normalized.rawId ? `#${normalized.rawId}` : normalized.id;
    }
    const idx = cachedIssues.findIndex((issue) => issue.rawId === normalized.rawId);
    if (idx >= 0) {
      cachedIssues[idx] = { ...cachedIssues[idx], ...normalized };
    } else {
      cachedIssues = [{ ...normalized, rawId: normalized.rawId }, ...cachedIssues];
    }
    renderAll();
  },
  getActiveDatabase: apiClient.getActiveDatabaseName
});
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
    onEdit: (issue) => modal.openEdit(issue),
    onDelete: (issue) => handleDelete(issue)
  });
};

const refreshFromApi = async () => {
  try {
    activeDatabase = await apiClient.fetchActiveDatabase();
  } catch (err) {
    console.error("Failed to load databases, continuing without:", err);
  }
  try {
    apiClient.setActiveDatabaseName(activeDatabase);
    cachedIssues = await apiClient.fetchIssues();
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

const handleDelete = async (issue) => {
  if (!issue || issue.rawId === undefined || issue.rawId === null) {
    setStatus("Cannot delete: missing issue id.", true);
    return;
  }
  try {
    await apiClient.deleteIssue(issue.rawId);
    cachedIssues = cachedIssues.filter((i) => i.rawId !== issue.rawId);
    renderAll();
    setStatus(`Deleted issue #${issue.rawId}.`, false);
  } catch (err) {
    console.error("Failed to delete issue:", err);
    setStatus(err.message || "Failed to delete issue.", true);
  }
};

refreshFromApi();
