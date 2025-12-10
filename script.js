// Simple frontend issue tracker using localStorage
// This mimics your C++ text-based ITS at a high level.

const STORAGE_KEY = "its_issues_v1";

let issues = [];

// Helpers
function loadIssues() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    issues = raw ? JSON.parse(raw) : [];
  } catch (e) {
    console.error("Failed to load issues from storage", e);
    issues = [];
  }
}

function saveIssues() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(issues));
  } catch (e) {
    console.error("Failed to save issues", e);
  }
}

function generateId() {
  // Simple incremental ID
  const maxId = issues.reduce((max, issue) => Math.max(max, issue.id), 0);
  return maxId + 1;
}

function formatDate(iso) {
  if (!iso) return "";
  const d = new Date(iso);
  return d.toLocaleString();
}

// UI updates
function updateStats() {
  const statsBar = document.getElementById("stats-bar");
  const total = issues.length;
  const openCount = issues.filter((i) => i.status === "open").length;
  const inProgCount = issues.filter((i) => i.status === "in_progress").length;
  const resCount = issues.filter((i) => i.status === "resolved").length;
  const closedCount = issues.filter((i) => i.status === "closed").length;

  statsBar.innerHTML = `
    <div class="stat-pill">Total: ${total}</div>
    <div class="stat-pill">Open: ${openCount}</div>
    <div class="stat-pill">In Progress: ${inProgCount}</div>
    <div class="stat-pill">Resolved: ${resCount}</div>
    <div class="stat-pill">Closed: ${closedCount}</div>
  `;
}

function applyFilters(issue, searchTerm, statusFilter) {
  // Status filter
  if (statusFilter !== "all" && issue.status !== statusFilter) return false;

  // Search filter
  if (searchTerm) {
    const s = searchTerm.toLowerCase();
    const haystack = [
      issue.title,
      issue.description,
      issue.assignee || "",
      (issue.tags || []).join(", "),
    ]
      .join(" ")
      .toLowerCase();

    if (!haystack.includes(s)) return false;
  }

  return true;
}

function renderIssues() {
  const list = document.getElementById("issues-list");
  const searchInput = document.getElementById("search");
  const statusFilter = document.getElementById("filter-status");

  const searchTerm = searchInput.value.trim().toLowerCase();
  const filterStatus = statusFilter.value;

  const filtered = issues.filter((issue) =>
    applyFilters(issue, searchTerm, filterStatus)
  );

  list.innerHTML = "";

  if (filtered.length === 0) {
    list.innerHTML =
      '<div class="empty-state">No issues found. Create one on the left or adjust your filters.</div>';
    updateStats();
    return;
  }

  filtered
    .slice()
    .sort((a, b) => b.id - a.id) // newest first
    .forEach((issue) => {
      const item = document.createElement("div");
      item.className = "issue-item";
      item.dataset.id = issue.id;

      const tagsText =
        issue.tags && issue.tags.length > 0
          ? issue.tags.join(", ")
          : "No tags";

      const assigneeText = issue.assignee
        ? `Assignee: ${issue.assignee}`
        : "Unassigned";

      item.innerHTML = `
        <div class="issue-header">
          <div>
            <div class="issue-title">${issue.title}</div>
            <div class="issue-id">#${issue.id}</div>
          </div>
          <div class="issue-status status-${issue.status}">
            ${issue.status.replace("_", " ")}
          </div>
        </div>

        <div class="issue-desc">
          ${issue.description}
        </div>

        <div class="issue-meta">
          <div>${assigneeText}</div>
          <div class="issue-tags"><span>Tags:</span> ${tagsText}</div>
        </div>

        <div class="issue-footer">
          <div class="issue-dates">
            <div>Created: ${formatDate(issue.createdAt)}</div>
            <div>Updated: ${formatDate(issue.updatedAt)}</div>
          </div>
          <div class="issue-actions">
            <button class="secondary js-edit">Edit</button>
            <button class="secondary js-cycle-status">Next status</button>
            <button class="danger js-delete">Delete</button>
          </div>
        </div>
      `;

      list.appendChild(item);
    });

  updateStats();
}

function resetForm() {
  const form = document.getElementById("issue-form");
  form.reset();
  document.getElementById("issue-id").value = "";
  document.getElementById("status").value = "open";
  document.getElementById("form-title").textContent = "Create New Issue";
  document.getElementById("save-btn").textContent = "Create Issue";
  document.getElementById("cancel-edit-btn").style.display = "none";
}

// Logic for creating/updating/deleting
function handleFormSubmit(e) {
  e.preventDefault();

  const idField = document.getElementById("issue-id");
  const titleField = document.getElementById("title");
  const descField = document.getElementById("description");
  const assigneeField = document.getElementById("assignee");
  const statusField = document.getElementById("status");
  const tagsField = document.getElementById("tags");

  const title = titleField.value.trim();
  const description = descField.value.trim();
  const assignee = assigneeField.value.trim();
  const status = statusField.value;
  const tagsRaw = tagsField.value.trim();

  if (!title || !description) {
    alert("Title and description are required.");
    return;
  }

  const tags =
    tagsRaw === ""
      ? []
      : tagsRaw
          .split(",")
          .map((t) => t.trim())
          .filter((t) => t.length > 0);

  const now = new Date().toISOString();
  const existingId = idField.value ? parseInt(idField.value, 10) : null;

  if (existingId) {
    // Update
    const idx = issues.findIndex((i) => i.id === existingId);
    if (idx === -1) {
      alert("Issue not found. It may have been deleted.");
      resetForm();
      return;
    }
    issues[idx] = {
      ...issues[idx],
      title,
      description,
      assignee: assignee || "",
      status,
      tags,
      updatedAt: now,
    };
  } else {
    // Create new
    const newIssue = {
      id: generateId(),
      title,
      description,
      assignee: assignee || "",
      status,
      tags,
      createdAt: now,
      updatedAt: now,
    };
    issues.push(newIssue);
  }

  saveIssues();
  renderIssues();
  resetForm();
}

function beginEditIssue(id) {
  const issue = issues.find((i) => i.id === id);
  if (!issue) return;

  document.getElementById("issue-id").value = issue.id;
  document.getElementById("title").value = issue.title;
  document.getElementById("description").value = issue.description;
  document.getElementById("assignee").value = issue.assignee || "";
  document.getElementById("status").value = issue.status;
  document.getElementById("tags").value = (issue.tags || []).join(", ");

  document.getElementById("form-title").textContent = `Edit Issue #${issue.id}`;
  document.getElementById("save-btn").textContent = "Save Changes";
  document.getElementById("cancel-edit-btn").style.display = "inline-flex";

  // Scroll to top of page on small screens
  window.scrollTo({ top: 0, behavior: "smooth" });
}

function deleteIssue(id) {
  if (!confirm(`Are you sure you want to delete issue #${id}?`)) return;
  issues = issues.filter((i) => i.id !== id);
  saveIssues();
  renderIssues();
}

function cycleStatus(id) {
  const idx = issues.findIndex((i) => i.id === id);
  if (idx === -1) return;

  const order = ["open", "in_progress", "resolved", "closed"];
  const current = issues[idx].status;
  const currentIndex = order.indexOf(current);
  const nextStatus = order[(currentIndex + 1) % order.length];

  issues[idx].status = nextStatus;
  issues[idx].updatedAt = new Date().toISOString();

  saveIssues();
  renderIssues();
}

// Clear all
function clearAllIssues() {
  if (!issues.length) {
    alert("There are no issues to clear.");
    return;
  }
  if (!confirm("Clear ALL issues from this demo? This cannot be undone.")) {
    return;
  }
  issues = [];
  saveIssues();
  renderIssues();
}

// Event wiring
document.addEventListener("DOMContentLoaded", () => {
  loadIssues();
  renderIssues();

  document.getElementById("year").textContent = new Date().getFullYear();

  const form = document.getElementById("issue-form");
  form.addEventListener("submit", handleFormSubmit);

  document
    .getElementById("cancel-edit-btn")
    .addEventListener("click", () => resetForm());

  document
    .getElementById("clear-all-btn")
    .addEventListener("click", clearAllIssues);

  document.getElementById("search").addEventListener("input", renderIssues);
  document
    .getElementById("filter-status")
    .addEventListener("change", renderIssues);

  // Event delegation for actions on issue cards
  document
    .getElementById("issues-list")
    .addEventListener("click", (event) => {
      const item = event.target.closest(".issue-item");
      if (!item) return;
      const id = parseInt(item.dataset.id, 10);

      if (event.target.classList.contains("js-edit")) {
        beginEditIssue(id);
      } else if (event.target.classList.contains("js-delete")) {
        deleteIssue(id);
      } else if (event.target.classList.contains("js-cycle-status")) {
        cycleStatus(id);
      }
    });
});
