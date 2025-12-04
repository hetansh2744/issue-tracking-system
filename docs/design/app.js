import { getIssues, addMockIssue } from "./data.js";
import { renderIssues } from "./issue.js";
import { createModal } from "./modal.js";
import { apiClient, fetchComments } from "./api.js";

const issuesListEl = document.getElementById("issues-list");
const addMockBtn = document.getElementById("add-mock");
const createIssueBtn = document.getElementById("create-issue-btn");
const statusEl = document.getElementById("load-status");
const searchInput = document.getElementById("issue-search");

document.querySelectorAll(".nav-btn[data-target]").forEach((btn) => {
  btn.addEventListener("click", () => {
    const target = btn.dataset.target;
    if (target) {
      window.location.href = target;
    }
  });
});

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
    renderFiltered();
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

const normalizeSearchId = (value) => {
  const trimmed = (value || "").trim();
  if (!trimmed) return null;
  const cleaned = trimmed.startsWith("#") ? trimmed.slice(1) : trimmed;
  const asNum = Number(cleaned);
  return Number.isNaN(asNum) ? null : asNum;
};

const matchesIssueId = (issue, id) => {
  if (issue.rawId === id) return true;
  const clean = `${issue.id || ""}`.replace(/^#/, "");
  return Number(clean) === id;
};

const filterIssues = (issues = [], queryRaw = "") => {
  const query = (queryRaw || "").toLowerCase().trim();
  if (!query) return issues;
  const tokens = query.split(/\s+/).filter(Boolean);
  if (!tokens.length) return issues;

  return issues.filter((issue) => {
    const tagLabels = (issue.tags || []).map((t) => t.label || "").join(" ");
    const baseFields = [
      issue.id,
      issue.rawId,
      issue.author,
      issue.assignedTo,
      issue.milestone,
      tagLabels
    ]
      .filter(Boolean)
      .map((f) => `${f}`.toLowerCase())
      .join(" ");

    return tokens.every((token) => {
      const [prefix, rest] = token.split(":", 2);
      if (rest !== undefined) {
        const value = rest.trim();
        if (!value) return true;
        if (["author", "by"].includes(prefix)) {
          return (issue.author || "").toLowerCase().includes(value);
        }
        if (["assignee", "assigned", "signee"].includes(prefix)) {
          return (issue.assignedTo || "").toLowerCase().includes(value);
        }
        return baseFields.includes(token);
      }
      return baseFields.includes(token);
    });
  });
};

const renderFiltered = (issuesToRender) => {
  const toRender =
    issuesToRender || filterIssues(cachedIssues, searchInput?.value || "");
  renderAll(toRender);
};

const ensureIssueLoadedById = async (id) => {
  const existingIdx = cachedIssues.findIndex((issue) => matchesIssueId(issue, id));
  if (existingIdx >= 0) return cachedIssues[existingIdx];

  const issue = await apiClient.fetchIssueById(id, activeDatabase);
  const updated = [...cachedIssues];
  updated.unshift(issue);
  cachedIssues = updated;
  return issue;
};

const handleSearchEnter = async () => {
  const parsedId = normalizeSearchId(searchInput?.value);
  if (parsedId !== null) {
    setStatus(`Searching for issue #${parsedId}...`);
    try {
      const issue = await ensureIssueLoadedById(parsedId);
      setStatus(`Found issue #${issue.rawId ?? parsedId}.`);
    } catch (err) {
      console.error("Failed to search issue by id:", err);
      const message = (err?.message || "").includes("404")
        ? `Issue #${parsedId} not found.`
        : "Search failed. Please try again.";
      setStatus(message, true);
    }
  }
  renderFiltered();
};

const handleSearchInput = () => {
  const filtered = filterIssues(cachedIssues, searchInput?.value || "");
  renderFiltered(filtered);
  if (searchInput?.value.trim()) {
    setStatus(`Found ${filtered.length} matching issue(s).`);
  }
};

const renderAll = (issuesToRender = cachedIssues) => {
  renderIssues({
    container: issuesListEl,
    issues: issuesToRender,
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
  renderFiltered();
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
  renderFiltered();
  setStatus("Added a mock issue locally.", false);
});

createIssueBtn.addEventListener("click", () => {
  modal.openCreate();
});

searchInput?.addEventListener("keydown", (evt) => {
  if (evt.key === "Enter") {
    evt.preventDefault();
    handleSearchEnter();
  }
});

searchInput?.addEventListener("input", () => {
  handleSearchInput();
});

const handleDelete = async (issue) => {
  if (!issue || issue.rawId === undefined || issue.rawId === null) {
    setStatus("Cannot delete: missing issue id.", true);
    return;
  }
  try {
    await apiClient.deleteIssue(issue.rawId);
    cachedIssues = cachedIssues.filter((i) => i.rawId !== issue.rawId);
    renderFiltered();
    setStatus(`Deleted issue #${issue.rawId}.`, false);
  } catch (err) {
    console.error("Failed to delete issue:", err);
    setStatus(err.message || "Failed to delete issue.", true);
  }
};

refreshFromApi();
