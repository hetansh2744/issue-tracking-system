import { getIssues, addMockIssue } from "./data.js";
import { renderIssues } from "./issue.js";
import { createModal } from "./modal.js";
import { apiClient, fetchComments } from "./api.js";

const issuesListEl = document.getElementById("issues-list");
const addMockBtn = document.getElementById("add-mock");
const createIssueBtn = document.getElementById("create-issue-btn");
const statusEl = document.getElementById("load-status");
const searchInput = document.getElementById("issue-search");
const statusButtons = document.querySelectorAll("[data-status]");

document.querySelectorAll(".nav-btn[data-target]").forEach((btn) => {
  btn.addEventListener("click", () => {
    const target = btn.dataset.target;
    if (target) {
      window.location.href = target;
    }
  });
});

const stateGroup = document.querySelector('[data-role="status-filters"]');
const stateButtons = stateGroup
  ? Array.from(stateGroup.querySelectorAll(".state-btn"))
  : [];

const STATUS_LABELS = {
  TODO: "To Be Done",
  IN_PROGRESS: "In Progress",
  DONE: "Done"
};

const normalizeStatusValue = (value) => {
  if (value === undefined || value === null) return "";
  const trimmed = `${value}`.trim();
  if (!trimmed) return "";
  const lower = trimmed.toLowerCase();
  if (trimmed === "1" || lower === "todo" || lower === "to be done") {
    return STATUS_LABELS.TODO;
  }
  if (trimmed === "2" || lower === "in progress") {
    return STATUS_LABELS.IN_PROGRESS;
  }
  if (trimmed === "3" || lower === "done") {
    return STATUS_LABELS.DONE;
  }
  return trimmed;
};

const getIssueStatus = (issue) =>
  normalizeStatusValue(issue.status || issue.milestone || "");

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
let activeStatusFilter = "all";
let currentStatusFilter = "ALL"; // CHECK BACK

const setStatus = (message, isError = false) => {
  if (!statusEl) return;
  statusEl.textContent = message;
  statusEl.classList.toggle("error", isError);
  statusEl.classList.add("show");
};

const normalizeStatusValue = (value) => {
  const normalized = (value || "").toString().trim().toLowerCase();
  if (normalized === "to be done" || normalized === "to do" || normalized === "todo") return "todo";
  if (normalized === "done") return "done";
  if (normalized === "backlog") return "backlog";
  return normalized;
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

const filterByStatus = (issues = []) => {
  if (currentStatusFilter === "ALL") return issues;
  const label = STATUS_LABELS[currentStatusFilter];
  if (!label) return issues;
  return issues.filter((issue) => getIssueStatus(issue) === label);
};

const filterIssuesByText = (issues = [], queryRaw = "") => {
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
      issue.status,
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
        if (["status"].includes(prefix)) {
          return getIssueStatus(issue).toLowerCase().includes(value);
        }
        return baseFields.includes(token);
      }
      return baseFields.includes(token);
    });
  });
};

const statusLabelByKey = (key) => {
  switch (key) {
    case "todo":
      return "To be Done";
    case "done":
      return "Done";
    case "backlog":
      return "Backlog";
    default:
      return "All";
  }
};

const updateStatusButtons = () => {
  const counts = cachedIssues.reduce(
    (acc, issue) => {
      const statusKey = normalizeStatusValue(issue.status);
      if (statusKey === "todo" || statusKey === "done" || statusKey === "backlog") {
        acc[statusKey] += 1;
      }
      acc.all += 1;
      return acc;
    },
    { todo: 0, done: 0, backlog: 0, all: 0 }
  );

  statusButtons.forEach((btn) => {
    const key = btn.dataset.status || "all";
    const label = statusLabelByKey(key);
    const count = counts[key] ?? counts.all;
    btn.textContent = `${label} (${count})`;
    btn.classList.toggle("is-active", key === activeStatusFilter);
    btn.setAttribute("aria-pressed", key === activeStatusFilter ? "true" : "false");
  });
};

const renderFiltered = (issuesToRender) => {
  const searchFiltered =
    issuesToRender || filterIssues(cachedIssues, searchInput?.value || "");
  const statusFiltered =
    activeStatusFilter === "all"
      ? searchFiltered
      : searchFiltered.filter(
          (issue) => normalizeStatusValue(issue.status) === activeStatusFilter
        );

  renderAll(statusFiltered);
  updateStatusButtons();
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
  renderFiltered();
  const filtered = getFilteredIssues();
  if (searchInput?.value.trim()) {
    const statusFiltered =
      activeStatusFilter === "all"
        ? filtered
        : filtered.filter(
            (issue) => normalizeStatusValue(issue.status) === activeStatusFilter
          );
    setStatus(`Found ${statusFiltered.length} matching issue(s).`);
  }
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
    setStatus(
      `Loaded ${cachedIssues.length} issue(s) from API.${
        activeDatabase ? " Active DB: " + activeDatabase : ""
      }`
    );
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

statusButtons.forEach((btn) => {
  btn.addEventListener("click", () => {
    const key = btn.dataset.status || "all";
    activeStatusFilter = key;
    renderFiltered();
  });
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

if (addMockBtn) {
  addMockBtn.addEventListener("click", () => {
    addMockIssue();
    cachedIssues = getIssues();
    renderFiltered();
    setStatus("Added a mock issue locally.", false);
  });
}

if (createIssueBtn) {
  createIssueBtn.addEventListener("click", () => {
    modal.openCreate();
  });
}

searchInput?.addEventListener("keydown", (evt) => {
  if (evt.key === "Enter") {
    evt.preventDefault();
    handleSearchEnter();
  }
});

searchInput?.addEventListener("input", () => {
  handleSearchInput();
});

stateButtons.forEach((btn) => {
  const filter = btn.dataset.statusFilter || "ALL";
  btn.addEventListener("click", () => {
    currentStatusFilter = filter;
    renderFiltered();
  });
});

refreshFromApi();
